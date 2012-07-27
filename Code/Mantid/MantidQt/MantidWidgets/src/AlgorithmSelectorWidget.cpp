#include "MantidQtMantidWidgets/AlgorithmSelectorWidget.h"
#include "MantidKernel/System.h"
#include "MantidAPI/AlgorithmManager.h"

using namespace Mantid::Kernel;
using namespace Mantid::API;

namespace {
  // Customised QCompleter to match anywhere in an algorithm name, not just from the start
  // Adapted from python code in http://stackoverflow.com/questions/5129211/qcompleter-custom-completion-rules
  class InlineCompleter : public QCompleter
  {
  public:
    InlineCompleter(QWidget * parent = NULL)
     : QCompleter(parent), m_local_completion_prefix(), m_source_model(NULL)
    {
    }

    void setModel ( QAbstractItemModel * model )
    {
      m_source_model = model;
      QCompleter::setModel(m_source_model);
    }

    QStringList splitPath ( const QString & path ) const
    {
      m_local_completion_prefix = path;
      updateModel();
      QStringList sl;
      sl << "";
      return sl;
    }

  private:
    void updateModel() const
    {
      class InnerProxyModel : public QSortFilterProxyModel
      {
      public:
        // This method is called for each algorithm and returns true if the text
        // entered so far matches anywhere in the algorithm name
        bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
        {
          // Only start matching once the 2nd character has been typed
          if ( local_completion_prefix.size() < 2 ) return false;

          QModelIndex index0 = sourceModel()->index(source_row,0,source_parent);
          return sourceModel()->data(index0).toString().lower().contains(local_completion_prefix.lower());
        }
        QString local_completion_prefix;
      };

      InnerProxyModel * proxy_model = new InnerProxyModel;
      proxy_model->local_completion_prefix = m_local_completion_prefix;
      proxy_model->setSourceModel(m_source_model);
      // Yes I know this is evil, but it's necessary to call the current method
      // from the overridden splitPath above.
      const_cast<InlineCompleter *>(this)->QCompleter::setModel(proxy_model);
    }

    mutable QString m_local_completion_prefix;
    QAbstractItemModel * m_source_model;
  };
}

namespace MantidQt
{
namespace MantidWidgets
{


  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  AlgorithmSelectorWidget::AlgorithmSelectorWidget(QWidget *parent)
  : QWidget(parent), m_tree(NULL), m_findAlg(NULL), m_execButton(NULL),
    m_updateObserver(*this, &AlgorithmSelectorWidget::handleAlgorithmFactoryUpdate),
    m_updateInProgress(false)
  {
    QHBoxLayout * buttonLayout = new QHBoxLayout();

    m_tree = new AlgorithmTreeWidget(this);
    m_tree->setHeaderLabel("Algorithms");
    connect(m_tree,SIGNAL(itemSelectionChanged()),
        this,SLOT(treeSelectionChanged()));
    connect(m_tree,SIGNAL(executeAlgorithm(const QString &, int)),
        this,SLOT(executeSelected()));

    m_findAlg = new FindAlgComboBox;
    m_findAlg->setEditable(true);
    m_findAlg->setInsertPolicy(QComboBox::NoInsert);

    // Must use full InlineCompleter type for pointer as overridden
    // setModel method called below is NOT virtual.
    InlineCompleter * completer = new InlineCompleter(m_findAlg);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setModel(m_findAlg->model());
    m_findAlg->setCompleter(completer);

    connect(m_findAlg,SIGNAL(enterPressed()),
        this,SLOT(executeSelected()));
    connect(m_findAlg,SIGNAL(editTextChanged(const QString&)),
        this,SLOT(findAlgTextChanged(const QString&)));
    connect(completer,SIGNAL(highlighted(const QString&)),
        m_findAlg,SLOT(setEditText(const QString&)));

    m_execButton = new QPushButton("Execute");
    connect(m_execButton,SIGNAL(clicked()),
        this,SLOT(executeSelected()));
    buttonLayout->addWidget(m_execButton);

    buttonLayout->addWidget(m_findAlg);
    buttonLayout->addStretch();

    // Layout the tree and combo box
    QVBoxLayout * layout = new QVBoxLayout(this, 0 /*border*/, 4 /*spacing*/);
    //this->setLayout(layout);
    layout->addLayout(buttonLayout);
    layout->addWidget(m_tree);

    AlgorithmFactory::Instance().notificationCenter.addObserver(m_updateObserver);
  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  AlgorithmSelectorWidget::~AlgorithmSelectorWidget()
  {
    AlgorithmFactory::Instance().notificationCenter.removeObserver(m_updateObserver);
  }
  

  /** Is the the execute button visible */
  bool AlgorithmSelectorWidget::showExecuteButton() const
  {
    return m_execButton->isVisible();
  }

  /** Show/hide the execute button */
  void AlgorithmSelectorWidget::showExecuteButton(const bool val)
  {
    m_execButton->setVisible(val);
  }

  //---------------------------------------------------------------------------
  /** Update the lists of algorithms */
  void AlgorithmSelectorWidget::update()
  {
    m_updateInProgress = true;
    m_findAlg->update();
    m_tree->update();
    m_updateInProgress = false;
  }


  //---------------------------------------------------------------------------
  /** Slot called to execute whatever is the selected algorithm
   **/
  void AlgorithmSelectorWidget::executeSelected()
  {
    QString algName;
    int version;
    this->getSelectedAlgorithm(algName,version);
    emit executeAlgorithm(algName, version);
  }


  //---------------------------------------------------------------------------
  /** Show the selection in the tree when it changes in the combo */
  void AlgorithmSelectorWidget::findAlgTextChanged(const QString& text)
  {
    int i = m_findAlg->findText(text,Qt::MatchFixedString);
    if (i >= 0) m_findAlg->setCurrentIndex(i);
    // De-select from the tree
    m_tree->blockSignals(true);
    m_tree->setCurrentIndex(QModelIndex());
    m_tree->blockSignals(false);

    // Emit the signal
    QString algName;
    int version;
    this->getSelectedAlgorithm(algName,version);
    emit algorithmSelectionChanged(algName, version);
  }

  //---------------------------------------------------------------------------
  /** Show the selection in the combo when it changes in the tree */
  void AlgorithmSelectorWidget::treeSelectionChanged()
  {
    QString algName;
    int version;
    this->getSelectedAlgorithm(algName,version);
    // Select in the combo box
    m_findAlg->blockSignals(true);
    m_findAlg->setCurrentIndex(m_findAlg->findText(algName,Qt::MatchFixedString));
    m_findAlg->blockSignals(false);
    // Emit the signal
    emit algorithmSelectionChanged(algName, version);
  }

  //---------------------------------------------------------------------------
  /** Return the selected algorithm.
   * The tree has priority. If nothing is selected in the tree,
   * return the ComboBox selection */
  void AlgorithmSelectorWidget::getSelectedAlgorithm(QString& algName, int& version)
  {
    algName.clear();
    m_tree->getSelectedAlgorithm(algName, version);
    if (algName.isEmpty())
      m_findAlg->getSelectedAlgorithm(algName, version);
  }

  //---------------------------------------------------------------------------
  /** @return just the name of the selected algorithm */
  QString AlgorithmSelectorWidget::getSelectedAlgorithm()
  {
    QString algName; int version;
    this->getSelectedAlgorithm(algName, version);
    return algName;
  }

  //---------------------------------------------------------------------------
  /** Set which algorithm is currently selected. Does not fire any signals.
   * Updates the combobox, deselects in the tree.
   *
   * @param algName :: name of the algorithm
   */
  void AlgorithmSelectorWidget::setSelectedAlgorithm(QString & algName)
  {
    m_findAlg->blockSignals(true);
    m_findAlg->setCurrentIndex(m_findAlg->findText(algName,Qt::MatchFixedString));
    m_findAlg->blockSignals(false);
    // De-select from the tree
    m_tree->blockSignals(true);
    m_tree->setCurrentIndex(QModelIndex());
    m_tree->blockSignals(false);
  }

  /**
   * The algorithm factory has been updated, refresh the widget
   * @param
   */
  void AlgorithmSelectorWidget::
  handleAlgorithmFactoryUpdate(Mantid::API::AlgorithmFactoryUpdateNotification_ptr)
  {
    if(!m_updateInProgress)
    {
      this->update();
    }
  }

  //============================================================================
  //============================================================================
  //============================================================================
  //Use an anonymous namespace to keep these at file scope
  namespace {

    bool Algorithm_descriptor_less(const Algorithm_descriptor& d1,const Algorithm_descriptor& d2)
    {
      if (d1.category < d2.category) return true;
      else if (d1.category == d2.category && d1.name < d2.name) return true;
      else if (d1.category == d2.category && d1.name == d2.name && d1.version > d2.version) return true;

      return false;
    }

    bool Algorithm_descriptor_name_less(const Algorithm_descriptor& d1,const Algorithm_descriptor& d2)
    {
      return d1.name < d2.name;
    }

  }



  //============================================================================
  //======================= AlgorithmTreeWidget ================================
  //============================================================================
  /** Return the selected algorithm in the tree */
  void AlgorithmTreeWidget::getSelectedAlgorithm(QString& algName, int& version)
  {
    QList<QTreeWidgetItem*> items = this->selectedItems();
    if ( items.size() == 0 )
    {
      // Nothing selected
      algName = "";
      version = 0;
    }
    else if ( items[0]->childCount() != 0 && !items[0]->text(0).contains(" v."))
    {
      algName = "";
      version = 0;
    }
    else
    {
      QString str = items[0]->text(0);
      QStringList lst = str.split(" v.");
      algName = lst[0];
      version = lst[1].toInt();
    }
  }


  //---------------------------------------------------------------------------
  /** SLOT called when clicking the mouse around the tree */
  void AlgorithmTreeWidget::mousePressEvent (QMouseEvent *e)
  {
    if (e->button() == Qt::LeftButton)
    {
      if( !itemAt(e->pos()) ) selectionModel()->clear();
      m_dragStartPosition = e->pos();
    }

    QTreeWidget::mousePressEvent(e);
  }

  //---------------------------------------------------------------------------
  /** SLOT called when dragging the mouse around the tree */
  void AlgorithmTreeWidget::mouseMoveEvent(QMouseEvent *e)
  {
    if (!(e->buttons() & Qt::LeftButton))
      return;
    if ((e->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance())
      return;

    // Start dragging
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    mimeData->setText("Algorithm");
    drag->setMimeData(mimeData);

    Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction);
    (void) dropAction;
  }

  //---------------------------------------------------------------------------
  /** SLOT called when double-clicking on an entry in the tree */
  void AlgorithmTreeWidget::mouseDoubleClickEvent(QMouseEvent *e)
  {
    QString algName;
    int version;
    this->getSelectedAlgorithm(algName,version);
    if ( ! algName.isEmpty() )
    {
      // Emit the signal that we are executing
      emit executeAlgorithm(algName, version);
      return;
    }
    QTreeWidget::mouseDoubleClickEvent(e);
  }

  //---------------------------------------------------------------------------
  /** Update the list of algos in the tree */
  void AlgorithmTreeWidget::update()
  {
    this->clear();

    typedef std::vector<Algorithm_descriptor> AlgNamesType;
    AlgNamesType names = AlgorithmFactory::Instance().getDescriptors();

    // sort by category/name/version to fill QTreeWidget
    sort(names.begin(),names.end(),Algorithm_descriptor_less);

    QMap<QString,QTreeWidgetItem*> categories;// keeps track of categories added to the tree
    QMap<QString,QTreeWidgetItem*> algorithms;// keeps track of algorithms added to the tree (needed in case there are different versions of an algorithm)

    for(AlgNamesType::const_iterator i=names.begin();i!=names.end();++i)
    {
      QString algName = QString::fromStdString(i->name);
      QString catName = QString::fromStdString(i->category);
      QStringList subCats = catName.split('\\');
      if (!categories.contains(catName))
      {
        if (subCats.size() == 1)
        {
          QTreeWidgetItem *catItem = new QTreeWidgetItem(QStringList(catName));
          categories.insert(catName,catItem);
          this->addTopLevelItem(catItem);
        }
        else
        {
          QString cn = subCats[0];
          QTreeWidgetItem *catItem = 0;
          int n = subCats.size();
          for(int j=0;j<n;j++)
          {
            if (categories.contains(cn))
            {
              catItem = categories[cn];
            }
            else
            {
              QTreeWidgetItem *newCatItem = new QTreeWidgetItem(QStringList(subCats[j]));
              categories.insert(cn,newCatItem);
              if (!catItem)
              {
                this->addTopLevelItem(newCatItem);
              }
              else
              {
                catItem->addChild(newCatItem);
              }
              catItem = newCatItem;
            }
            if (j != n-1) cn += "\\" + subCats[j+1];
          }
        }
      }

      QTreeWidgetItem *algItem = new QTreeWidgetItem(QStringList(algName+" v."+QString::number(i->version)));
      QString cat_algName = catName+algName;
      if (!algorithms.contains(cat_algName))
      {
        algorithms.insert(cat_algName,algItem);
        categories[catName]->addChild(algItem);
      }
      else
        algorithms[cat_algName]->addChild(algItem);

    }
  }

  //============================================================================
  //============================== FindAlgComboBox =============================
  //============================================================================
  /** Called when the combo box for finding algorithms has a key press
   * event  */
  void FindAlgComboBox::keyPressEvent(QKeyEvent *e)
  {
    if (e->key() == Qt::Key_Return)
    {
      emit enterPressed();
      return;
    }
    QComboBox::keyPressEvent(e);
  }

   //---------------------------------------------------------------------------
  /** Update the list of algos in the combo box */
  void FindAlgComboBox::update()
  {
    typedef std::vector<Algorithm_descriptor> AlgNamesType;
    AlgNamesType names = AlgorithmFactory::Instance().getDescriptors();

    // sort by algorithm names only to fill this combobox
    sort(names.begin(),names.end(),Algorithm_descriptor_name_less);

    this->clear();
    std::string prevName = "";
    for(AlgNamesType::const_iterator i=names.begin();i!=names.end();++i)
    {
      if (i->name != prevName)
        this->addItem(QString::fromStdString(i->name));
      prevName = i->name;
    }
    this->setCurrentIndex(-1);

  }

  //---------------------------------------------------------------------------
  /** Return the selected algorithm */
  void FindAlgComboBox::getSelectedAlgorithm(QString& algName, int& version)
  {
    //typed selection
    int i = this->currentIndex(); //selected index in the combobox, could be from an old selection
    QString itemText = this->itemText(i); //text in the combobox at the selected index
    QString typedText = this->currentText(); //text as typed in the combobox
    if (i < 0 || itemText != typedText)
    {
      // Typed text has priority over selected item
      algName = typedText;
      version = -1;
    }
    else
    {
      algName = itemText;
      version = -1;
    }
  }


} // namespace Mantid
} // namespace MantidWidgets

#include "MantidQtSliceViewer/PeaksWorkspaceWidget.h"
#include "MantidQtSliceViewer/QPeaksTableModel.h"
#include <QColorDialog>
#include <QPlastiqueStyle>

namespace MantidQt
{
  namespace SliceViewer
  {

    /**
    Constructor

    @param ws : Peaks Workspace (MODEL)
    @param coordinateSystem : Name of coordinate system used
    @param defaultForegroundColour : Default peak foreground colour
    @param defaultBackgroundColour : Default peak background colour
    @param parent : parent widget
    */
    PeaksWorkspaceWidget::PeaksWorkspaceWidget(Mantid::API::IPeaksWorkspace_const_sptr ws, const std::string& coordinateSystem, const QColor& defaultForegroundColour, const QColor& defaultBackgroundColour, QWidget *parent)
      : QWidget(parent), m_ws(ws), m_coordinateSystem(coordinateSystem), m_foregroundColour(defaultForegroundColour), m_backgroundColour(defaultBackgroundColour)
    {

      ui.setupUi(this);

      // Connect internal signals-slots.
      connect(ui.ckShowBackground, SIGNAL(clicked(bool)), this, SLOT(onShowBackgroundChanged(bool)));
      connect(ui.btnBackgroundColor, SIGNAL(clicked()), this, SLOT(onBackgroundColourClicked()));
      connect(ui.btnPeakColor, SIGNAL(clicked()), this, SLOT(onForegroundColourClicked()));
      connect(ui.btnRemove, SIGNAL(clicked()), this, SLOT(onRemoveWorkspaceClicked()));
      connect(ui.btnHide, SIGNAL(toggled(bool)), this, SLOT(onToggleHideInPlot(bool)));
      connect(ui.tblPeaks, SIGNAL(clicked(const QModelIndex&)), this, SLOT(onTableClicked(const QModelIndex&)));

      // Override the styles for the colour buttons, because with some inherited styles, the button background colour will be hidden.
      ui.btnBackgroundColor->setStyle(new QPlastiqueStyle);
      ui.btnPeakColor->setStyle(new QPlastiqueStyle);

      // Populate controls with data.
      populate();
    }

    /**
    Populate controls with data ready for rendering.
    */
    void PeaksWorkspaceWidget::populate()
    {
      const QString nameText = QString(m_ws->name().c_str());
      ui.lblWorkspaceName->setText(nameText);
      ui.lblWorkspaceName->setToolTip(nameText);

      const QString integratedText = m_ws->hasIntegratedPeaks() ? "Yes" : "No";

      ui.lblWorkspaceState->setText(integratedText);
      ui.lblWorkspaceState->setToolTip(integratedText);

      const QString coordinateText = QString(m_coordinateSystem.c_str());
      ui.lblWorkspaceCoordinates->setText(coordinateText);
      ui.lblWorkspaceCoordinates->setToolTip(coordinateText);

      ui.btnBackgroundColor->setBackgroundColor(m_backgroundColour);
      ui.btnPeakColor->setBackgroundColor(m_foregroundColour);

      ui.tblPeaks->setModel(new QPeaksTableModel(this->m_ws));
      ui.tblPeaks->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
      ui.tblPeaks->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
      m_originalTableWidth = ui.tblPeaks->horizontalHeader()->length();

    }

    /// Destructor
    PeaksWorkspaceWidget::~PeaksWorkspaceWidget()
    {
    }

    /**
    Handler for changing the foreground colour of an integrated peak.
    */
    void PeaksWorkspaceWidget::onForegroundColourClicked()
    {
      QColorDialog colourDlg;
      QColor selectedColour = colourDlg.getColor();

      ui.btnPeakColor->setBackgroundColor(selectedColour);
      emit peakColourChanged(this->m_ws, selectedColour);
    }

    /**
    Handler for changing the background colour of an integrated peak.
    */
    void PeaksWorkspaceWidget::onBackgroundColourClicked()
    {
      QColorDialog colourDlg;
      QColor selectedColour = colourDlg.getColor();
      ui.btnBackgroundColor->setBackgroundColor(selectedColour);
      emit backgroundColourChanged(this->m_ws, selectedColour);
    }

    /**
    Handler or showing/hiding the background radius of integrated peaks.
    @param show : TRUE if the background radius is to be shown.
    */
    void PeaksWorkspaceWidget::onShowBackgroundChanged(bool show)
    {
      emit backgroundRadiusShown(this->m_ws, show);
    }

    /**
    Handler for removing a workspace from the plotting tools.
    */
    void PeaksWorkspaceWidget::onRemoveWorkspaceClicked()
    {
      emit removeWorkspace(this->m_ws);
      this->hide();
    }

    /**
    Handler to hide/show the widget on request.
    @param hidden: flag indicating what to do.
    */
    void PeaksWorkspaceWidget::onToggleHideInPlot(bool hidden)
    {
      emit hideInPlot(this->m_ws, hidden);
    }

    /**
    Event handler for handling the user seleting a row in the table.
    @param index : Index selected.
    */
    void PeaksWorkspaceWidget::onTableClicked(const QModelIndex& index)
    {
      if(index.isValid())
      {
        emit zoomToPeak(this->m_ws, index.row());
      }
    }

    /**
    Override the resizeEvent handler so that we can provide some nice behaviour for the peaks table shown in this widget.

    When the widget is below a threshold width, use scroll-bars.
    When the widget is greather then the threshold width, then we want to stretch the table to fill the full width.
    @param event: Resize event to pass on.
    */
    void PeaksWorkspaceWidget::resizeEvent(QResizeEvent * event)
    {
      QWidget::resizeEvent(event);
      const int currentWidth = ui.tblPeaks->horizontalHeader()->length();
      if(currentWidth > m_originalTableWidth)
      {
        ui.tblPeaks->horizontalHeader()->setResizeMode(QHeaderView::Stretch); // Works well when the widget is expanded, but not well when it's compressed.
      }
      else
      {
        ui.tblPeaks->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents); // Doesn't work well the widget is expanded, but works well when the width is compressed.
      }
    }

  } // namespace
}

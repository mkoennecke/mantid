#include "MantidQtCustomInterfaces/SANSEventSlicing.h"
#include "MantidAPI/AlgorithmManager.h"
#include "ui_SANSEventSlicing.h"
#include <QSettings>
#include <QMessageBox>
#include <QKeyEvent>

namespace MantidQt
{
namespace CustomInterfaces
{

using namespace MantidQt::CustomInterfaces;


// Initialize the logger
Mantid::Kernel::Logger& SANSEventSlicing::g_log = Mantid::Kernel::Logger::get("SANSEventSlicing");
const QString SANSEventSlicing::OUT_MSG("Output Directory: ");


SANSEventSlicing::SANSEventSlicing(QWidget *parent) :
  UserSubWindow(parent),
  parForm(parent), m_pythonRunning(false)
{
  setWindowFlags(windowFlags() | Qt::Dialog| Qt::Popup);
  initLayout();
}

SANSEventSlicing::~SANSEventSlicing()
{
  try
  {
    saveSettings();
  }
  catch(...)
  {
    //we've cleaned up the best we can, move on
  }
}

//Connect signals and setup widgets
void SANSEventSlicing::initLayout()
{
  ui.setupUi(this);
  connect(this,SIGNAL(runAsPythonScript(const QString & , bool)),
          parForm, SIGNAL(runAsPythonScript(const QString & , bool)));
  connect(ui.slicePushButton, SIGNAL(clicked()), 
          this, SLOT(doApplySlice()));
  
  setToolTips();
}

void SANSEventSlicing::initLocalPython(){
  readSettings();
}

/**
 * Restore previous input
 */
void SANSEventSlicing::readSettings()
{
}
/**
 * Save input for future use
 */
void SANSEventSlicing::saveSettings()
{

}
/** sets tool tip strings for the components on the form
*/
void SANSEventSlicing::setToolTips()
{
  /*  m_SANSForm->summedPath_lb->setToolTip("The output files from summing the workspaces\nwill be saved to this directory");
  m_SANSForm->summedPath_Btn->setToolTip("Set the directories used both for loading and\nsaving run data");
  m_SANSForm->loadSeparateEntries->setToolTip("Where possible load a minimum amount into\nmemory at any time");

  m_SANSForm->add_Btn->setToolTip("Click here to do the sum");
  m_SANSForm->clear_Btn->setToolTip("Clear the run files to sum box");
  m_SANSForm->browse_to_add_Btn->setToolTip("Select a run to add to the sum");
  m_SANSForm->new2Add_edit->setToolTip("Select a run to add to the sum");
  m_SANSForm->add_Btn->setToolTip("Select a run to add to the sum");*/
}

void SANSEventSlicing::changeRun(QString value)
{
  ui.run_lineEdit->setText(value);
}

void SANSEventSlicing::changeSlicingString(QString value){
  if (value != m_advancedSlice)
    m_advancedSlice = value; 
}

void SANSEventSlicing::doApplySlice()
{
  if (m_pythonRunning)
    return;

  if (ui.run_lineEdit->text().isEmpty() && false){
    QMessageBox::information(this, "Wrong Input", 
               "Invalid run Number.\nPlease, provide a correct run number of file!");
    return;
  }
  
  if (run_previous == ui.run_lineEdit->text() 
      && 
      from_previous == ui.sliced_from_lineEdit->text()
      && 
      to_previous == ui.sliced_to_lineEdit->text())
    {
      // this means that we have just executed this
    return; 
    }

  bool from_empty = ui.sliced_from_lineEdit->text().isEmpty(); 
  bool to_empty = ui.sliced_to_lineEdit->text().isEmpty(); 
  
  QString code = "import sys\nfrom ISISCommandInterface import sliceSANS2D\n"; 
  code += "try:\n    ws, tt, st, tc, sc = sliceSANS2D(";
  code += "filename='"+ ui.run_lineEdit->text()+ "'"; 
  code += ", outWs='slice'";
  if (!from_empty)
    code += ", time_start="+ui.sliced_from_lineEdit->text();
  
  if (!to_empty)
    code += ", time_stop="+ui.sliced_to_lineEdit->text(); 
  code += ")\n";
  code +="except :\n";
  code +="    print sys.exc_info()\n"; 
  g_log.notice() << "Run : " << code.toStdString() << std::endl; 
  m_pythonRunning = true;
  QString status = runPythonCode(code).simplified();
  if (status.isEmpty()){
    run_previous = ui.run_lineEdit->text(); 
    from_previous = ui.sliced_from_lineEdit->text(); 
    to_previous = ui.sliced_to_lineEdit->text();


    QString value; 
    value = runPythonCode("print '%.1f'%(tt)\n").trimmed(); 
    ui.exp_t_time_label->setText(value);
    
    value = runPythonCode("print '%.2f'%(tc)\n").trimmed(); 
    ui.exp_charge_label->setText(value); 
    
    value = runPythonCode("print '%.2f'%(sc)\n").trimmed(); 
    ui.sliced_charge_label->setText(value);

    QString curr_text = m_advancedSlice;

    if (!(from_empty || to_empty)){
      // this means that one of them is not empty
      if (!curr_text.isEmpty())
        curr_text += ", "; 
    }


    if (!from_empty && to_empty){
      curr_text += ui.sliced_from_lineEdit->text();      
    }else if (from_empty && !to_empty){
      curr_text += ui.sliced_to_lineEdit->text();
    }else if (!from_empty && !to_empty){
      curr_text += ui.sliced_from_lineEdit->text() + "-" + ui.sliced_to_lineEdit->text();
    }
    if (m_advancedSlice != curr_text)
    {
      m_advancedSlice = curr_text; 
      emit slicingString(m_advancedSlice); 
    }

    m_advancedSlice = curr_text; 
  }else{
    QMessageBox::warning(this,"Slice SANS failed",
                         QString("Failed to execute the slicing with the following information: ")
                         + status );
  }
  
  m_pythonRunning = false;
  ui.sliced_from_lineEdit->setFocus();

}

void SANSEventSlicing::showEvent(QShowEvent * ev ){
  if (ui.run_lineEdit->text().isEmpty())
    ui.run_lineEdit->setFocus(); 
  else
    ui.sliced_from_lineEdit->setFocus();
  
  UserSubWindow::showEvent(ev);
}

void SANSEventSlicing::keyPressEvent(QKeyEvent * ev){
  if (ev->key() == Qt::Key_Escape)
    hide(); 
  else if (ev->key() == Qt::Key_Enter)
    doApplySlice(); 
  else
    UserSubWindow::keyPressEvent(ev); 
  
}

/*
void SANSEventSlicing::runPythonEventslicing()
{
  if (m_pythonRunning)
  {//it is only possible to run one python script at a time
    return;
  }

  add2Runs2Add();

  QString code_torun = "import SANSadd2\n";
  code_torun += "print SANSadd2.add_runs((";
  //there are multiple file list inputs that can be filled in loop through them
  for(int i = 0; i < m_SANSForm->toAdd_List->count(); ++i )
  {
    const QString filename =
      m_SANSForm->toAdd_List->item(i)->data(Qt::WhatsThisRole).toString();
    //allow but do nothing with empty entries
    if ( ! filename.isEmpty() )
    {
      code_torun += "'"+filename+"',";
    }
  }
  if ( code_torun.endsWith(',') )
  {//we've made a comma separated list, there can be no comma at the end
    code_torun.truncate(code_torun.size()-1);
  }
  //pass the current instrument
  code_torun += "),'"+m_SANSForm->inst_opt->currentText()+"', '";
  QString ext = m_SANSForm->file_opt->itemData(
    m_SANSForm->file_opt->currentIndex()).toString();
  code_torun += ext+"'";
  
  code_torun += ", rawTypes=(";
  std::set<std::string>::const_iterator end = m_rawExts.end();
  for(std::set<std::string>::const_iterator j=m_rawExts.begin(); j != end; ++j)
  {
    code_torun += "'"+QString::fromStdString(*j)+"',";
  }
  //remove the comma that would remain at the end of the list
  code_torun.truncate(code_torun.length()-1);
  code_torun += ")";
  
  QString lowMem = m_SANSForm->loadSeparateEntries->isChecked()?"True":"False";
  code_torun += ", lowMem="+lowMem;

  if ( m_SANSForm->takeBinningFromMonitors->isChecked() == false )
    code_torun += ", binning='" + m_SANSForm->eventToHistBinning->text() + "'";
  code_torun += ")\n";

  g_log.debug() << "Executing Python: \n" << code_torun.toStdString() << std::endl;

  m_SANSForm->sum_Btn->setEnabled(false);
  m_pythonRunning = true;
  //call the algorithms by executing the above script as Python
  QString status = runPythonCode(code_torun, false);
  
  // reset the controls and display any errors
  m_SANSForm->sum_Btn->setEnabled(true);
  m_pythonRunning = false;
  if (status.startsWith("The following file has been created:"))
  {
    QMessageBox::information(this, "Files summed", status);
  }
  else if (status.startsWith("Error copying log file:"))
  {
    QMessageBox::warning(this, "Error adding files", status);
  }
  else
  {
    if (status.isEmpty())
    {
      status = "Could not sum files, there may be more\ninformation in the Results Log window";
    }
    QMessageBox::critical(this, "Error adding files", status);
  }
}
*/

}//namespace CustomInterfaces
}//namespace MantidQt

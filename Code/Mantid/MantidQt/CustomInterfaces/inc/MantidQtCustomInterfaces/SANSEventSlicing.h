#ifndef MANTIDQTCUSTOMINTERFACES_SANSEVENTSLICING_H_
#define MANTIDQTCUSTOMINTERFACES_SANSEVENTSLICING_H_


#include "MantidQtAPI/UserSubWindow.h"
#include "MantidKernel/ConfigService.h"
#include <Poco/NObserver.h>
#include <QString>
#include "ui_SANSEventSlicing.h"

namespace Ui {
class SANSEventSlicing;
}
namespace MantidQt
{
namespace CustomInterfaces
{

class SANSEventSlicing : public MantidQt::API::UserSubWindow
{
  Q_OBJECT
public:
  /// Default Constructor
  SANSEventSlicing(QWidget *parent=NULL);
  /// Destructor
  virtual ~SANSEventSlicing();


  public slots:
  void changeRun(QString value);
  void changeSlicingString(QString value); 
  
 signals:
  void slicingString(QString value); 

 protected:
  virtual void showEvent(QShowEvent  * ); 
  virtual void keyPressEvent(QKeyEvent *event);

private:
  //set to a pointer to the parent form
  QWidget *parForm;
  //set to true when execution of the python scripts starts and false on completion
  bool m_pythonRunning;

  ///the directory to which files will be saved
  QString m_outDir;
  ///A reference to a logger
  static Mantid::Kernel::Logger & g_log;
  ///The text that goes into the beginning of the output directory message
  static const QString OUT_MSG;

  void initLayout();
  virtual void initLocalPython();
  void setToolTips();
  
  void readSettings();
  void saveSettings();

private slots:

  /// Apply the slice for the SANS data, and update the view with the last sliced data.
  void doApplySlice();

  
 private:
  Ui::SANSEventSlicing ui; 
  QString m_advancedSlice, from_previous, to_previous,run_previous;
};

}
}

#endif  //MANTIDQTCUSTOMINTERFACES_SANSEVENTSLICING_H_

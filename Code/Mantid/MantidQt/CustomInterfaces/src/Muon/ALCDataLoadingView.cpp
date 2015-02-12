#include "MantidQtCustomInterfaces/Muon/ALCDataLoadingView.h"

#include <QMessageBox>

#include <qwt_symbol.h>

namespace MantidQt
{
namespace CustomInterfaces
{
  ALCDataLoadingView::ALCDataLoadingView(QWidget* widget)
    : m_widget(widget), m_dataCurve(new QwtPlotCurve())
  {}

  void ALCDataLoadingView::initialize()
  {
    m_ui.setupUi(m_widget);
    connect(m_ui.load, SIGNAL(clicked()), SIGNAL(loadRequested()));
    connect(m_ui.firstRun, SIGNAL(fileFindingFinished()), SIGNAL(firstRunSelected()));

    m_ui.dataPlot->setCanvasBackground(Qt::white);
    m_ui.dataPlot->setAxisFont(QwtPlot::xBottom, m_widget->font());
    m_ui.dataPlot->setAxisFont(QwtPlot::yLeft, m_widget->font());

    m_dataCurve->setStyle(QwtPlotCurve::NoCurve);
    m_dataCurve->setSymbol(QwtSymbol(QwtSymbol::Ellipse, QBrush(), QPen(), QSize(7,7)));
    m_dataCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    m_dataCurve->attach(m_ui.dataPlot);
  }

  std::string ALCDataLoadingView::firstRun() const
  {
    if (m_ui.firstRun->isValid())
    {
      return m_ui.firstRun->getFirstFilename().toStdString();
    }
    else
    {
      return "";
    }
  }

  std::string ALCDataLoadingView::lastRun() const
  {
    if (m_ui.lastRun->isValid())
    {
      return m_ui.lastRun->getFirstFilename().toStdString();
    }
    else
    {
      return "";
    }
  }

  std::string ALCDataLoadingView::log() const
  {
    return m_ui.log->currentText().toStdString();
  }

  std::string ALCDataLoadingView::calculationType() const
  {
    // XXX: "text" property of the buttons should be set correctly, as accepted by
    //      PlotAsymmetryByLogValue
    return m_ui.calculationType->checkedButton()->text().toStdString();
  }

  boost::optional< std::pair<double,double> > ALCDataLoadingView::timeRange() const
  {
    if (m_ui.timeLimit->isChecked())
    {
      auto range = std::make_pair(m_ui.minTime->value(), m_ui.maxTime->value());
      return boost::make_optional(range);
    }
    else
    {
      return boost::none;
    }
  }

  void ALCDataLoadingView::setDataCurve(const QwtData& data)
  {
    m_dataCurve->setData(data);
    m_ui.dataPlot->replot();
  }

  void ALCDataLoadingView::displayError(const std::string& error)
  {
    QMessageBox::critical(m_widget, "Loading error", QString::fromStdString(error));
  }

  void ALCDataLoadingView::setAvailableLogs(const std::vector<std::string>& logs)
  {
    // Clear previous log list
    m_ui.log->clear();

    // Add new items
    for (auto it = logs.begin(); it != logs.end(); ++it)
    {
      m_ui.log->addItem(QString::fromStdString(*it));
    }
  }

  void ALCDataLoadingView::setWaitingCursor()
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);
  }

  void ALCDataLoadingView::restoreCursor()
  {
    QApplication::restoreOverrideCursor();
  }

} // namespace CustomInterfaces
} // namespace MantidQt

#include "MantidQtCustomInterfaces/Muon/ALCDataLoadingPresenter.h"

#include "MantidAPI/AlgorithmManager.h"

#include "MantidQtCustomInterfaces/Muon/ALCHelper.h"
#include "MantidQtCustomInterfaces/Muon/MuonAnalysisHelper.h"
#include "MantidQtAPI/AlgorithmInputHistory.h"

#include <QApplication>
#include <QFileInfo>
#include <QDir>

using namespace Mantid::Kernel;
using namespace Mantid::API;

using namespace MantidQt::API;

namespace MantidQt
{
namespace CustomInterfaces
{
  ALCDataLoadingPresenter::ALCDataLoadingPresenter(IALCDataLoadingView* view)
    : m_view(view)
  {}

  void ALCDataLoadingPresenter::initialize()
  {
    m_view->initialize();

    connect(m_view, SIGNAL(loadRequested()), SLOT(load()));
    connect(m_view, SIGNAL(firstRunSelected()), SLOT(updateAvailableLogs()));
  }

  void ALCDataLoadingPresenter::load()
  {
    m_view->setWaitingCursor();

    try
    {
      IAlgorithm_sptr alg = AlgorithmManager::Instance().create("PlotAsymmetryByLogValue");
      alg->setChild(true); // Don't want workspaces in the ADS
      alg->setProperty("FirstRun", m_view->firstRun());
      alg->setProperty("LastRun", m_view->lastRun());
      alg->setProperty("LogValue", m_view->log());
      alg->setProperty("Type", m_view->calculationType());

      // If time limiting requested, set min/max times
      if (auto timeRange = m_view->timeRange())
      {
        alg->setProperty("TimeMin", timeRange->first);
        alg->setProperty("TimeMax", timeRange->second);
      }

      alg->setPropertyValue("OutputWorkspace", "__NotUsed");
      alg->execute();

      m_loadedData = alg->getProperty("OutputWorkspace");

      assert(m_loadedData); // If errors are properly caught, shouldn't happen
      assert(m_loadedData->getNumberHistograms() == 1); // PlotAsymmetryByLogValue guarantees that

      m_view->setDataCurve(*(ALCHelper::curveDataFromWs(m_loadedData, 0)));
    }
    catch(std::exception& e)
    {
      m_view->displayError(e.what());
    }

    m_view->restoreCursor();
  }

  void ALCDataLoadingPresenter::updateAvailableLogs()
  {
    Workspace_sptr loadedWs;

    try //... to load the first run
    {
      IAlgorithm_sptr load = AlgorithmManager::Instance().create("LoadMuonNexus");
      load->setChild(true); // Don't want workspaces in the ADS
      load->setProperty("Filename", m_view->firstRun());
      // Don't load any data - we need logs only
      load->setPropertyValue("SpectrumMin","0");
      load->setPropertyValue("SpectrumMax","0");
      load->setPropertyValue("OutputWorkspace", "__NotUsed");
      load->execute();

      loadedWs = load->getProperty("OutputWorkspace");
    }
    catch(...)
    {
      m_view->setAvailableLogs(std::vector<std::string>()); // Empty logs list
      return;
    }

    MatrixWorkspace_const_sptr ws = MuonAnalysisHelper::firstPeriod(loadedWs);
    std::vector<std::string> logs;

    const auto& properties = ws->run().getProperties();
    for(auto it = properties.begin(); it != properties.end(); ++it)
    {
      logs.push_back((*it)->name());
    }

    m_view->setAvailableLogs(logs);
  }

} // namespace CustomInterfaces
} // namespace MantidQt

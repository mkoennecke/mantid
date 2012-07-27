/*WIKI* 


This algorithm is used in the Paraview event nexus loader to both load an event nexus file and convert it into a [[MDEventWorkspace]] for use in visualization.

The [[LoadEventNexus]] algorithm is called with default parameters to load into an [[EventWorkspace]].

After, the [[MakeDiffractionMDEventWorkspace]] algorithm is called with the new EventWorkspace as input. The parameters are set to convert to Q in the lab frame, with Lorentz correction, and default size/splitting behavior parameters.


*WIKI*/
#include "MantidMDEvents/OneStepMDEW.h"
#include "MantidKernel/System.h"
#include "MantidAPI/FileProperty.h"
#include "MantidDataObjects/EventWorkspace.h"
#include "MantidAPI/IMDEventWorkspace.h"
//#include "MantidNexus/LoadEventNexus.h"
//#include "MantidMDEvents/ConvertToDiffractionMDWorkspace.h"

namespace Mantid
{
  namespace MDEvents
  {

    // Register the algorithm into the AlgorithmFactory
    DECLARE_ALGORITHM(OneStepMDEW)

    using namespace Mantid::Kernel;
    using namespace Mantid::API;
    using namespace Mantid::DataObjects;


    //----------------------------------------------------------------------------------------------
    /** Constructor
    */
    OneStepMDEW::OneStepMDEW()
    {
    }

    //----------------------------------------------------------------------------------------------
    /** Destructor
    */
    OneStepMDEW::~OneStepMDEW()
    {
    }


    //----------------------------------------------------------------------------------------------
    /// Sets documentation strings for this algorithm
    void OneStepMDEW::initDocs()
    {
      this->setWikiSummary("Create a MDEventWorkspace in one step from a EventNexus file. For use by Paraview loader.");
      this->setOptionalMessage("Create a MDEventWorkspace in one step from a EventNexus file. For use by Paraview loader.");
    }

    //----------------------------------------------------------------------------------------------
    /** Initialize the algorithm's properties.
    */
    void OneStepMDEW::init()
    {
      this->declareProperty(new FileProperty("Filename", "", FileProperty::Load, ".nxs"),
        "The name (including its full or relative path) of the Nexus file to\n"
        "attempt to load. The file extension must either be .nxs or .NXS" );

      this->declareProperty(new WorkspaceProperty<Workspace>("OutputWorkspace","",Direction::Output),
        "Name of the output MDEventWorkspace.");
    }

    //----------------------------------------------------------------------------------------------
    /** Execute the algorithm.
    */
    void OneStepMDEW::exec()
    {
      std::string tempWsName = getPropertyValue("OutputWorkspace") + "_nxs";

      Algorithm_sptr childAlg;

      // -------- First we load the event nexus file -------------
      childAlg = AlgorithmFactory::Instance().create("LoadEventNexus", 1); // new Mantid::NeXus::LoadEventNexus();
      childAlg->initialize();
      childAlg->setPropertyValue("Filename", getPropertyValue("Filename"));
      childAlg->setPropertyValue("OutputWorkspace", tempWsName);
      childAlg->executeAsSubAlg();

      //    Workspace_sptr tempWS = childAlg->getProperty<Workspace>("OutputWorkspace");
      //    IEventWorkspace_sptr tempEventWS = AnalysisDataService::Instance().retrieveWS<IEventWorkspace>(tempWsName);
      //    IEventWorkspace_sptr tempEventWS = AnalysisDataService::Instance().retrieveWS<IEventWorkspace>(tempWsName);


      // --------- Now Convert -------------------------------
      //childAlg = createSubAlgorithm("ConvertToDiffractionMDWorkspace");
      childAlg = AlgorithmFactory::Instance().create("ConvertToDiffractionMDWorkspace", 1);  // new ConvertToDiffractionMDWorkspace();
      childAlg->initialize();
      childAlg->setPropertyValue("InputWorkspace", tempWsName);
      childAlg->setProperty<bool>("ClearInputWorkspace", false);
      childAlg->setProperty<bool>("LorentzCorrection", true);
      childAlg->setPropertyValue("OutputWorkspace", getPropertyValue("OutputWorkspace"));
      childAlg->executeAsSubAlg();

      //    Workspace_sptr tempWS = childAlg->getProperty("OutputWorkspace");
      //    IMDEventWorkspace_sptr outWS = boost::dynamic_pointer_cast<IMDEventWorkspace>(tempWS);
      IMDEventWorkspace_sptr outWS = boost::dynamic_pointer_cast<IMDEventWorkspace>(
        AnalysisDataService::Instance().retrieve(getPropertyValue("OutputWorkspace")));

      setProperty<Workspace_sptr>("OutputWorkspace", outWS);
    }



  } // namespace Mantid
} // namespace MDEvents


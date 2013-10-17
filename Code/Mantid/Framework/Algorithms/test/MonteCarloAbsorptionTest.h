#ifndef MONTECARLOABSORPTIONTEST_H_
#define MONTECARLOABSORPTIONTEST_H_

#include "MantidAlgorithms/MonteCarloAbsorption.h"
#include "MantidAPI/SampleEnvironment.h"
#include "MantidGeometry/Objects/ShapeFactory.h"
#include "MantidKernel/UnitFactory.h"
#include "MantidKernel/PhysicalConstants.h"
#include <cxxtest/TestSuite.h>

#include "MantidTestHelpers/WorkspaceCreationHelper.h"
#include "MantidTestHelpers/ComponentCreationHelper.h"

using namespace Mantid;
using namespace Mantid::API;
using namespace Mantid::Geometry;
using namespace Mantid::Kernel;

using Mantid::Algorithms::MonteCarloAbsorption;
using Mantid::DataObjects::Workspace2D_sptr;
using Mantid::Geometry::ShapeFactory;
using Mantid::Kernel::V3D;

class MonteCarloAbsorptionTest : public CxxTest::TestSuite
{
public:
  
  void test_That_Workspace_With_No_Instrument_Is_Not_Accepted()
  {
    IAlgorithm_sptr mcAbsorb = createAlgorithm();
    // Create a simple test workspace that has no instrument
    Workspace2D_sptr testWS = WorkspaceCreationHelper::Create2DWorkspace(10, 5);
    
    TS_ASSERT_THROWS(mcAbsorb->setProperty<MatrixWorkspace_sptr>("InputWorkspace", testWS), 
		     std::invalid_argument);
  }

   void test_That_Workspace_With_An_Invalid_Sample_Shape_Is_Not_Accepted()
   {
     Workspace2D_sptr testWS = WorkspaceCreationHelper::create2DWorkspaceWithFullInstrument(9, 10);
     // Needs to have units of wavelength
     testWS->getAxis(0)->unit() = Mantid::Kernel::UnitFactory::Instance().create("Wavelength");
     
     IAlgorithm_sptr mcAbsorb = createAlgorithm();
     TS_ASSERT_THROWS_NOTHING(mcAbsorb->setProperty<MatrixWorkspace_sptr>("InputWorkspace", testWS));
     const std::string outputName("mctest-workspace");
     TS_ASSERT_THROWS_NOTHING(mcAbsorb->setPropertyValue("OutputWorkspace",outputName));
     TS_ASSERT_THROWS(mcAbsorb->execute(), std::invalid_argument);
   }

  void test_That_Workspace_With_A_Correctly_Defined_Sample_Shape_And_Material_Succeeds()
  {
    const std::string inputName("mcabsorb-input");
    setUpWS(inputName);

    // Run the algorithm
    IAlgorithm_sptr mcAbsorb = createAlgorithm();
    TS_ASSERT_THROWS_NOTHING(mcAbsorb->setPropertyValue("InputWorkspace", inputName));
    const std::string outputName("mcabsorb-factors");
    TS_ASSERT_THROWS_NOTHING(mcAbsorb->setPropertyValue("OutputWorkspace",outputName));
    mcAbsorb->execute();
    TS_ASSERT_THROWS_NOTHING(mcAbsorb->execute());

    AnalysisDataServiceImpl & dataStore = AnalysisDataService::Instance();
    MatrixWorkspace_sptr factorWS = 
      boost::dynamic_pointer_cast<MatrixWorkspace>(dataStore.retrieve(outputName));

    TS_ASSERT(factorWS);
    if( !factorWS ) TS_FAIL("Cannot retrieve output workspace");

    const size_t nbins(factorWS->blocksize());

    // Pick out some random values
    const double delta(1e-08);
    const size_t middle_index = (nbins/2) - 1;
    TS_ASSERT_DELTA(factorWS->readY(0).front(), 0.00477768, delta);
    TS_ASSERT_DELTA(factorWS->readY(0)[middle_index], 0.000134089, delta);
    TS_ASSERT_DELTA(factorWS->readY(0).back(), 1.18e-7, delta);
        
    // Different spectra
    TS_ASSERT_DELTA(factorWS->readY(4).front(), 0.00868274, delta);
    TS_ASSERT_DELTA(factorWS->readY(4)[middle_index], 0.000177871, delta);
    TS_ASSERT_DELTA(factorWS->readY(4).back(), 0.000131046, delta);

    TS_ASSERT_DELTA(factorWS->readY(8).front(), 0.00654336, delta);
    TS_ASSERT_DELTA(factorWS->readY(8)[middle_index], 0.000224293, delta);
    TS_ASSERT_DELTA(factorWS->readY(8).back(), 1.14231e-5, delta);

    dataStore.remove(inputName);
    dataStore.remove(outputName);
  }

  void test_That_Workspace_With_A_Defined_Sample_Shape_And_Container_Succeeds()
  {
    AnalysisDataServiceImpl & dataStore = AnalysisDataService::Instance();
    const std::string inputName("mcabsorb-input");
    setUpWS(inputName, 1, 10, true);

    // Run the algorithm
    IAlgorithm_sptr mcAbsorb = createAlgorithm();
    TS_ASSERT_THROWS_NOTHING(mcAbsorb->setPropertyValue("InputWorkspace", inputName));
    const std::string outputName("mcabsorb-factors");
    TS_ASSERT_THROWS_NOTHING(mcAbsorb->setPropertyValue("OutputWorkspace",outputName));
    mcAbsorb->execute();
    TS_ASSERT_THROWS_NOTHING(mcAbsorb->execute());


    MatrixWorkspace_sptr factorWS = 
      boost::dynamic_pointer_cast<MatrixWorkspace>(dataStore.retrieve(outputName));

    TS_ASSERT(factorWS);
    if( !factorWS ) TS_FAIL("Cannot retrieve output workspace");

    const size_t nbins(factorWS->blocksize());

    // Pick out some random values
    const double delta(1e-08);
    const size_t middle_index = (nbins/2) - 1;
    TS_ASSERT_DELTA(factorWS->readY(0).front(), 1.9588e-05, delta);
    TS_ASSERT_DELTA(factorWS->readY(0)[middle_index], 6.56478e-09, 1e-9);
    TS_ASSERT_DELTA(factorWS->readY(0).back(), 2.3604e-11, 1e-12);

    dataStore.remove(inputName);
    dataStore.remove(outputName);
  }


private:
  
  void setUpWS(const std::string & name, const int nspectra = 9, const int nbins = 10,
	       bool addContainer = false)
  {
    Workspace2D_sptr space = WorkspaceCreationHelper::create2DWorkspaceWithFullInstrument(nspectra, nbins);
    // Needs to have units of wavelength
    space->getAxis(0)->unit() = Mantid::Kernel::UnitFactory::Instance().create("Wavelength");
    
    // Define a sample shape
    Object_sptr sampleShape = ComponentCreationHelper::createSphere(0.1, V3D(), "sample-sphere");
    space->mutableSample().setShape(*sampleShape);
    // And a material
    Material vanadium("Vanadium", PhysicalConstants::getNeutronAtom(23,0), 0.072);
    space->mutableSample().setMaterial(vanadium);

    if( addContainer )
    {
      const std::string id("container");
      const double radius(0.25);
      const double height(0.4);
      const V3D baseCentre(0.0, -height/2.0,0.0);
      const V3D axis(0.0, 1.0,0.0);
      
      // Define a container shape. Use a simple cylinder
      std::ostringstream xml;
      xml << "<cylinder id=\"" << id << "\">" 
	  << "<centre-of-bottom-base x=\"" << baseCentre.X() << "\" y=\"" << baseCentre.Y() << "\" z=\"" << baseCentre.Z() << "\"/>"
	  << "<axis x=\"" << axis.X() << "\" y=\"" << axis.Y() << "\" z=\"" << axis.Z() << "\"/>"
	  << "<radius val=\"" << radius << "\" />"
	  << "<height val=\"" << height << "\" />"
	  << "</cylinder>";
      
      ShapeFactory shapeMaker;
      Object_sptr containerShape = shapeMaker.createShape(xml.str());
      Material_sptr canMaterial(new Material("CanMaterial", PhysicalConstants::getNeutronAtom(26, 0), 0.01));
      SampleEnvironment *can = new SampleEnvironment("can");
      can->add(new ObjComponent("1", containerShape, NULL, canMaterial));
      space->mutableSample().setEnvironment(can);
    }

    // Register the workspace in the data service
    AnalysisDataService::Instance().add(name, space);
  }

  IAlgorithm_sptr createAlgorithm()
  {
    IAlgorithm_sptr mcAbsorb = boost::shared_ptr<IAlgorithm>(new MonteCarloAbsorption());
    TS_ASSERT_THROWS_NOTHING(mcAbsorb->initialize());
    TS_ASSERT_EQUALS(mcAbsorb->isInitialized(), true);
    mcAbsorb->setRethrows(true);
    return mcAbsorb;
  }

};

#endif

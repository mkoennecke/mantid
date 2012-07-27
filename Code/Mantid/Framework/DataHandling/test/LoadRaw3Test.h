#ifndef LoadRaw3TEST_H_
#define LoadRaw3TEST_H_

#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/FrameworkManager.h"
#include "MantidAPI/SpectraDetectorMap.h"
#include "MantidAPI/WorkspaceFactory.h"
#include "MantidAPI/WorkspaceGroup.h"
#include "MantidDataHandling/LoadRaw3.h"
#include "MantidDataObjects/ManagedWorkspace2D.h"
#include "MantidGeometry/Instrument.h"
#include "MantidKernel/ConfigService.h"
#include "MantidKernel/TimeSeriesProperty.h"
#include <cxxtest/TestSuite.h>

using namespace Mantid;
using namespace Mantid::API;
using namespace Mantid::DataHandling;
using namespace Mantid::DataObjects;
using namespace Mantid::Geometry;
using namespace Mantid::Kernel;

class LoadRaw3Test : public CxxTest::TestSuite
{
public:
  static LoadRaw3Test *createSuite() { return new LoadRaw3Test(); }
  static void destroySuite(LoadRaw3Test *suite) { delete suite; }

  LoadRaw3Test()
  {
    // Path to test input file assumes Test directory checked out from SVN
    inputFile = "HET15869.raw";
  }

  void testInit()
  {
    TS_ASSERT_THROWS_NOTHING( loader.initialize());
    TS_ASSERT( loader.isInitialized() );
  }

  void testExec()
  {
	 	 
	if ( !loader.isInitialized() ) loader.initialize();

    // Should fail because mandatory parameter has not been set
    TS_ASSERT_THROWS(loader.execute(),std::runtime_error);

    // Now set it...
    loader.setPropertyValue("Filename", inputFile);
	loader.setPropertyValue("LoadMonitors", "Include");

    outputSpace = "outer";
    loader.setPropertyValue("OutputWorkspace", outputSpace);

    TS_ASSERT_THROWS_NOTHING(loader.execute());
    TS_ASSERT( loader.isExecuted() );

    // Get back the saved workspace
    Workspace_sptr output;
    TS_ASSERT_THROWS_NOTHING(output = AnalysisDataService::Instance().retrieve(outputSpace));
    Workspace2D_sptr output2D = boost::dynamic_pointer_cast<Workspace2D>(output);
    // Should be 2584 for file HET15869.RAW
    TS_ASSERT_EQUALS( output2D->getNumberHistograms(), 2584);
    // Check two X vectors are the same
    TS_ASSERT( (output2D->dataX(99)) == (output2D->dataX(1734)) );
    // Check two Y arrays have the same number of elements
    TS_ASSERT_EQUALS( output2D->dataY(673).size(), output2D->dataY(2111).size() );
    // Check one particular value
    TS_ASSERT_EQUALS( output2D->dataY(999)[777], 9);
    // Check that the error on that value is correct
    TS_ASSERT_EQUALS( output2D->dataE(999)[777], 3);
    // Check that the error on that value is correct
    TS_ASSERT_EQUALS( output2D->dataX(999)[777], 554.1875);

    // Check the unit has been set correctly
    TS_ASSERT_EQUALS( output2D->getAxis(0)->unit()->unitID(), "TOF" )
    TS_ASSERT( ! output2D-> isDistribution() )

    // Check the proton charge has been set correctly
    TS_ASSERT_DELTA( output2D->run().getProtonCharge(), 171.0353, 0.0001 )

    //----------------------------------------------------------------------
     //Tests taken from LoadInstrumentTest to check sub-algorithm is running properly
  //  ----------------------------------------------------------------------
    boost::shared_ptr<const Instrument> i = output2D->getInstrument();
    boost::shared_ptr<const Mantid::Geometry::IComponent> source = i->getSource();

    TS_ASSERT_EQUALS( source->getName(), "undulator");
    TS_ASSERT_DELTA( source->getPos().Y(), 0.0,0.01);

    boost::shared_ptr<const Mantid::Geometry::IComponent> samplepos = i->getSample();
    TS_ASSERT_EQUALS( samplepos->getName(), "nickel-holder");
    TS_ASSERT_DELTA( samplepos->getPos().Z(), 0.0,0.01);

    boost::shared_ptr<const Mantid::Geometry::Detector> ptrDet103 = boost::dynamic_pointer_cast<const Mantid::Geometry::Detector>(i->getDetector(103));
    TS_ASSERT_EQUALS( ptrDet103->getID(), 103);
    TS_ASSERT_EQUALS( ptrDet103->getName(), "pixel");
    TS_ASSERT_DELTA( ptrDet103->getPos().X(), 0.4013,0.01);
    TS_ASSERT_DELTA( ptrDet103->getPos().Z(), 2.4470,0.01);

    //----------------------------------------------------------------------
    // Test code copied from LoadLogTest to check sub-algorithm is running properly
    //----------------------------------------------------------------------
   // boost::shared_ptr<Sample> sample = output2D->getSample();
    Property *l_property = output2D->run().getLogData( std::string("TEMP1") );
    TimeSeriesProperty<double> *l_timeSeriesDouble = dynamic_cast<TimeSeriesProperty<double>*>(l_property);
    std::string timeSeriesString = l_timeSeriesDouble->value();
    TS_ASSERT_EQUALS( timeSeriesString.substr(0,23), "2007-Nov-13 15:16:20  0" );

    l_property = output2D->run().getLogData( "run_number" );
    TS_ASSERT_EQUALS( l_property->value(), "15869" );

    //----------------------------------------------------------------------
    // Tests to check that Loading SpectraDetectorMap is done correctly
    //----------------------------------------------------------------------
    // Test one to one mapping, for example spectra 6 has only 1 pixel
    TS_ASSERT_EQUALS( output2D->getSpectrum(6)->getDetectorIDs().size(), 1);   // rummap.ndet(6),1);

    // Test one to many mapping, for example 10 pixels contribute to spectra 2084 (workspace index 2083)
    TS_ASSERT_EQUALS( output2D->getSpectrum(2083)->getDetectorIDs().size(), 10);   //map.ndet(2084),10);

    // Check the id number of all pixels contributing
    std::set<detid_t> detectorgroup;
    detectorgroup = output2D->getSpectrum(2083)->getDetectorIDs();
    std::set<detid_t>::const_iterator it;
    int pixnum=101191;
    for (it=detectorgroup.begin();it!=detectorgroup.end();it++)
      TS_ASSERT_EQUALS(*it,pixnum++);

	AnalysisDataService::Instance().remove(outputSpace);
  }

 void testMixedLimits()
  {
	 if ( !loader2.isInitialized() ) loader2.initialize();

    loader2.setPropertyValue("Filename", inputFile);
    loader2.setPropertyValue("OutputWorkspace", "outWS");
    loader2.setPropertyValue("SpectrumList", "998,999,1000");
    loader2.setPropertyValue("SpectrumMin", "5");
    loader2.setPropertyValue("SpectrumMax", "10");

    TS_ASSERT_THROWS_NOTHING(loader2.execute());
    TS_ASSERT( loader2.isExecuted() );

    // Get back the saved workspace
    Workspace_sptr output;
    TS_ASSERT_THROWS_NOTHING(output = AnalysisDataService::Instance().retrieve("outWS"));
    Workspace2D_sptr output2D = boost::dynamic_pointer_cast<Workspace2D>(output);

    // Should be 6 for selected input
    TS_ASSERT_EQUALS( output2D->getNumberHistograms(), 9);

    // Check two X vectors are the same
    TS_ASSERT( (output2D->dataX(1)) == (output2D->dataX(5)) );

    // Check two Y arrays have the same number of elements
    TS_ASSERT_EQUALS( output2D->dataY(2).size(), output2D->dataY(7).size() );

    // Check one particular value
    TS_ASSERT_EQUALS( output2D->dataY(8)[777], 9);
    // Check that the error on that value is correct
    TS_ASSERT_EQUALS( output2D->dataE(8)[777], 3);
    // Check that the error on that value is correct
    TS_ASSERT_EQUALS( output2D->dataX(8)[777], 554.1875);
  }

  void testMinlimit()
  {
    LoadRaw3 alg;
    std::string outWS = "outWSLimitTest";
    if ( !alg.isInitialized() ) alg.initialize();

    alg.setPropertyValue("Filename", inputFile);
    alg.setPropertyValue("OutputWorkspace", outWS);
    alg.setPropertyValue("SpectrumMin", "2580");

    TS_ASSERT_THROWS_NOTHING(alg.execute());
    TS_ASSERT( alg.isExecuted() );

    // Get back the saved workspace
    Workspace_sptr output;
    TS_ASSERT_THROWS_NOTHING(output = AnalysisDataService::Instance().retrieve(outWS));
    Workspace2D_sptr output2D = boost::dynamic_pointer_cast<Workspace2D>(output);

    TS_ASSERT_EQUALS( output2D->getNumberHistograms(), 5);
    AnalysisDataService::Instance().remove(outWS);
  }

  void testMaxlimit()
  {
    LoadRaw3 alg;
    std::string outWS = "outWSLimitTest";
    if ( !alg.isInitialized() ) alg.initialize();

    alg.setPropertyValue("Filename", inputFile);
    alg.setPropertyValue("OutputWorkspace", outWS);
    alg.setPropertyValue("SpectrumMax", "5");

    TS_ASSERT_THROWS_NOTHING(alg.execute());
    TS_ASSERT( alg.isExecuted() );

    // Get back the saved workspace
    Workspace_sptr output;
    TS_ASSERT_THROWS_NOTHING(output = AnalysisDataService::Instance().retrieve(outWS));
    Workspace2D_sptr output2D = boost::dynamic_pointer_cast<Workspace2D>(output);

    TS_ASSERT_EQUALS( output2D->getNumberHistograms(), 5);
    AnalysisDataService::Instance().remove(outWS);
  }

  void testMinMaxlimit()
  {
    LoadRaw3 alg;
    std::string outWS = "outWSLimitTest";
    if ( !alg.isInitialized() ) alg.initialize();

    alg.setPropertyValue("Filename", inputFile);
    alg.setPropertyValue("OutputWorkspace", outWS);
    alg.setPropertyValue("SpectrumMin", "5");
    alg.setPropertyValue("SpectrumMax", "10");

    TS_ASSERT_THROWS_NOTHING(alg.execute());
    TS_ASSERT( alg.isExecuted() );

    // Get back the saved workspace
    Workspace_sptr output;
    TS_ASSERT_THROWS_NOTHING(output = AnalysisDataService::Instance().retrieve(outWS));
    Workspace2D_sptr output2D = boost::dynamic_pointer_cast<Workspace2D>(output);

    TS_ASSERT_EQUALS( output2D->getNumberHistograms(), 6);
    TS_ASSERT_EQUALS( output2D->getSpectrum(0)->getSpectrumNo(), 5);
    TS_ASSERT_EQUALS( output2D->getSpectrum(1)->getSpectrumNo(), 6);
    TS_ASSERT(        output2D->getSpectrum(1)->hasDetectorID(4103));
    TS_ASSERT_EQUALS( output2D->getSpectrum(5)->getSpectrumNo(), 10);
    TS_ASSERT(        output2D->getSpectrum(5)->hasDetectorID(4107));
    AnalysisDataService::Instance().remove(outWS);
  }

  void testListlimit()
  {
	 LoadRaw3 alg;
    std::string outWS = "outWSLimitTest";
    if ( !alg.isInitialized() ) alg.initialize();

    alg.setPropertyValue("Filename", inputFile);
    alg.setPropertyValue("OutputWorkspace", outWS);
    alg.setPropertyValue("SpectrumList", "998,999,1000");

    TS_ASSERT_THROWS_NOTHING(alg.execute());
    TS_ASSERT( alg.isExecuted() );

    // Get back the saved workspace
    Workspace_sptr output;
    TS_ASSERT_THROWS_NOTHING(output = AnalysisDataService::Instance().retrieve(outWS));
    Workspace2D_sptr output2D = boost::dynamic_pointer_cast<Workspace2D>(output);

    TS_ASSERT_EQUALS( output2D->getNumberHistograms(), 3);
    AnalysisDataService::Instance().remove(outWS);
  }

  void testfail()
  {
	  LoadRaw3 loader3;
    if ( !loader3.isInitialized() ) loader3.initialize();
    std::string outWS="LoadRaw3-out2";
    loader3.setPropertyValue("Filename", inputFile);
    loader3.setPropertyValue("OutputWorkspace",outWS );
    loader3.setPropertyValue("SpectrumList", "0,999,1000");
    loader3.setPropertyValue("SpectrumMin", "5");
    loader3.setPropertyValue("SpectrumMax", "10");
    loader3.execute();
    Workspace_sptr output;
    // test that there is no workspace as it should have failed
    TS_ASSERT_THROWS(output = AnalysisDataService::Instance().retrieve(outWS),std::runtime_error);

    loader3.setPropertyValue("SpectrumMin", "5");
    loader3.setPropertyValue("SpectrumMax", "1");
     loader3.execute();
    TS_ASSERT_THROWS(output = AnalysisDataService::Instance().retrieve(outWS),std::runtime_error);

    loader3.setPropertyValue("SpectrumMin", "5");
    loader3.setPropertyValue("SpectrumMax", "3");
    loader3.execute();
    TS_ASSERT_THROWS(output = AnalysisDataService::Instance().retrieve(outWS),std::runtime_error);

    loader3.setPropertyValue("SpectrumMin", "5");
    loader3.setPropertyValue("SpectrumMax", "5");
    loader3.execute();
    TS_ASSERT_THROWS(output = AnalysisDataService::Instance().retrieve(outWS),std::runtime_error);

    loader3.setPropertyValue("SpectrumMin", "5");
    loader3.setPropertyValue("SpectrumMax", "3000");
    loader3.execute();
    TS_ASSERT_THROWS(output = AnalysisDataService::Instance().retrieve(outWS),std::runtime_error);

    loader3.setPropertyValue("SpectrumMin", "5");
    loader3.setPropertyValue("SpectrumMax", "10");
    loader3.setPropertyValue("SpectrumList", "999,3000");
    loader3.execute();
    TS_ASSERT_THROWS(output = AnalysisDataService::Instance().retrieve(outWS),std::runtime_error);

    loader3.setPropertyValue("SpectrumList", "999,2000");
    loader3.execute();
    TS_ASSERT_THROWS_NOTHING(output = AnalysisDataService::Instance().retrieve(outWS));
	AnalysisDataService::Instance().remove(outWS);
  }

  void testMultiPeriod()
  {
    LoadRaw3 loader5;
    loader5.initialize();
    loader5.setPropertyValue("Filename", "CSP78173.raw");
    loader5.setPropertyValue("OutputWorkspace", "multiperiod");
    // loader5.setPropertyValue("SpectrumList", "0,1,2,3");
    
    TS_ASSERT_THROWS_NOTHING( loader5.execute() )
    TS_ASSERT( loader5.isExecuted() )
	
    WorkspaceGroup_sptr work_out;
    TS_ASSERT_THROWS_NOTHING(work_out = AnalysisDataService::Instance().retrieveWS<WorkspaceGroup>("multiperiod"));
	
    Workspace_sptr wsSptr=AnalysisDataService::Instance().retrieve("multiperiod");
    WorkspaceGroup_sptr sptrWSGrp=boost::dynamic_pointer_cast<WorkspaceGroup>(wsSptr);
    std::vector<std::string>wsNamevec;
    wsNamevec=sptrWSGrp->getNames();
    int period=1;
    std::vector<std::string>::const_iterator it=wsNamevec.begin();
    for (;it!=wsNamevec.end();it++)
    {	std::stringstream count;
      count <<period;
      std::string wsName="multiperiod_"+count.str();
      TS_ASSERT_EQUALS(*it,wsName)
      period++;
    }
    std::vector<std::string>::const_iterator itr1=wsNamevec.begin();
    int periodNumber = 0;
    const int nHistograms = 4;
    for (;itr1!=wsNamevec.end();itr1++)
    {	
      MatrixWorkspace_sptr  outsptr;
      TS_ASSERT_THROWS_NOTHING(outsptr=AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>((*itr1)));
      doTestMultiPeriodWorkspace(outsptr, nHistograms, ++periodNumber);
    }
    std::vector<std::string>::const_iterator itr=wsNamevec.begin();
    MatrixWorkspace_sptr  outsptr1;
    TS_ASSERT_THROWS_NOTHING(outsptr1=AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>((*itr)));
    MatrixWorkspace_sptr  outsptr2;
    TS_ASSERT_THROWS_NOTHING(outsptr2=AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>((*++itr)));

		
    TS_ASSERT_EQUALS( outsptr1->dataX(0), outsptr2->dataX(0) )

    // But the data should be different
    TS_ASSERT_DIFFERS( outsptr1->dataY(1)[8], outsptr2->dataY(1)[8] )

    TS_ASSERT_EQUALS( outsptr1->getInstrument()->baseInstrument(), outsptr2->getInstrument()->baseInstrument() )
    TS_ASSERT_EQUALS( &(outsptr1->sample()), &(outsptr2->sample()) )
    TS_ASSERT_DIFFERS( &(outsptr1->run()), &(outsptr2->run()))

	itr1=wsNamevec.begin();
    for (;itr1!=wsNamevec.end();++itr1)
    {	
      AnalysisDataService::Instance().remove(*itr);

    }
	
  }

  // test if parameters set in instrument definition file are loaded properly
  void testIfParameterFromIDFLoaded()
  {
	 LoadRaw3 loader4;
    loader4.initialize();
    loader4.setPropertyValue("Filename", "TSC10076.raw");
    loader4.setPropertyValue("OutputWorkspace", "parameterIDF");
    TS_ASSERT_THROWS_NOTHING( loader4.execute() )
    TS_ASSERT( loader4.isExecuted() )

    // Get back workspace and check it really is a ManagedWorkspace2D
    Workspace_sptr output;
    TS_ASSERT_THROWS_NOTHING( output = AnalysisDataService::Instance().retrieve("parameterIDF") );

    Workspace2D_sptr output2D = boost::dynamic_pointer_cast<Workspace2D>(output);

    boost::shared_ptr<const Instrument> i = output2D->getInstrument();
    Mantid::Geometry::IDetector_const_sptr ptrDet = i->getDetector(60);
    TS_ASSERT_EQUALS( ptrDet->getID(), 60);

    Mantid::Geometry::ParameterMap& pmap = output2D->instrumentParameters();
    TS_ASSERT_EQUALS( static_cast<int>(pmap.size()), 155);
	AnalysisDataService::Instance().remove("parameterIDF");
  }

  void testTwoTimeRegimes()
  {
    LoadRaw3 loader5;
    loader5.initialize();
    loader5.setPropertyValue("Filename", "IRS38633.raw");
    loader5.setPropertyValue("OutputWorkspace", "twoRegimes");
    loader5.setPropertyValue("SpectrumList", "2,3");
    loader5.setPropertyValue("LoadMonitors", "Include");
    TS_ASSERT_THROWS_NOTHING( loader5.execute() )
    TS_ASSERT( loader5.isExecuted() )

    MatrixWorkspace_const_sptr output;
    TS_ASSERT( output = boost::dynamic_pointer_cast<MatrixWorkspace>
      (AnalysisDataService::Instance().retrieve("twoRegimes")) )
    // Shift should be 3300 - check a couple of values
    TS_ASSERT_EQUALS( output->readX(0).front()+3300, output->readX(1).front() )
    TS_ASSERT_EQUALS( output->readX(0).back()+3300, output->readX(1).back() )

    AnalysisDataService::Instance().remove("twoRegimes");
  }
  void testSeparateMonitors()
  {
    LoadRaw3 loader6;
    if ( !loader6.isInitialized() ) loader6.initialize();

    // Should fail because mandatory parameter has not been set
    TS_ASSERT_THROWS(loader6.execute(),std::runtime_error);

    // Now set it...
    loader6.setPropertyValue("Filename", inputFile);
    loader6.setPropertyValue("LoadMonitors", "Separate");

    outputSpace = "outer1";
    loader6.setPropertyValue("OutputWorkspace", outputSpace);

    TS_ASSERT_THROWS_NOTHING(loader6.execute());
    TS_ASSERT( loader6.isExecuted() );

    // Get back the saved workspace
    Workspace_sptr output;
    TS_ASSERT_THROWS_NOTHING(output = AnalysisDataService::Instance().retrieve(outputSpace));
    Workspace2D_sptr output2D = boost::dynamic_pointer_cast<Workspace2D>(output);

    Workspace_sptr monitoroutput;
    TS_ASSERT_THROWS_NOTHING(monitoroutput = AnalysisDataService::Instance().retrieve(outputSpace+"_Monitors"));
    Workspace2D_sptr monitoroutput2D = boost::dynamic_pointer_cast<Workspace2D>(monitoroutput);
    // Should be 2584 for file HET15869.RAW
    TS_ASSERT_EQUALS( output2D->getNumberHistograms(), 2580);

    TS_ASSERT_EQUALS( monitoroutput2D->getNumberHistograms(), 4);

    TS_ASSERT( monitoroutput2D->getSpectrum(0)->hasDetectorID(601));
    TS_ASSERT( monitoroutput2D->getSpectrum(1)->hasDetectorID(602));

    //Check two X vectors are the same
    TS_ASSERT( (output2D->dataX(95)) == (output2D->dataX(1730)) );
    // Check two Y arrays have the same number of elements
    TS_ASSERT_EQUALS( output2D->dataY(669).size(), output2D->dataY(2107).size() );
    // Check one particular value
    TS_ASSERT_EQUALS( output2D->dataY(995)[0],1);
    // Check that the error on that value is correct
    TS_ASSERT_EQUALS( output2D->dataE(995)[777], 3);
    // Check that the error on that value is correct
    TS_ASSERT_EQUALS( output2D->dataX(995)[777], 554.1875);

    // Check the unit has been set correctly
    TS_ASSERT_EQUALS( output2D->getAxis(0)->unit()->unitID(), "TOF" )
    TS_ASSERT( ! output2D-> isDistribution() )

    // Check the proton charge has been set correctly
    TS_ASSERT_DELTA( output2D->run().getProtonCharge(), 171.0353, 0.0001 )

    //----------------------------------------------------------------------
    // Tests taken from LoadInstrumentTest to check sub-algorithm is running properly
    //----------------------------------------------------------------------
    boost::shared_ptr<const Instrument> i = output2D->getInstrument();
    boost::shared_ptr<const Mantid::Geometry::IComponent> source = i->getSource();

    TS_ASSERT_EQUALS( source->getName(), "undulator");
    TS_ASSERT_DELTA( source->getPos().Y(), 0.0,0.01);

    boost::shared_ptr<const Mantid::Geometry::IComponent> samplepos = i->getSample();
    TS_ASSERT_EQUALS( samplepos->getName(), "nickel-holder");
    TS_ASSERT_DELTA( samplepos->getPos().Z(), 0.0,0.01);

    boost::shared_ptr<const Mantid::Geometry::Detector> ptrDet103 = boost::dynamic_pointer_cast<const Mantid::Geometry::Detector>(i->getDetector(103));
    TS_ASSERT_EQUALS( ptrDet103->getID(), 103);
    TS_ASSERT_EQUALS( ptrDet103->getName(), "pixel");
    TS_ASSERT_DELTA( ptrDet103->getPos().X(), 0.4013,0.01);
    TS_ASSERT_DELTA( ptrDet103->getPos().Z(), 2.4470,0.01);

      ////----------------------------------------------------------------------
    // Test code copied from LoadLogTest to check sub-algorithm is running properly
    //----------------------------------------------------------------------
    Property *l_property = output2D->run().getLogData( std::string("TEMP1") );
    TimeSeriesProperty<double> *l_timeSeriesDouble = dynamic_cast<TimeSeriesProperty<double>*>(l_property);
    std::string timeSeriesString = l_timeSeriesDouble->value();
    TS_ASSERT_EQUALS( timeSeriesString.substr(0,23), "2007-Nov-13 15:16:20  0" );

    //----------------------------------------------------------------------
    // Tests to check that Loading SpectraDetectorMap is done correctly
    //----------------------------------------------------------------------
    // Test one to one mapping, for example spectra 6 has only 1 pixel
    TS_ASSERT_EQUALS( output2D->getSpectrum(6)->getDetectorIDs().size(), 1);   // rummap.ndet(6),1);

    // Test one to many mapping, for example 10 pixels contribute to spectra 2084 (workspace index 2083)
    TS_ASSERT_EQUALS( output2D->getSpectrum(2079)->getDetectorIDs().size(), 10);   //map.ndet(2084),10);

    // Check the id number of all pixels contributing
    std::set<detid_t> detectorgroup;
    detectorgroup = output2D->getSpectrum(2079)->getDetectorIDs();
    std::set<detid_t>::const_iterator it;
    int pixnum=101191;
    for (it=detectorgroup.begin();it!=detectorgroup.end();it++)
      TS_ASSERT_EQUALS(*it,pixnum++);

    AnalysisDataService::Instance().remove(outputSpace);
    AnalysisDataService::Instance().remove(outputSpace+"_Monitors");
  }

  void testSeparateMonitorsMultiPeriod()
  {
    LoadRaw3 loader7;
    loader7.initialize();
    loader7.setPropertyValue("Filename", "CSP79590.raw");
    loader7.setPropertyValue("OutputWorkspace", "multiperiod");
    loader7.setPropertyValue("LoadMonitors", "Separate");

    TS_ASSERT_THROWS_NOTHING( loader7.execute() )
    TS_ASSERT( loader7.isExecuted() )

    WorkspaceGroup_sptr work_out;
    TS_ASSERT_THROWS_NOTHING(work_out = AnalysisDataService::Instance().retrieveWS<WorkspaceGroup>("multiperiod"));

	  WorkspaceGroup_sptr monitor_work_out;
	  TS_ASSERT_THROWS_NOTHING(monitor_work_out = AnalysisDataService::Instance().retrieveWS<WorkspaceGroup>("multiperiod_Monitors"));

	  Workspace_sptr monitorwsSptr=AnalysisDataService::Instance().retrieve("multiperiod_Monitors");
	  WorkspaceGroup_sptr monitorsptrWSGrp=boost::dynamic_pointer_cast<WorkspaceGroup>(monitorwsSptr);


	  const std::vector<std::string>monitorwsNamevec = monitorsptrWSGrp->getNames();
	  int period=1;
    std::vector<std::string>::const_iterator it=monitorwsNamevec.begin();
    for (;it!=monitorwsNamevec.end();it++)
    {	std::stringstream count;
      count <<period;
      std::string wsName="multiperiod_Monitors_"+count.str();
      TS_ASSERT_EQUALS(*it,wsName)
      period++;
    }
    std::vector<std::string>::const_iterator itr1=monitorwsNamevec.begin();
    for (;itr1!=monitorwsNamevec.end();itr1++)
    {	
      MatrixWorkspace_sptr  outsptr;
      TS_ASSERT_THROWS_NOTHING(outsptr=AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>((*itr1)));
      TS_ASSERT_EQUALS( outsptr->getNumberHistograms(), 2)

    }
	 std::vector<std::string>::const_iterator monitr=monitorwsNamevec.begin();
    MatrixWorkspace_sptr  monoutsptr1;
    TS_ASSERT_THROWS_NOTHING(monoutsptr1=AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>((*monitr)));
    MatrixWorkspace_sptr  monoutsptr2;
    TS_ASSERT_THROWS_NOTHING(monoutsptr2=AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>((*++monitr)));
		
    TS_ASSERT_EQUALS( monoutsptr1->dataX(0), monoutsptr2->dataX(0) )

    // But the data should be different
    TS_ASSERT_DIFFERS( monoutsptr1->dataY(1)[555], monoutsptr2->dataY(1)[555] )

    TS_ASSERT_EQUALS( &(monoutsptr1->run()), &(monoutsptr2->run()) )
	
    Workspace_sptr wsSptr=AnalysisDataService::Instance().retrieve("multiperiod");
    WorkspaceGroup_sptr sptrWSGrp=boost::dynamic_pointer_cast<WorkspaceGroup>(wsSptr);
   
    const  std::vector<std::string>wsNamevec=sptrWSGrp->getNames();
     period=1;
     it=wsNamevec.begin();
    for (;it!=wsNamevec.end();it++)
    {	std::stringstream count;
      count <<period;
      std::string wsName="multiperiod_"+count.str();
      TS_ASSERT_EQUALS(*it,wsName)
      period++;
    }
    itr1=wsNamevec.begin();
    int periodNumber = 0;
    const int nHistograms = 2;
    for (;itr1!=wsNamevec.end();itr1++)
    {	
      MatrixWorkspace_sptr  outsptr;
      TS_ASSERT_THROWS_NOTHING(outsptr=AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>((*itr1)));
      doTestMultiPeriodWorkspace(outsptr, nHistograms, ++periodNumber);
    }
    std::vector<std::string>::const_iterator itr=wsNamevec.begin();
    MatrixWorkspace_sptr  outsptr1;
    TS_ASSERT_THROWS_NOTHING(outsptr1=AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>((*itr)));
    MatrixWorkspace_sptr  outsptr2;
    TS_ASSERT_THROWS_NOTHING(outsptr2=AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>((*++itr)));
		
    TS_ASSERT_EQUALS( outsptr1->dataX(0), outsptr2->dataX(0) )
    TS_ASSERT_EQUALS( outsptr1->dataY(1)[555], outsptr2->dataY(1)[555] )

	// But the data should be different
    TS_ASSERT_DIFFERS( &(outsptr1->run()), &(outsptr2->run() ))

	it=monitorwsNamevec.begin();
    for (;it!=monitorwsNamevec.end();++it)
	{
		AnalysisDataService::Instance().remove(*it);
	}
	it=wsNamevec.begin();
	for (;it!=wsNamevec.end();++it)
	{
		AnalysisDataService::Instance().remove(*it);
	}
	
	
  }
 
  
  //no monitors in the selected range 
  void testSeparateMonitorswithMixedLimits()
  {
	LoadRaw3 loader9;
    if ( !loader9.isInitialized() ) loader9.initialize();

    loader9.setPropertyValue("Filename", inputFile);
    loader9.setPropertyValue("OutputWorkspace", "outWS");
    loader9.setPropertyValue("SpectrumList", "998,999,1000");
    loader9.setPropertyValue("SpectrumMin", "5");
    loader9.setPropertyValue("SpectrumMax", "10");
	loader9.setPropertyValue("LoadMonitors", "Separate");

    TS_ASSERT_THROWS_NOTHING(loader9.execute());
    TS_ASSERT( loader9.isExecuted() );

    // Get back the saved workspace
    Workspace_sptr output;
    TS_ASSERT_THROWS_NOTHING(output = AnalysisDataService::Instance().retrieve("outWS"));
    Workspace2D_sptr output2D = boost::dynamic_pointer_cast<Workspace2D>(output);

    // Should be 6 for selected input
    TS_ASSERT_EQUALS( output2D->getNumberHistograms(), 9);

    // Check two X vectors are the same
    TS_ASSERT( (output2D->dataX(1)) == (output2D->dataX(5)) );

    // Check two Y arrays have the same number of elements
    TS_ASSERT_EQUALS( output2D->dataY(2).size(), output2D->dataY(7).size() );

    // Check one particular value
    TS_ASSERT_EQUALS( output2D->dataY(8)[777], 9);
    // Check that the error on that value is correct
    TS_ASSERT_EQUALS( output2D->dataE(8)[777], 3);
    // Check that the error on that value is correct
    TS_ASSERT_EQUALS( output2D->dataX(8)[777], 554.1875);
	AnalysisDataService::Instance().remove("outWS");
  }

  // start and end spectra contains  monitors only  
  void testSeparateMonitorswithMaxMinLimits1()
  {	 
    LoadRaw3 loader9;
    if ( !loader9.isInitialized() ) loader9.initialize();

    loader9.setPropertyValue("Filename", inputFile);
    loader9.setPropertyValue("OutputWorkspace", "outWS");
    //loader9.setPropertyValue("SpectrumList", "998,999,1000");
    loader9.setPropertyValue("SpectrumMin", "2");
    loader9.setPropertyValue("SpectrumMax", "4");
    loader9.setPropertyValue("LoadMonitors", "Separate");

    TS_ASSERT_THROWS_NOTHING(loader9.execute());
    TS_ASSERT( loader9.isExecuted() );

    // Get back the saved workspace
    Workspace_sptr output;
    TS_ASSERT_THROWS_NOTHING(output = AnalysisDataService::Instance().retrieve("outWS"));
    Workspace2D_sptr output2D = boost::dynamic_pointer_cast<Workspace2D>(output);
    TS_ASSERT(output2D);
    if (!output2D) return;

    // Should be 3 for selected input
    TS_ASSERT_EQUALS( output2D->getNumberHistograms(), 3);

    // Check two X vectors are the same
    //TS_ASSERT( (output2D->dataX(1)) == (output2D->dataX(5)) );

    // Check two Y arrays have the same number of elements
    TS_ASSERT_EQUALS( output2D->dataY(1).size(), output2D->dataY(2).size() );

    // Check one particular value
    TS_ASSERT_EQUALS( output2D->dataY(1)[1], 192);
    AnalysisDataService::Instance().remove("outWS");
    }

  //select start and end spectra a mix of monitors and normal workspace
  void testSeparateMonitorswithMaxMinimits2()
  {	
	 LoadRaw3 loader10;
    if ( !loader10.isInitialized() ) loader10.initialize();

    loader10.setPropertyValue("Filename", inputFile);
    loader10.setPropertyValue("OutputWorkspace", "outWS");
    loader10.setPropertyValue("SpectrumMin", "2");
    loader10.setPropertyValue("SpectrumMax", "100");
	loader10.setPropertyValue("LoadMonitors", "Separate");

    TS_ASSERT_THROWS_NOTHING(loader10.execute());
    TS_ASSERT( loader10.isExecuted() );

    // Get back the saved workspace
    Workspace_sptr output;
    TS_ASSERT_THROWS_NOTHING(output = AnalysisDataService::Instance().retrieve("outWS"));
    Workspace2D_sptr output2D = boost::dynamic_pointer_cast<Workspace2D>(output);

	Workspace_sptr monitoroutput;
	TS_ASSERT_THROWS_NOTHING(monitoroutput = AnalysisDataService::Instance().retrieve("outWS_Monitors"));
    Workspace2D_sptr monitoroutput2D = boost::dynamic_pointer_cast<Workspace2D>(monitoroutput);

   // Should be 6 for selected input
    TS_ASSERT_EQUALS( output2D->getNumberHistograms(),96);

	 TS_ASSERT_EQUALS( monitoroutput2D->getNumberHistograms(),3);

    // Check two X vectors are the same
    TS_ASSERT( (monitoroutput2D->dataX(1)) == (output2D->dataX(1)) );

    // Check two Y arrays have the same number of elements
    TS_ASSERT_EQUALS( output2D->dataY(2).size(), output2D->dataY(3).size() );
	AnalysisDataService::Instance().remove("outWS_Monitors");
	AnalysisDataService::Instance().remove("outWS");

    // Check one particular value

    }
   //no monitors in the selected range 
  void testSeparateMonitorswithMixedLimits3()
  {
	LoadRaw3 loader11;
    if ( !loader11.isInitialized() ) loader11.initialize();

    loader11.setPropertyValue("Filename", inputFile);
    loader11.setPropertyValue("OutputWorkspace", "outWS");
    loader11.setPropertyValue("SpectrumList", "2,3,1000,1001,1002");
    loader11.setPropertyValue("SpectrumMin", "2");
    loader11.setPropertyValue("SpectrumMax", "100");
	loader11.setPropertyValue("LoadMonitors", "Separate");

    TS_ASSERT_THROWS_NOTHING(loader11.execute());
    TS_ASSERT( loader11.isExecuted() );

    // Get back the saved workspace
    Workspace_sptr output;
    TS_ASSERT_THROWS_NOTHING(output = AnalysisDataService::Instance().retrieve("outWS"));
    Workspace2D_sptr output2D = boost::dynamic_pointer_cast<Workspace2D>(output);

	Workspace_sptr monitoroutput;
	TS_ASSERT_THROWS_NOTHING(monitoroutput = AnalysisDataService::Instance().retrieve("outWS_Monitors"));
    Workspace2D_sptr monitoroutput2D = boost::dynamic_pointer_cast<Workspace2D>(monitoroutput);


    // Should be 6 for selected input
    TS_ASSERT_EQUALS( output2D->getNumberHistograms(), 99);

	TS_ASSERT_EQUALS( monitoroutput2D->getNumberHistograms(),3 );

	AnalysisDataService::Instance().remove("outWS_Monitors");
	AnalysisDataService::Instance().remove("outWS");
  }
   //no monitors in the selected range 
  void testExcludeMonitors()
  {
	 LoadRaw3 loader11;
    if ( !loader11.isInitialized() ) loader11.initialize();

    loader11.setPropertyValue("Filename", inputFile);
    loader11.setPropertyValue("OutputWorkspace", "outWS");
    loader11.setPropertyValue("LoadMonitors", "Exclude");

    TS_ASSERT_THROWS_NOTHING(loader11.execute());
    TS_ASSERT( loader11.isExecuted() );

    // Get back the saved workspace
    Workspace_sptr output;
    TS_ASSERT_THROWS_NOTHING(output = AnalysisDataService::Instance().retrieve("outWS"));
    Workspace2D_sptr output2D = boost::dynamic_pointer_cast<Workspace2D>(output);
    // Should be 6 for selected input
    TS_ASSERT_EQUALS( output2D->getNumberHistograms(), 2580);
    // Check one particular value
    TS_ASSERT_EQUALS( output2D->dataY(995)[777], 9);
    // Check that the error on that value is correct
    TS_ASSERT_EQUALS( output2D->dataE(995)[777], 3);
    // Check that the error on that value is correct
    TS_ASSERT_EQUALS( output2D->dataX(995)[777], 554.1875);
	AnalysisDataService::Instance().remove("outWS");
  }

  void testExcludeMonitorswithMaxMinLimits()
  {
	 LoadRaw3 loader11;
    if ( !loader11.isInitialized() ) loader11.initialize();

    loader11.setPropertyValue("Filename", inputFile);
    loader11.setPropertyValue("OutputWorkspace", "outWS");
	loader11.setPropertyValue("SpectrumList", "2,3,1000,1001,1002");
    loader11.setPropertyValue("SpectrumMin", "2");
    loader11.setPropertyValue("SpectrumMax", "100");
    loader11.setPropertyValue("LoadMonitors", "Exclude");

    TS_ASSERT_THROWS_NOTHING(loader11.execute());
    TS_ASSERT( loader11.isExecuted() );

    // Get back the saved workspace
    Workspace_sptr output;
    TS_ASSERT_THROWS_NOTHING(output = AnalysisDataService::Instance().retrieve("outWS"));
    Workspace2D_sptr output2D = boost::dynamic_pointer_cast<Workspace2D>(output);
    // Should be 6 for selected input
    TS_ASSERT_EQUALS( output2D->getNumberHistograms(), 99);
    AnalysisDataService::Instance().remove("outWS");
    
  }

  void testWithManagedWorkspace()
  {
    ConfigServiceImpl& conf = ConfigService::Instance();
    const std::string managed = "ManagedWorkspace.LowerMemoryLimit";
    const std::string oldValue = conf.getString(managed);
    conf.setString(managed,"0");

    LoadRaw3 loader4;
    loader4.initialize();
    loader4.setPropertyValue("Filename", inputFile);
    loader4.setPropertyValue("OutputWorkspace", "managedws2");
    TS_ASSERT_THROWS_NOTHING( loader4.execute() )
    TS_ASSERT( loader4.isExecuted() )

    // Get back workspace and check it really is a ManagedWorkspace2D
    Workspace_sptr output;
    TS_ASSERT_THROWS_NOTHING( output = AnalysisDataService::Instance().retrieve("managedws2") );
    TS_ASSERT( dynamic_cast<ManagedWorkspace2D*>(output.get()) )

    AnalysisDataService::Instance().remove("managedws2");
    conf.setString(managed,oldValue);
  }

  void testSeparateMonitorsWithManagedWorkspace()
  {
    ConfigServiceImpl& conf = ConfigService::Instance();
    const std::string managed = "ManagedWorkspace.LowerMemoryLimit";
    const std::string oldValue = conf.getString(managed);
    conf.setString(managed,"0");

	  LoadRaw3 loader8;
	  loader8.initialize();
	  loader8.setPropertyValue("Filename", inputFile);
	  loader8.setPropertyValue("OutputWorkspace", "managedws2");
	  loader8.setPropertyValue("LoadMonitors", "Separate");
	  TS_ASSERT_THROWS_NOTHING( loader8.execute() )
	  TS_ASSERT( loader8.isExecuted() )

	  // Get back workspace and check it really is a ManagedWorkspace2D
	  Workspace_sptr output;
	  TS_ASSERT_THROWS_NOTHING( output = AnalysisDataService::Instance().retrieve("managedws2") );
	  TS_ASSERT( dynamic_cast<ManagedWorkspace2D*>(output.get()) )
		  Workspace_sptr output1;
	  TS_ASSERT_THROWS_NOTHING( output1 = AnalysisDataService::Instance().retrieve("managedws2_Monitors") );
	 // TS_ASSERT( dynamic_cast<ManagedWorkspace2D*>(output1.get()) )
	  AnalysisDataService::Instance().remove("managedws2");
	  AnalysisDataService::Instance().remove("managedws2_Monitors");
    conf.setString(managed,oldValue);
  } 

private:

  /// Helper method to run common set of tests on a workspace in a multi-period group.
  void doTestMultiPeriodWorkspace(MatrixWorkspace_sptr workspace, const size_t& nHistograms, int expected_period)
  {
    // Check the number of histograms.
    TS_ASSERT_EQUALS(workspace->getNumberHistograms(), nHistograms);
    // Check the current period property.
    const Mantid::API::Run& run = workspace->run();
    Property* prop = run.getLogData("current_period");
    PropertyWithValue<int>* current_period_property = dynamic_cast<PropertyWithValue<int>* >(prop); 
    TS_ASSERT(current_period_property != NULL);
    int actual_period;
    Kernel::toValue<int>(current_period_property->value(), actual_period);
    TS_ASSERT_EQUALS(expected_period, actual_period);
    // Check the period n property.
    std::stringstream stream;
    stream << "period " << actual_period;
    TSM_ASSERT_THROWS_NOTHING("period number series could not be found.", run.getLogData(stream.str()));
  }

  LoadRaw3 loader,loader2,loader3;
  std::string inputFile;
  std::string outputSpace;
};

//------------------------------------------------------------------------------
// Performance test
//------------------------------------------------------------------------------

class LoadRaw3TestPerformance : public CxxTest::TestSuite
{
public:
  void testDefaultLoad()
  {
    LoadRaw3 loader;
    loader.initialize();
    loader.setPropertyValue("Filename", "HET15869.raw");
    loader.setPropertyValue("OutputWorkspace", "ws");
    TS_ASSERT( loader.execute() );
  }
};



#endif /*LoadRaw3TEST_H_*/

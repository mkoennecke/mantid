#ifndef LOADTEST_H_
#define LOADTEST_H_

#include <cxxtest/TestSuite.h>
#include "MantidDataHandling/Load.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/WorkspaceGroup.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/AlgorithmManager.h"

using namespace Mantid::API;
using namespace Mantid::DataObjects;
using namespace Mantid::DataHandling;

class LoadTest : public CxxTest::TestSuite
{
public:
  void tearDown()
  {
    AnalysisDataService::Instance().clear();
  }

  void testViaProxy()
  {
    IAlgorithm_sptr proxy = AlgorithmManager::Instance().create("Load");
    TS_ASSERT_EQUALS(proxy->existsProperty("Filename"), true);
    TS_ASSERT_EQUALS(proxy->existsProperty("OutputWorkspace"), true);
    
    TS_ASSERT_THROWS_NOTHING(proxy->setPropertyValue("Filename","IRS38633.raw"));
    TS_ASSERT_EQUALS(proxy->existsProperty("Cache"), true);
    TS_ASSERT_EQUALS(proxy->existsProperty("LoadLogFiles"), true);

    proxy->setPropertyValue("Filename","IRS38633.raw");
    TS_ASSERT_EQUALS(proxy->existsProperty("Cache"), true);
    TS_ASSERT_EQUALS(proxy->existsProperty("LoadLogFiles"), true);

    TS_ASSERT_THROWS_NOTHING(proxy->setPropertyValue("Filename","LOQ49886.nxs"));   
    TS_ASSERT_EQUALS(proxy->existsProperty("Cache"), false);
    TS_ASSERT_EQUALS(proxy->existsProperty("LoadLogFiles"), false);
  }

  void testPropertyValuesViaProxy()
  {
    IAlgorithm_sptr proxy = AlgorithmManager::Instance().create("Load");
    TS_ASSERT_EQUALS(proxy->existsProperty("Filename"), true);
    TS_ASSERT_EQUALS(proxy->existsProperty("OutputWorkspace"), true);
    
    TS_ASSERT_THROWS_NOTHING(proxy->setPropertyValue("Filename","IRS38633.raw"));
    TS_ASSERT_EQUALS(proxy->existsProperty("Cache"), true);
    TS_ASSERT_EQUALS(proxy->existsProperty("LoadLogFiles"), true);
    TS_ASSERT_THROWS_NOTHING(proxy->setPropertyValue("SpectrumMin","10"));
    TS_ASSERT_THROWS_NOTHING(proxy->setPropertyValue("SpectrumMax","100"));

    // Test that the properties have the correct values
    TS_ASSERT_EQUALS(proxy->getPropertyValue("SpectrumMin"),"10");
    TS_ASSERT_EQUALS(proxy->getPropertyValue("SpectrumMax"),"100");
  }

  void testSwitchingLoaderViaProxy()
  {    
    IAlgorithm_sptr proxy = AlgorithmManager::Instance().create("Load");
    TS_ASSERT_EQUALS(proxy->existsProperty("Filename"), true);
    TS_ASSERT_EQUALS(proxy->existsProperty("OutputWorkspace"), true);
    TS_ASSERT_THROWS_NOTHING(proxy->setPropertyValue("Filename","IRS38633.raw"));
    TS_ASSERT_EQUALS(proxy->existsProperty("Cache"), true);
    TS_ASSERT_EQUALS(proxy->existsProperty("LoadLogFiles"), true);

    TS_ASSERT_THROWS_NOTHING(proxy->setPropertyValue("SpectrumMin","10"));
    TS_ASSERT_THROWS_NOTHING(proxy->setPropertyValue("SpectrumMax","100"));

    // Test that the properties have the correct values
    TS_ASSERT_EQUALS(proxy->getPropertyValue("SpectrumMin"),"10");
    TS_ASSERT_EQUALS(proxy->getPropertyValue("SpectrumMax"),"100");

    // Change loader
    proxy->setPropertyValue("Filename","LOQ49886.nxs");
    TS_ASSERT_EQUALS(proxy->existsProperty("EntryNumber"), true);
    TS_ASSERT_EQUALS(proxy->existsProperty("Cache"), false);

    TS_ASSERT_THROWS_NOTHING(proxy->setPropertyValue("SpectrumMin","11"));
    TS_ASSERT_THROWS_NOTHING(proxy->setPropertyValue("SpectrumMax","101"));

    TS_ASSERT_EQUALS(proxy->getPropertyValue("SpectrumMin"),"11");
    TS_ASSERT_EQUALS(proxy->getPropertyValue("SpectrumMax"),"101");

  }

  void testFindLoader()
  {
    Load loader;
    loader.initialize();
    const char * loadraw_props[5] = {"SpectrumMin", "SpectrumMax", "SpectrumList", "Cache", "LoadLogFiles"};
    const size_t numProps = (size_t)(sizeof(loadraw_props)/sizeof(const char*));
    // Basic load has no additional loader properties
    for( size_t i = 0; i < numProps ; ++i )
    {
      TS_ASSERT_EQUALS(loader.existsProperty(loadraw_props[i]), false);
    }
    //After setting the file property, the algorithm should have aquired the appropriate properties
    TS_ASSERT_THROWS_NOTHING(loader.setPropertyValue("Filename","IRS38633.raw"));
    // Now
    for( size_t i = 0; i < numProps; ++i )
    {
      TS_ASSERT_EQUALS(loader.existsProperty(loadraw_props[i]), true);
    }

    // Did it find the right loader
    TS_ASSERT_EQUALS(loader.getPropertyValue("LoaderName"), "LoadRaw");
  }

  void testRaw()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","IRS38633.raw");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    loader.setRethrows(true);
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    MatrixWorkspace_sptr ws = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("LoadTest_Output");
    TS_ASSERT(ws);
  }

  void testRawWithOneSpectrum()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","IRS38633.raw");
    const std::string outputName("LoadTest_IRS38633raw");
    loader.setPropertyValue("OutputWorkspace", outputName);
    loader.setPropertyValue("SpectrumList", "1");
    loader.setRethrows(true);
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    TS_ASSERT_EQUALS(loader.isExecuted(), true);

    AnalysisDataServiceImpl& dataStore = AnalysisDataService::Instance();
    TS_ASSERT_EQUALS(dataStore.doesExist(outputName), true);
    
    MatrixWorkspace_sptr ws = boost::dynamic_pointer_cast<MatrixWorkspace>(dataStore.retrieve(outputName));
    if(!ws) TS_FAIL("Cannot retrieve workspace from the store");

    // Check it only has 1 spectrum
    TS_ASSERT_EQUALS( ws->getNumberHistograms(), 1 );
  }

  void testRaw1()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","CSP74683.s02");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    MatrixWorkspace_sptr ws = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("LoadTest_Output");
    TS_ASSERT(ws);
  }

  void testRawGroup()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","CSP79590.raw");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    WorkspaceGroup_sptr wsg = AnalysisDataService::Instance().retrieveWS<WorkspaceGroup>("LoadTest_Output");
    TS_ASSERT(wsg);
    MatrixWorkspace_sptr ws = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("LoadTest_Output_1");
    TS_ASSERT(ws);

  }
  
  void testHDF4Nexus()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","emu00006473.nxs");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    MatrixWorkspace_sptr ws = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("LoadTest_Output");
    TS_ASSERT(ws);
  }

  void _ARGUS_NXS()
  {
    Load loader;
    loader.initialize();
    TS_ASSERT_THROWS_NOTHING(loader.setPropertyValue("Filename","argus0026287.nxs"));

    TS_ASSERT_EQUALS(loader.getPropertyValue("LoaderName"), "LoadMuonNexus");
  }

  void testHDF4NexusGroup()
  {
    // Note that there are no 64-bit HDF4 libraries for Windows.
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","MUSR00015189.nxs");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    WorkspaceGroup_sptr wsg = AnalysisDataService::Instance().retrieveWS<WorkspaceGroup>("LoadTest_Output");
    TS_ASSERT(wsg);
    MatrixWorkspace_sptr ws = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("LoadTest_Output_1");
    TS_ASSERT(ws);
  }
   void testISISNexus()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","LOQ49886.nxs");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    MatrixWorkspace_sptr ws = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("LoadTest_Output");
    TS_ASSERT(ws);
  }

  void testUnknownExt()
  {
    Load loader;
    loader.initialize();
    TS_ASSERT_THROWS_NOTHING(loader.setPropertyValue("Filename","hrpd_new_072_01.cal"));
  }

  void testSPE()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","Example.spe");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    MatrixWorkspace_sptr ws = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("LoadTest_Output");
    TS_ASSERT(ws);
  }
  
  void testAscii()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","AsciiExample.txt");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    MatrixWorkspace_sptr ws = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("LoadTest_Output");
    TS_ASSERT(ws);
  }

  void testSpice2D()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","BioSANS_exp61_scan0004_0001.xml");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    MatrixWorkspace_sptr ws = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("LoadTest_Output");
    TS_ASSERT(ws);
  }
  void testSNSSpec()
  {
     Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","LoadSNSspec.txt");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    MatrixWorkspace_sptr ws = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("LoadTest_Output");
    TS_ASSERT(ws);
  }

  void testGSS()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","gss.txt");
    // Check correct loader identified
    TS_ASSERT_EQUALS(loader.getPropertyValue("LoaderName"), "LoadGSS");

    Load loader2;
    loader2.initialize();
    loader2.setPropertyValue("Filename","gss-ExtendedHeader.gsa");
    // Check correct loader identified
    TS_ASSERT_EQUALS(loader2.getPropertyValue("LoaderName"), "LoadGSS");
  }

   void testRKH()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","DIRECT.041");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    MatrixWorkspace_sptr ws = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("LoadTest_Output");
    TS_ASSERT(ws);
  }

  void test_EventPreNeXus_WithNoExecute()
  {
    Load loader;
    loader.initialize();
    TS_ASSERT_THROWS_NOTHING(loader.setPropertyValue("Filename", "CNCS_7860_neutron_event.dat"));
    TS_ASSERT_EQUALS(loader.existsProperty("EventFilename"), false);
    TS_ASSERT_EQUALS(loader.getPropertyValue("LoaderName"), "LoadEventPreNexus");
  }

  void test_SNSEventNeXus_WithNoExecute()
  {
    Load loader;
    loader.initialize();
    TS_ASSERT_THROWS_NOTHING(loader.setPropertyValue("Filename", "CNCS_7860_event.nxs"));
    TS_ASSERT_EQUALS(loader.existsProperty("EventFilename"), false);
    TS_ASSERT_EQUALS(loader.getPropertyValue("LoaderName"), "LoadEventNexus");
  }

  void testDaveGrp()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename", "DaveAscii.grp");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    MatrixWorkspace_sptr ws = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("LoadTest_Output");
    TS_ASSERT(ws);
  }

  void testArgusFileLoadingWithIncorrectZeroPadding()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename", "argus0026287.nxs");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    MatrixWorkspace_sptr ws = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("LoadTest_Output");
    TS_ASSERT(ws);
  }

  void testMDWorkspace()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename", "SEQ_MDEW.nxs");
    const std::string outputWS("MDWS");
    loader.setPropertyValue("OutputWorkspace",outputWS);
    TS_ASSERT( loader.execute() );
    TS_ASSERT_THROWS_NOTHING( AnalysisDataService::Instance().retrieveWS<IMDWorkspace>(outputWS) );
  }

  void testList()
  {
    AnalysisDataService::Instance().clear();
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename", "MUSR15189,15190,15191.nxs");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    WorkspaceGroup_sptr output = AnalysisDataService::Instance().retrieveWS<WorkspaceGroup>("LoadTest_Output");
    TS_ASSERT(output);
    TS_ASSERT_EQUALS(output->getNumberOfEntries(),6);
    MatrixWorkspace_sptr ws1 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015189_1");
    TS_ASSERT(ws1);
    MatrixWorkspace_sptr ws2 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015189_2");
    TS_ASSERT(ws2);
    MatrixWorkspace_sptr ws3 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015190_1");
    TS_ASSERT(ws3);
    MatrixWorkspace_sptr ws4 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015190_2");
    TS_ASSERT(ws4);
    MatrixWorkspace_sptr ws5 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015191_1");
    TS_ASSERT(ws5);
    MatrixWorkspace_sptr ws6 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015191_2");
    TS_ASSERT(ws6);

    // Check that originally loaded groups are not left in the ADS
    TS_ASSERT(!AnalysisDataService::Instance().doesExist("MUSR00015189"));
    TS_ASSERT(!AnalysisDataService::Instance().doesExist("MUSR00015190"));
    TS_ASSERT(!AnalysisDataService::Instance().doesExist("MUSR00015191"));
  }

  void testPlus()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename", "IRS38633+38633.nxs");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());

    MatrixWorkspace_sptr output = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("LoadTest_Output");
    TS_ASSERT(output);
  }

  void testPlusGroupWorkspaces()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename", "MUSR15189+15190.nxs");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());

    WorkspaceGroup_sptr output = AnalysisDataService::Instance().retrieveWS<WorkspaceGroup>("LoadTest_Output");
    TS_ASSERT(output);
    MatrixWorkspace_sptr ws1 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015189_MUSR00015190_1");
    TS_ASSERT(ws1);
    MatrixWorkspace_sptr ws2 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015189_MUSR00015190_2");
  }

  void testRange()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename", "MUSR15189:15192.nxs");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    WorkspaceGroup_sptr output = AnalysisDataService::Instance().retrieveWS<WorkspaceGroup>("LoadTest_Output");
    TS_ASSERT(output);
    TS_ASSERT_EQUALS(output->getNumberOfEntries(),8);
    MatrixWorkspace_sptr ws1 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015189_1");
    TS_ASSERT(ws1);
    MatrixWorkspace_sptr ws2 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015189_2");
    TS_ASSERT(ws2);
    MatrixWorkspace_sptr ws3 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015190_1");
    TS_ASSERT(ws3);
    MatrixWorkspace_sptr ws4 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015190_2");
    TS_ASSERT(ws4);
    MatrixWorkspace_sptr ws5 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015191_1");
    TS_ASSERT(ws5);
    MatrixWorkspace_sptr ws6 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015191_2");
    TS_ASSERT(ws6);
    MatrixWorkspace_sptr ws7 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015192_1");
    TS_ASSERT(ws7);
    MatrixWorkspace_sptr ws8 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015192_2");
    TS_ASSERT(ws8);
  }

  void testSteppedRange()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename", "MUSR15189:15192:2.nxs");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    WorkspaceGroup_sptr output = AnalysisDataService::Instance().retrieveWS<WorkspaceGroup>("LoadTest_Output");
    TS_ASSERT(output);
    TS_ASSERT_EQUALS(output->getNumberOfEntries(),4);
    MatrixWorkspace_sptr ws1 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015189_1");
    TS_ASSERT(ws1);
    MatrixWorkspace_sptr ws2 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015189_2");
    TS_ASSERT(ws2);
    MatrixWorkspace_sptr ws3 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015191_1");
    TS_ASSERT(ws3);
    MatrixWorkspace_sptr ws4 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015191_2");
    TS_ASSERT(ws4);
  }

  void testAddedRange()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename", "MUSR15189-15192.nxs");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());

    WorkspaceGroup_sptr output = AnalysisDataService::Instance().retrieveWS<WorkspaceGroup>("LoadTest_Output");
    TS_ASSERT(output);
    TS_ASSERT_EQUALS(output->getNumberOfEntries(),2);
    MatrixWorkspace_sptr ws1 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015189_MUSR00015190_MUSR00015191_MUSR00015192_1");
    TS_ASSERT(ws1);
    TS_ASSERT_DELTA(ws1->readY(0)[0], 16.0, 1e-12);
    TS_ASSERT_DELTA(ws1->readY(6)[4], 1.0, 1e-12);

    MatrixWorkspace_sptr ws2 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015189_MUSR00015190_MUSR00015191_MUSR00015192_2");
    TS_ASSERT(ws2);
    TS_ASSERT_DELTA(ws2->readY(0)[5], 2.0, 1e-12);
    TS_ASSERT_DELTA(ws2->readY(8)[0], 6.0, 1e-12);
  }

  void testAddedSteppedRange()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename", "MUSR15189-15192:2.nxs");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());

    WorkspaceGroup_sptr output = AnalysisDataService::Instance().retrieveWS<WorkspaceGroup>("LoadTest_Output");
    TS_ASSERT(output);
    TS_ASSERT_EQUALS(output->getNumberOfEntries(),2);
    MatrixWorkspace_sptr ws1 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015189_MUSR00015191_1");
    TS_ASSERT(ws1);
    MatrixWorkspace_sptr ws2 = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("MUSR00015189_MUSR00015191_2");
    TS_ASSERT(ws2);
  }

  void testMultiFilesExtraProperties()
  {    
    IAlgorithm_sptr proxy = AlgorithmManager::Instance().create("Load");

    TS_ASSERT_THROWS_NOTHING(proxy->setPropertyValue("Filename","IRS21360,26173,38633.raw"));
    TS_ASSERT_THROWS_NOTHING(proxy->setPropertyValue("OutputWorkspace","test"));

    TS_ASSERT_THROWS_NOTHING(proxy->setPropertyValue("SpectrumMin","10"));
    TS_ASSERT_THROWS_NOTHING(proxy->setPropertyValue("SpectrumMax","100"));

    TS_ASSERT_THROWS_NOTHING(proxy->execute());

    // get result
    WorkspaceGroup_sptr wsg = AnalysisDataService::Instance().retrieveWS<WorkspaceGroup>("test");
    TS_ASSERT(wsg);

    // get first ws in group
    std::vector<std::string> childNames = wsg->getNames();
    MatrixWorkspace_sptr childWs = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>(childNames[0]);
    TS_ASSERT(childWs);

    // Make sure that it contains the requested number of spectra as per SpectrumMin and SpectrumMax
    TS_ASSERT_EQUALS(childWs->getNumberHistograms(), 91);
  }

  void testCommaSeparatedListOfDifferentIntruments()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename", "LOQ48127.raw, CSP79590.raw");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());

    WorkspaceGroup_sptr wsg = AnalysisDataService::Instance().retrieveWS<WorkspaceGroup>("LoadTest_Output");
    TS_ASSERT(wsg);
    TS_ASSERT_EQUALS(wsg->getNames().size(), 3);
  }

  void test_outputWsNameSameAsOneOfTheSinglePeriodFileNames()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename", "LOQ48127.raw, CSP79590.raw");
    loader.setPropertyValue("OutputWorkspace","LOQ48127");
    TS_ASSERT_THROWS_NOTHING(loader.execute());

    WorkspaceGroup_sptr wsg = AnalysisDataService::Instance().retrieveWS<WorkspaceGroup>("LOQ48127");
    TS_ASSERT(wsg);
    TS_ASSERT_EQUALS(wsg->getNames().size(), 3);
  }

  void test_cleanupAfterMultifileLoading()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename", "MUSR15189-15192:2.nxs");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());

    TS_ASSERT_EQUALS(3, AnalysisDataService::Instance().size());

    const auto wsg = AnalysisDataService::Instance().retrieveWS<WorkspaceGroup>("LoadTest_Output");
    TS_ASSERT(wsg);
    TS_ASSERT_EQUALS(wsg->getNames().size(), 2);
  }
};


//-------------------------------------------------------------------------------------------------
// Performance test
//
// This simple checks how long it takes to run the search for a Loader, which is done when
// the file property is set
//-------------------------------------------------------------------------------------------------

class LoadTestPerformance : public CxxTest::TestSuite
{
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static LoadTestPerformance *createSuite() { return new LoadTestPerformance(); }
  static void destroySuite( LoadTestPerformance *suite ) { delete suite; }

  void test_find_loader_performance()
  {
    const size_t ntimes(5);

    for(size_t i = 0; i < ntimes; ++i)
    {
      Mantid::DataHandling::Load loader;
      loader.initialize();
      loader.setPropertyValue("Filename", "CNCS_7860_event.nxs");
    }
  }
};


#endif /*LOADTEST_H_*/

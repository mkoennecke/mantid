#ifndef FINDPEAKSTEST_H_
#define FINDPEAKSTEST_H_

#include <cxxtest/TestSuite.h>
#include "MantidTestHelpers/WorkspaceCreationHelper.h"

#include "MantidAlgorithms/FindPeaks.h"

#include "MantidAPI/FrameworkManager.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidDataHandling/LoadNexusProcessed.h"
#include "MantidDataHandling/LoadInstrument.h"
#include "MantidDataObjects/Workspace2D.h"

#include <fstream>

using Mantid::Algorithms::FindPeaks;

using namespace Mantid::API;

class FindPeaksTest : public CxxTest::TestSuite
{
public:
  static FindPeaksTest *createSuite() { return new FindPeaksTest(); }
  static void destroySuite( FindPeaksTest *suite ) { delete suite; }

  FindPeaksTest()
  {
    FrameworkManager::Instance();
  }

  void testTheBasics()
  {
    FindPeaks finder;
    TS_ASSERT_EQUALS( finder.name(), "FindPeaks" );
    TS_ASSERT_EQUALS( finder.version(), 1 );
  }

  void testInit()
  {
    FindPeaks finder;
    TS_ASSERT_THROWS_NOTHING( finder.initialize() );
    TS_ASSERT( finder.isInitialized() );
  }

  void testExec()
  {
    // Load data file
    Mantid::DataHandling::LoadNexusProcessed loader;
    loader.initialize();
    loader.setProperty("Filename","focussed.nxs");
    loader.setProperty("OutputWorkspace", "FindPeaksTest_peaksWS");
    loader.execute();

    // Find peaks (Test)
    FindPeaks finder;
    if ( !finder.isInitialized() ) finder.initialize();

    TS_ASSERT_THROWS_NOTHING( finder.setPropertyValue("InputWorkspace","FindPeaksTest_peaksWS") );
    TS_ASSERT_THROWS_NOTHING( finder.setPropertyValue("WorkspaceIndex","4") );
    // TS_ASSERT_THROWS_NOTHING( finder.setPropertyValue("SmoothedData","smoothed") );
    TS_ASSERT_THROWS_NOTHING( finder.setPropertyValue("PeaksList","FindPeaksTest_foundpeaks") );

    TS_ASSERT_THROWS_NOTHING( finder.execute() );
    TS_ASSERT( finder.isExecuted() );

    Mantid::API::ITableWorkspace_sptr peaklist = boost::dynamic_pointer_cast<Mantid::API::ITableWorkspace>
                  (Mantid::API::AnalysisDataService::Instance().retrieve("FindPeaksTest_foundpeaks"));

    TS_ASSERT( peaklist );
    TS_ASSERT_EQUALS( peaklist->rowCount() , 9 );
    TS_ASSERT_DELTA( peaklist->Double(1,1), 0.59, 0.01 );
    TS_ASSERT_DELTA( peaklist->Double(2,1), 0.71, 0.01 );
    TS_ASSERT_DELTA( peaklist->Double(3,1), 0.81, 0.01 );
    // This is a dodgy value, that comes out different on different platforms
    //TS_ASSERT_DELTA( peaklist->Double(3,1), 1.03, 0.01 );
    TS_ASSERT_DELTA( peaklist->Double(5,1), 0.96, 0.01 );
    TS_ASSERT_DELTA( peaklist->Double(6,1), 1.24, 0.01 );
    TS_ASSERT_DELTA( peaklist->Double(7,1), 1.52, 0.01 );
    TS_ASSERT_DELTA( peaklist->Double(8,1), 2.14, 0.01 );

  }

  void LoadPG3_733()
  {
    Mantid::DataHandling::LoadNexusProcessed loader;
    loader.initialize();
    loader.setProperty("Filename","PG3_733_focussed.nxs");
    loader.setProperty("OutputWorkspace", "FindPeaksTest_vanadium");
    loader.execute();
  }

  void PtestExecGivenPeaksList()
  {
    this->LoadPG3_733();

    FindPeaks finder;
    if ( !finder.isInitialized() ) finder.initialize();
    TS_ASSERT_THROWS_NOTHING( finder.setPropertyValue("InputWorkspace","FindPeaksTest_vanadium") );
    TS_ASSERT_THROWS_NOTHING( finder.setPropertyValue("WorkspaceIndex","0") );
    TS_ASSERT_THROWS_NOTHING( finder.setPropertyValue("PeakPositions", "0.5044,0.5191,0.5350,0.5526,0.5936,0.6178,0.6453,0.6768,0.7134,0.7566,0.8089,0.8737,0.9571,1.0701,1.2356,1.5133,2.1401") );
//    TS_ASSERT_THROWS_NOTHING( finder.setPropertyValue("SmoothedData","ignored_smoothed_data") );
    TS_ASSERT_THROWS_NOTHING( finder.setPropertyValue("PeaksList","FindPeaksTest_foundpeaks2") );

    TS_ASSERT_THROWS_NOTHING( finder.execute() );
    TS_ASSERT( finder.isExecuted() );

  }

private:
};

//=================================================================================================
/** Performance test with large workspaces.
  */

class FindPeaksTestPerformance : public CxxTest::TestSuite
{
  Mantid::API::MatrixWorkspace_sptr m_dataWS;

public:
  static FindPeaksTestPerformance *createSuite() { return new FindPeaksTestPerformance(); }
  static void destroySuite( FindPeaksTestPerformance *suite ) { delete suite; }

  /** Constructor
    */
  FindPeaksTestPerformance()
  {

  }

  /** Set up workspaces
    */
  void setUp()
  {
    // Load data file
    Mantid::DataHandling::LoadNexusProcessed loader;
    loader.initialize();
    loader.setProperty("Filename","focussed.nxs");
    loader.setProperty("OutputWorkspace", "FindPeaksTest_peaksWS");
    loader.execute();

    m_dataWS = boost::dynamic_pointer_cast<Mantid::API::MatrixWorkspace>(
          AnalysisDataService::Instance().retrieve("FindPeaksTest_peaksWS"));

    return;
  }

  /** Find peaks by auto-determine peaks' positions
    */
  void test_FindPeaksAutoPeakPositions()
  {
    // Find peaks (Test)
    FindPeaks finder;
    if ( !finder.isInitialized() ) finder.initialize();

    if (!m_dataWS)
      throw std::runtime_error("Unable to get input matrix workspace. ");
    finder.setPropertyValue("InputWorkspace","FindPeaksTest_peaksWS");
    finder.setPropertyValue("PeakPositions", "0.8089, 0.9571, 1.0701,1.2356,1.5133,2.1401");
    finder.setPropertyValue("PeaksList","FindPeaksTest_foundpeaks");

    finder.execute();
  }

}; // end of class FindPeaksTestPerformance


#endif /*FINDPEAKSTEST_H_*/

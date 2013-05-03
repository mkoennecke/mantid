#ifndef MANTID_ALGORITHMS_DIVIDESPLITTERSTEST_H_
#define MANTID_ALGORITHMS_DIVIDESPLITTERSTEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidAlgorithms/DivideSplitters.h"

using Mantid::Algorithms::DivideSplitters;

using namespace Mantid;
using namespace Mantid::Algorithms;
using namespace Mantid::API;
using namespace Mantid::Kernel;
using namespace Mantid::DataObjects;

using namespace std;

class DivideSplittersTest : public CxxTest::TestSuite
{
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static DivideSplittersTest *createSuite() { return new DivideSplittersTest(); }
  static void destroySuite( DivideSplittersTest *suite ) { delete suite; }


  void test_Init()
  {
    DivideSplitters dividealg;
    dividealg.initialize();
    TS_ASSERT(dividealg.isInitialized())
  }


  /** Test to divide a workspace
    */
  void test_divideSplitters()
  {
    // Create a Splitters workspace
    size_t numwsgroups = 10;
    size_t numsplitters = 100;
    SplittersWorkspace_sptr splitws = createSplittersWS(numwsgroups, numsplitters);
    Workspace_sptr infows = createInfoWS(numwsgroups);

    AnalysisDataService::Instance().addOrReplace("SplittersFull", splitws);
    AnalysisDataService::Instance().addOrReplace("InfomationFull", infows);

    // Call algorithm
    DivideSplitters divider;
    divider.initialize();;

    divider.setProperty("InputWorkspace", "SplittersFull");
    divider.setProperty("InfoWorkspace", "InfomationFull");
    divider.setProperty("NumberOfSegments", 6);
    divider.setProperty("WorkspaceIndex", 5);
    divider.setProperty("OutputWorkspace", "NewSplitters");
    divider.setProperty("OutputInfoWorkspace", "NewInfomationTable");

    divider.execute();
    TS_ASSERT(divider.isExecuted());

    // Check result
    SplittersWorkspace_sptr outws = boost::dynamic_pointer_cast<SplittersWorkspace>(
          AnalysisDataService::Instance().retrieve("SplittersFull"));
    TS_ASSERT(outws);


  }


};


#endif /* MANTID_ALGORITHMS_DIVIDESPLITTERSTEST_H_ */

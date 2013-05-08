#ifndef MANTID_ALGORITHMS_DIVIDESPLITTERSTEST_H_
#define MANTID_ALGORITHMS_DIVIDESPLITTERSTEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidAlgorithms/DivideSplitters.h"

#include "MantidAPI/TableRow.h"

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


  /** Test initialization
    */
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
    int64_t starttime = 20000;
    int64_t timestep = 1000;
    size_t numwsgroups = 10;
    size_t numsplitters = 100;
    SplittersWorkspace_sptr splitws = createSplittersWS(starttime, timestep, numwsgroups, numsplitters);
    Workspace_sptr infows = createInfoWS(numwsgroups);

    AnalysisDataService::Instance().addOrReplace("SplittersFull", splitws);
    AnalysisDataService::Instance().addOrReplace("InfomationFull", infows);

    TS_ASSERT_EQUALS(splitws->rowCount(), numsplitters);

    // Call algorithm
    DivideSplitters divider;
    divider.initialize();;

    divider.setProperty("InputWorkspace", "SplittersFull");
    divider.setProperty("InfoTableWorkspace", "InfomationFull");
    divider.setProperty("NumberOfSegments", 5);
    divider.setProperty("WorkspaceIndex", 3);
    divider.setProperty("OutputWorkspace", "NewSplitters");
    divider.setProperty("OutputInfoWorkspace", "NewInfomationTable");

    divider.execute();
    TS_ASSERT(divider.isExecuted());

    // Check result
    SplittersWorkspace_sptr outws = boost::dynamic_pointer_cast<SplittersWorkspace>(
          AnalysisDataService::Instance().retrieve("NewSplitters"));
    TS_ASSERT(outws);

    // Number of splitters
    TS_ASSERT_EQUALS(outws->rowCount(), numsplitters/numwsgroups*5);

    // Check some individual splitters
    for (size_t i = 0; i < 10; ++i)
    {
      // Every 5 + 1 row
      int64_t t0, tf;
      TableRow splitterrow = outws->getRow(i*5+1);
      splitterrow >> t0 >> tf;
      cout << i << ": t0 = " << t0 << "\n";
      TS_ASSERT_EQUALS(t0, starttime + 3*1000 + i*timestep/numwsgroups + timestep);
      TS_ASSERT_EQUALS(tf, starttime + 3*1000 + i*timestep/numwsgroups + timestep*2);
    }

    // Check the cyclic of workspace gorup index
    for (size_t i = 1; i < 5; ++i)
    {
      int64_t t0, tf;
      int prevgroupindex, currgroupindex;

      TableRow prevrow = outws->getRow(i*5+1-5);
      prevrow >> t0 >> tf >> prevgroupindex;
      TableRow currrow = outws->getRow(i*5+1);
      currrow >> t0 >> tf >> currgroupindex;
      TS_ASSERT_EQUALS(prevgroupindex, currgroupindex);
    }

    // Check the increment of workspace group index
    for (int ws = 0; ws < static_cast<int>(numwsgroups); ++ws)
    {
      int64_t t0, tf;
      int groupindex;
      TableRow row = outws->getRow(ws);
      row >> t0 >> tf >> groupindex;
      TS_ASSERT_EQUALS(groupindex, ws);
    }

    return;
  }

  /** Generate a Splitters workspace
    * @param starttime :: start time of the first splitter
    * @param timestep :: duration of each splitter
    * @param numwsgroups :: number of workspace groups in the index
    * @param numsplitters :: number of splitters
    * From the first splitter, the workspace group index changes by 1 cyclicly.
    */
  SplittersWorkspace_sptr createSplittersWS(int64_t starttime, int64_t timestep, size_t numwsgroups, size_t numsplitters)
  {
    SplittersWorkspace_sptr splitws(new SplittersWorkspace());

    int wsindex = 0;

    int64_t ti = starttime;

    for (size_t i = 0; i < numsplitters; ++i)
    {
      TableRow newsplitter = splitws->appendRow();

      int64_t tf = ti + timestep;
      newsplitter << ti << tf << wsindex;

      ti = tf;
      ++ wsindex;
      if (wsindex >= static_cast<int>(numwsgroups))
        wsindex = 0;
    }

    return splitws;
  }

  /** Create information workspace
    * @param numwsgroups :: number of workspace groups
    */
  TableWorkspace_sptr createInfoWS(size_t numwsgroups)
  {
    TableWorkspace_sptr infows(new TableWorkspace());
    infows->addColumn("int", "workspacegroup");
    infows->addColumn("str", "title");

    for (size_t i = 0; i < numwsgroups; ++i)
    {
      TableRow newrow = infows->appendRow();
      int wsgroup = static_cast<int>(i);
      newrow << wsgroup << "Blablabal";
    }

    return infows;
  }

};


#endif /* MANTID_ALGORITHMS_DIVIDESPLITTERSTEST_H_ */

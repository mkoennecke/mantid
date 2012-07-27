#ifndef MANTID_CURVEFITTING_QUADRATICBACKGROUNDTEST_H_
#define MANTID_CURVEFITTING_QUADRATICBACKGROUNDTEST_H_

#include <cxxtest/TestSuite.h>
#include "MantidKernel/Timer.h"
#include "MantidKernel/System.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidAPI/WorkspaceFactory.h"
#include "MantidCurveFitting/Fit.h"
#include <iostream>
#include <iomanip>

#include "MantidCurveFitting/QuadraticBackground.h"

using namespace Mantid;
using namespace Mantid::CurveFitting;
using namespace Mantid::API;

class QuadraticBackgroundTest: public CxxTest::TestSuite
{
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static QuadraticBackgroundTest *createSuite()
  {
    return new QuadraticBackgroundTest();
  }
  static void destroySuite(QuadraticBackgroundTest *suite)
  {
    delete suite;
  }

  void test_LinearBackground()
  {
    // create mock data to test against
    std::string wsName = "LinearBackgroundTest";
    int histogramNumber = 1;
    int timechannels = 5;
    DataObjects::Workspace2D_sptr ws2D =
        boost::dynamic_pointer_cast<DataObjects::Workspace2D>(
            API::WorkspaceFactory::Instance().create("Workspace2D", histogramNumber, timechannels, timechannels));
    for (int i = 0; i < timechannels; i++)
    {
      ws2D->dataX(0)[i] = i + 1;
      ws2D->dataY(0)[i] = i + 1;
      ws2D->dataE(0)[i] = 1.0;
    }

    //put this workspace in the data service
    TS_ASSERT_THROWS_NOTHING(AnalysisDataService::Instance().add(wsName, ws2D));

    CurveFitting::Fit alg2;
    TS_ASSERT_THROWS_NOTHING(alg2.initialize());
    TS_ASSERT( alg2.isInitialized() );

    // set up fitting function
    IFunction_sptr quadB(new QuadraticBackground());
    quadB->initialize();

    quadB->setParameter("A0", 0.0);
    quadB->setParameter("A1", 1.0);
    quadB->setParameter("A2", 1.0);

    //alg2.setFunction(linB);
    alg2.setProperty("Function", quadB);

    // Set which spectrum to fit against and initial starting values
    alg2.setPropertyValue("InputWorkspace", wsName);
    alg2.setPropertyValue("WorkspaceIndex", "0");

    // execute fit
    TS_ASSERT_THROWS_NOTHING(TS_ASSERT( alg2.execute() ))

    TS_ASSERT( alg2.isExecuted() );

    // test the output from fit is what you expect
    double dummy = alg2.getProperty("OutputChi2overDoF");

    TS_ASSERT_DELTA( dummy, 0.0,0.1);
    IFunction_sptr out = alg2.getProperty("Function");
    TS_ASSERT_DELTA( out->getParameter("A0"), 0.0, 0.01);
    TS_ASSERT_DELTA( out->getParameter("A1"), 1.0, 0.0003);
    TS_ASSERT_DELTA( out->getParameter("A2"), 0.0, 0.01);

    // check its categories
    const std::vector<std::string> categories = out->categories();
    TS_ASSERT( categories.size() == 1 );
    TS_ASSERT( categories[0] == "Background" );

    // Clean up
    AnalysisDataService::Instance().remove(wsName);

    return;
  }


  void test_QuadraticBackground()
  {
    // create mock data to test against
    std::string wsName = "QuadraticBackgroundTest";
    int histogramNumber = 1;
    int timechannels = 5;
    DataObjects::Workspace2D_sptr ws2D =
        boost::dynamic_pointer_cast<DataObjects::Workspace2D>(
            API::WorkspaceFactory::Instance().create("Workspace2D", histogramNumber, timechannels, timechannels));
    for (int i = 0; i < timechannels; i++)
    {
      ws2D->dataX(0)[i] = i + 1;
      ws2D->dataY(0)[i] = (i + 1)*(i + 1) + 2*(i+1) + 3.0;
      ws2D->dataE(0)[i] = 1.0;
    }

    //put this workspace in the data service
    TS_ASSERT_THROWS_NOTHING(AnalysisDataService::Instance().add(wsName, ws2D));

    CurveFitting::Fit alg2;
    TS_ASSERT_THROWS_NOTHING(alg2.initialize());
    TS_ASSERT( alg2.isInitialized() );

    // set up fitting function
    IFunction_sptr quadB(new QuadraticBackground());
    quadB->initialize();

    quadB->setParameter("A0", 0.0);
    quadB->setParameter("A1", 1.0);
    quadB->setParameter("A2", 1.0);

    //alg2.setFunction(linB);
    alg2.setProperty("Function", quadB);

    // Set which spectrum to fit against and initial starting values
    alg2.setPropertyValue("InputWorkspace", wsName);
    alg2.setPropertyValue("WorkspaceIndex", "0");

    // execute fit
    TS_ASSERT_THROWS_NOTHING(TS_ASSERT( alg2.execute() ))

    TS_ASSERT( alg2.isExecuted() );

    // test the output from fit is what you expect
    double dummy = alg2.getProperty("OutputChi2overDoF");

    TS_ASSERT_DELTA( dummy, 0.0,0.1);
    IFunction_sptr out = alg2.getProperty("Function");
    TS_ASSERT_DELTA( out->getParameter("A0"), 3.0, 0.01);
    TS_ASSERT_DELTA( out->getParameter("A1"), 2.0, 0.0003);
    TS_ASSERT_DELTA( out->getParameter("A2"), 1.0, 0.01);

    // check its categories
    const std::vector<std::string> categories = out->categories();
    TS_ASSERT( categories.size() == 1 );
    TS_ASSERT( categories[0] == "Background" );

    return;
  }

  void testForCategories()
  {
    QuadraticBackground forCat;
    const std::vector<std::string> categories = forCat.categories();
    TS_ASSERT( categories.size() == 1 );
    TS_ASSERT( categories[0] == "Background" );
  }

};

#endif /* MANTID_CURVEFITTING_QUADRATICBACKGROUNDTEST_H_ */


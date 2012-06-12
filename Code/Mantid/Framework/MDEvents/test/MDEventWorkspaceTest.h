#ifndef MDEVENTWORKSPACETEST_H
#define MDEVENTWORKSPACETEST_H

#include "MantidAPI/IMDIterator.h"
#include "MantidAPI/ITableWorkspace.h"
#include "MantidGeometry/MDGeometry/MDDimensionExtents.h"
#include "MantidGeometry/MDGeometry/MDHistoDimension.h"
#include "MantidGeometry/MDGeometry/MDBoxImplicitFunction.h"
#include "MantidKernel/ProgressText.h"
#include "MantidKernel/Timer.h"
#include "MantidAPI/BoxController.h"
#include "MantidMDEvents/CoordTransformDistance.h"
#include "MantidMDEvents/MDBox.h"
#include "MantidMDEvents/MDEventFactory.h"
#include "MantidMDEvents/MDEventWorkspace.h"
#include "MantidMDEvents/MDGridBox.h"
#include "MantidMDEvents/MDLeanEvent.h"
#include "MantidTestHelpers/MDEventsTestHelper.h"
#include <boost/random/linear_congruential.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <cxxtest/TestSuite.h>
#include <map>
#include <memory>
#include <vector>
#include <boost/math/special_functions/fpclassify.hpp>

using namespace Mantid;
using namespace Mantid::Kernel;
using namespace Mantid::MDEvents;
using namespace Mantid::API;
using namespace Mantid::Geometry;

class MDEventWorkspaceTest :    public CxxTest::TestSuite
{
private:
    /// Helper function to return the number of masked bins in a workspace. TODO: move helper into test helpers
  size_t getNumberMasked(Mantid::API::IMDWorkspace_sptr ws)
  {
    Mantid::API::IMDIterator* it = ws->createIterator(NULL);
    size_t numberMasked = 0;
    size_t counter = 0;
    for(;counter < it->getDataSize(); ++counter)
    {
      if(it->getIsMasked())
      {
        ++numberMasked;
      }
      it->next(1); //Doesn't perform skipping on masked, bins, but next() does.
    }
    return numberMasked;
    delete it;
  }

public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static MDEventWorkspaceTest *createSuite() { return new MDEventWorkspaceTest(); }
  static void destroySuite( MDEventWorkspaceTest *suite ) { delete suite; }

  bool DODEBUG;
  MDEventWorkspaceTest()
  {
    DODEBUG = false;
  }


  void test_constructor()
  {
    MDEventWorkspace<MDLeanEvent<3>, 3> ew3;
    TS_ASSERT_EQUALS( ew3.getNumDims(), 3);
    TS_ASSERT_EQUALS( ew3.getNPoints(), 0);
    TS_ASSERT_EQUALS( ew3.id(), "MDEventWorkspace<MDLeanEvent,3>");
    // Box controller MUST always be present
    TS_ASSERT(ew3.getBoxController() );
    TS_ASSERT(ew3.getBox());
    TS_ASSERT(ew3.getBox()->getBoxController());
    TS_ASSERT_EQUALS(ew3.getBox()->getId(), 0);

    // Now with the MDEvent type
    MDEventWorkspace<MDEvent<3>, 3> ew3b;
    TS_ASSERT_EQUALS( ew3b.id(), "MDEventWorkspace<MDEvent,3>");
  }

  void test_constructor_IMDEventWorkspace()
  {
    IMDEventWorkspace * ew3 = new MDEventWorkspace<MDLeanEvent<3>, 3>();
    TS_ASSERT_EQUALS( ew3->getNumDims(), 3);
    TS_ASSERT_EQUALS( ew3->getNPoints(), 0);
    delete ew3;
  }

  void test_copy_constructor()
  {
    MDEventWorkspace<MDLeanEvent<3>, 3> ew3;
    for (size_t i=0; i<3; i++)
      ew3.addDimension( MDHistoDimension_sptr(new MDHistoDimension("x","x","m",-1,1,0)) );
    ew3.initialize();
    ew3.addEvent( MDLeanEvent<3>(1.23, 4.56) );
    ExperimentInfo_sptr ei(new ExperimentInfo);
    TS_ASSERT_EQUALS( ew3.addExperimentInfo(ei), 0);

    MDEventWorkspace<MDLeanEvent<3>, 3> copy(ew3);
    TS_ASSERT( !copy.isGridBox());
    TS_ASSERT_EQUALS( copy.getNumDims(), 3);
    TS_ASSERT_EQUALS( copy.getDimension(0)->getName(), "x");
    TS_ASSERT_EQUALS( copy.getNumExperimentInfo(), 1);
    TSM_ASSERT_DIFFERS( "ExperimentInfo's were deep-copied", copy.getExperimentInfo(0), ew3.getExperimentInfo(0));
    TSM_ASSERT_DIFFERS( "BoxController was deep-copied", copy.getBoxController(), ew3.getBoxController());
    TSM_ASSERT_DIFFERS( "Dimensions were deep-copied", copy.getDimension(0), ew3.getDimension(0));
  }

  void test_initialize_throws()
  {
    IMDEventWorkspace * ew = new MDEventWorkspace<MDLeanEvent<3>, 3>();
    TS_ASSERT_THROWS( ew->initialize(), std::runtime_error);
    for (size_t i=0; i<5; i++)
          ew->addDimension( MDHistoDimension_sptr(new MDHistoDimension("x","x","m",-1,1,0)) );
        TS_ASSERT_THROWS( ew->initialize(), std::runtime_error);
    delete ew;
  }

  void test_initialize()
  {
    IMDEventWorkspace * ew = new MDEventWorkspace<MDLeanEvent<3>, 3>();
    TS_ASSERT_THROWS( ew->initialize(), std::runtime_error);
    for (size_t i=0; i<3; i++)
      ew->addDimension( MDHistoDimension_sptr(new MDHistoDimension("x","x","m",-1,1,0)) );
    TS_ASSERT_THROWS_NOTHING( ew->initialize() );
    delete ew;
  }


  //-------------------------------------------------------------------------------------
  /** Split the main box into a grid box */
  void test_splitBox()
  {
    MDEventWorkspace3 * ew = new MDEventWorkspace3();
    BoxController_sptr bc = ew->getBoxController();
    bc->setSplitInto(4);
    TS_ASSERT( !ew->isGridBox() );
    TS_ASSERT_THROWS_NOTHING( ew->splitBox(); )
    TS_ASSERT( ew->isGridBox() );
    delete ew;
  }


  //-------------------------------------------------------------------------------------
  /** MDBox->addEvent() tracks when a box is too big.
   * */
  void test_trackBoxes()
  {
    MDEventWorkspace1Lean::sptr ew = MDEventsTestHelper::makeMDEW<1>(2, 0.0, 1.0, 0);
    BoxController_sptr bc = ew->getBoxController();
    bc->setSplitInto(2);
    bc->setSplitThreshold(100);
    ew->splitBox();

    typedef MDGridBox<MDLeanEvent<1>,1> gbox_t;
    typedef MDBox<MDLeanEvent<1>,1> box_t;
    typedef MDBoxBase<MDLeanEvent<1>,1> ibox_t;

    // Make 99 events
    coord_t centers[1] = {0};
    for (size_t i=0; i<99; i++)
    {
      centers[0] = coord_t(i*0.001);
      ew->addEvent(MDEvent<1>(1.0, 1.0, centers) );
    }
    TS_ASSERT_EQUALS( bc->getBoxesToSplit().size(), 0);

    // The 100th event triggers the adding to the list
    ew->addEvent(MDEvent<1>(1.0, 1.0, centers) );
    TS_ASSERT_EQUALS( bc->getBoxesToSplit().size(), 1);
  }

  void test_splitTrackedBoxes()
  {
    MDEventWorkspace1Lean::sptr ew = MDEventsTestHelper::makeMDEW<1>(2, 0.0, 1.0, 0);
    BoxController_sptr bc = ew->getBoxController();
    bc->setSplitThreshold(10);
    ew->splitBox();

    coord_t centers[1] = {0};
    for (size_t i=0; i<10; i++)
    {
      centers[0] = coord_t(i*0.001);
      ew->addEvent(MDEvent<1>(1.0, 1.0, centers) );
    }

    TS_ASSERT_EQUALS( bc->getBoxesToSplit().size(), 1);

    const size_t nOriginalGriddedBoxes = bc->getTotalNumMDGridBoxes();
    const size_t nOriginalMDBoxes = bc->getTotalNumMDBoxes();

    TSM_ASSERT_THROWS_NOTHING("splitTrackedBoxes should not throw anything.", ew->splitTrackedBoxes(NULL));
    TSM_ASSERT_EQUALS("Hit list of boxes to split should be cleared after splitting.", bc->getBoxesToSplit().size(), 0);

    const size_t nCurrrentGriddedBoxes = bc->getTotalNumMDGridBoxes();
    const size_t nCurrentMDBoxes = bc->getTotalNumMDBoxes();

    //Splitting should lead to more boxes being generated than we had originally.
    TS_ASSERT_LESS_THAN(nOriginalGriddedBoxes, nCurrrentGriddedBoxes);
    TS_ASSERT_LESS_THAN(nOriginalMDBoxes, nCurrentMDBoxes);
  }

  //-------------------------------------------------------------------------------------
  /** Test that splitTrackedBoxes does the same thing as the older splitAllIfNeeded */
  void test_consistencySplitTrackedBoxes()
  {
    MDEventWorkspace1Lean::sptr A = MDEventsTestHelper::makeMDEW<1>(2, 0.0, 1.0, 0);
    BoxController_sptr bc = A->getBoxController();
    bc->setSplitThreshold(2);
    A->splitBox();

    coord_t centers[1] = {0};
    for (size_t i=0; i<10; i++)
    {
      centers[0] = coord_t(i*0.001);
      A->addEvent(MDEvent<1>(1.0, 1.0, centers) );
    }

    MDEventWorkspace1Lean::sptr cloneA = MDEventWorkspace1Lean::sptr(new MDEventWorkspace1Lean(*A));

    A->splitAllIfNeeded(NULL);
    cloneA->splitTrackedBoxes(NULL);

    Mantid::API::BoxController_sptr ABoxController = A->getBoxController();
    BoxController_sptr cloneABoxController = A->getBoxController();

    //Splitting either way should yield the same results.
    TS_ASSERT_EQUALS(ABoxController->getAverageDepth(), cloneABoxController->getAverageDepth());
    TS_ASSERT_EQUALS(ABoxController->getMaxDepth(), cloneABoxController->getMaxDepth());
    TS_ASSERT_EQUALS(ABoxController->getMaxId(), cloneABoxController->getMaxId());
    TS_ASSERT_EQUALS(ABoxController->getTotalNumMDBoxes(), cloneABoxController->getTotalNumMDBoxes());
    TS_ASSERT_EQUALS(ABoxController->getTotalNumMDGridBoxes(), cloneABoxController->getTotalNumMDGridBoxes());
  }

  //-------------------------------------------------------------------------------------
  /** Runs in a multithreaded context, gets the same results as in the single threaded context. */
  void test_spitTrackedBoxesParallel()
  {
    MDEventWorkspace1Lean::sptr A = MDEventsTestHelper::makeMDEW<1>(2, 0.0, 1.0, 0);
    BoxController_sptr bc = A->getBoxController();
    bc->setSplitThreshold(2);
    A->splitBox();

    coord_t centers[1] = {0};
    for (size_t i=0; i<1000; i++)
    {
      centers[0] = coord_t(i*0.0001);
      A->addEvent(MDEvent<1>(1.0, 1.0, centers) );
    }

    //Now we should have two identical input workspaces.
    MDEventWorkspace1Lean::sptr cloneA = MDEventWorkspace1Lean::sptr(new MDEventWorkspace1Lean(*A));

    //Split in single threaded context
    A->splitTrackedBoxes(NULL);

    //Split in multithreaded context
    ThreadScheduler* tScheduler = new ThreadSchedulerFIFO();
    ThreadPool tPool(tScheduler);
    cloneA->splitTrackedBoxes(tScheduler);
    tPool.joinAll();

    //Get the respective box controllers prior to comparsison
    Mantid::API::BoxController_sptr ABoxController = A->getBoxController();
    BoxController_sptr cloneABoxController = A->getBoxController();
  
    //Compare via the box controller.
    TS_ASSERT_EQUALS(ABoxController->getTotalNumMDBoxes(), cloneABoxController->getTotalNumMDBoxes());
    TS_ASSERT_EQUALS(ABoxController->getTotalNumMDGridBoxes(), cloneABoxController->getTotalNumMDGridBoxes());
    TS_ASSERT_EQUALS(ABoxController->getAverageDepth(), cloneABoxController->getAverageDepth());
    TS_ASSERT_EQUALS(ABoxController->getMaxDepth(), cloneABoxController->getMaxDepth());
  }


  //-------------------------------------------------------------------------------------
  /** Create an IMDIterator */
  void test_createIterator()
  {
    MDEventWorkspace3 * ew = new MDEventWorkspace3();
    BoxController_sptr bc = ew->getBoxController();
    bc->setSplitInto(4);
    ew->splitBox();
    IMDIterator * it = ew->createIterator();
    TS_ASSERT(it);
    TS_ASSERT_EQUALS(it->getDataSize(), 4*4*4);
    TS_ASSERT(it->next());
    delete it;
    it = ew->createIterator(new MDImplicitFunction());
    TS_ASSERT(it);
    TS_ASSERT_EQUALS(it->getDataSize(), 4*4*4);
    TS_ASSERT(it->next());
    delete ew;
  }

  //-------------------------------------------------------------------------------------
  /** Create several IMDIterators to run them in parallel */
  void test_createIterators()
  {
    MDEventWorkspace3 * ew = new MDEventWorkspace3();
    BoxController_sptr bc = ew->getBoxController();
    bc->setSplitInto(4);
    ew->splitBox();
    std::vector<IMDIterator *> iterators = ew->createIterators(3);
    TS_ASSERT_EQUALS(iterators.size(), 3);

    TS_ASSERT_EQUALS(iterators[0]->getDataSize(), 21);
    TS_ASSERT_EQUALS(iterators[1]->getDataSize(), 21);
    TS_ASSERT_EQUALS(iterators[2]->getDataSize(), 22);
  }

  //-------------------------------------------------------------------------------------
  /** Method that makes a table workspace for use in MantidPlot */
  void test_makeBoxTable()
  {
    MDEventWorkspace3Lean::sptr ew = MDEventsTestHelper::makeMDEW<3>(4, 0.0, 4.0, 1);
    ITableWorkspace_sptr itab = ew->makeBoxTable(0,0);
    TS_ASSERT_EQUALS( itab->rowCount(), 4*4*4+1);
    TS_ASSERT_EQUALS( itab->cell<int>(3,0), 3);
  }


  //-------------------------------------------------------------------------------------
  /** Get the signal at a given coord */
  void test_getSignalAtCoord()
  {
    MDEventWorkspace3Lean::sptr ew = MDEventsTestHelper::makeMDEW<3>(4, 0.0, 4.0, 1);
    coord_t coords1[3] = {1.5,1.5,1.5};
    coord_t coords2[3] = {2.5,2.5,2.5};
    coord_t coords3[3] = {-0.1f, 2, 2};
    coord_t coords4[3] = {2, 2, 4.1f};
    ew->addEvent(MDLeanEvent<3>(2.0, 2.0, coords2));
    ew->refreshCache();
    TSM_ASSERT_DELTA("A regular box with a single event", ew->getSignalAtCoord(coords1, Mantid::API::NoNormalization), 1.0, 1e-5);
    TSM_ASSERT_DELTA("The box with 2 events", ew->getSignalAtCoord(coords2, Mantid::API::NoNormalization), 3.0, 1e-5);
    TSM_ASSERT("Out of bounds returns NAN", boost::math::isnan( ew->getSignalAtCoord(coords3, Mantid::API::NoNormalization) ) );
    TSM_ASSERT("Out of bounds returns NAN", boost::math::isnan( ew->getSignalAtCoord(coords4, Mantid::API::NoNormalization) ) );
  }


  //-------------------------------------------------------------------------------------
  void test_estimateResolution()
  {
    MDEventWorkspace2Lean::sptr b = MDEventsTestHelper::makeMDEW<2>(10, 0.0, 10.0);
    std::vector<coord_t> binSizes;
    // First, before any splitting
    binSizes = b->estimateResolution();
    TS_ASSERT_EQUALS( binSizes.size(), 2);
    TS_ASSERT_DELTA( binSizes[0], 10.0, 1e-6);
    TS_ASSERT_DELTA( binSizes[1], 10.0, 1e-6);

    // Resolution is smaller after splitting
    b->splitBox();
    binSizes = b->estimateResolution();
    TS_ASSERT_EQUALS( binSizes.size(), 2);
    TS_ASSERT_DELTA( binSizes[0], 1.0, 1e-6);
    TS_ASSERT_DELTA( binSizes[1], 1.0, 1e-6);
  }

  //-------------------------------------------------------------------------------------
  /** Fill a 10x10 gridbox with events
   *
   * Tests that bad events are thrown out when using addEvents.
   * */
  void test_addManyEvents()
  {
    ProgressText * prog = NULL;
    if (DODEBUG) prog = new ProgressText(0.0, 1.0, 10, false);

    typedef MDGridBox<MDLeanEvent<2>,2> box_t;
    MDEventWorkspace2Lean::sptr b = MDEventsTestHelper::makeMDEW<2>(10, 0.0, 10.0);
    box_t * subbox;

    // Manually set some of the tasking parameters
    b->getBoxController()->setAddingEvents_eventsPerTask(1000);
    b->getBoxController()->setAddingEvents_numTasksPerBlock(20);
    b->getBoxController()->setSplitThreshold(100);
    b->getBoxController()->setMaxDepth(4);

    std::vector< MDLeanEvent<2> > events;
    size_t num_repeat = 1000;
    // Make an event in the middle of each box
    for (double x=0.0005; x < 10; x += 1.0)
      for (double y=0.0005; y < 10; y += 1.0)
      {
        for (size_t i=0; i < num_repeat; i++)
        {
          coord_t centers[2] = {static_cast<coord_t>(x), static_cast<coord_t>(y)};
          events.push_back( MDLeanEvent<2>(2.0, 2.0, centers) );
        }
      }
    TS_ASSERT_EQUALS( events.size(), 100*num_repeat);

    TS_ASSERT_THROWS_NOTHING( b->addManyEvents( events, prog ); );
    TS_ASSERT_EQUALS( b->getNPoints(), 100*num_repeat);
    TS_ASSERT_EQUALS( b->getBox()->getSignal(), 100*double(num_repeat)*2.0);
    TS_ASSERT_EQUALS( b->getBox()->getErrorSquared(), 100*double(num_repeat)*2.0);

    box_t * gridBox = dynamic_cast<box_t *>(b->getBox());
    std::vector<MDBoxBase<MDLeanEvent<2>,2>*> boxes = gridBox->getBoxes();
    TS_ASSERT_EQUALS( boxes[0]->getNPoints(), num_repeat);
    // The box should have been split itself into a gridbox, because 1000 events > the split threshold.
    subbox = dynamic_cast<box_t *>(boxes[0]);
    TS_ASSERT( subbox ); if (!subbox) return;
    // The sub box is at a depth of 1.
    TS_ASSERT_EQUALS( subbox->getDepth(), 1);

    // And you can keep recursing into the box.
    boxes = subbox->getBoxes();
    subbox = dynamic_cast<box_t *>(boxes[0]);
    TS_ASSERT( subbox ); if (!subbox) return;
    TS_ASSERT_EQUALS( subbox->getDepth(), 2);

    // And so on (this type of recursion was checked in test_splitAllIfNeeded()
    if (prog) delete prog;
  }


  void checkExtents( std::vector<Mantid::Geometry::MDDimensionExtents> & ext, coord_t xmin, coord_t xmax, coord_t ymin, coord_t ymax)
  {
    TS_ASSERT_DELTA( ext[0].min, xmin, 1e-4);
    TS_ASSERT_DELTA( ext[0].max, xmax, 1e-4);
    TS_ASSERT_DELTA( ext[1].min, ymin, 1e-4);
    TS_ASSERT_DELTA( ext[1].max, ymax, 1e-4);
  }

  void addEvent(MDEventWorkspace2Lean::sptr b, double x, double y)
  {
    coord_t centers[2] = {static_cast<coord_t>(x), static_cast<coord_t>(y)};
    b->addEvent(MDLeanEvent<2>(2.0, 2.0, centers));
  }

  void test_getMinimumExtents()
  {
    MDEventWorkspace2Lean::sptr ws = MDEventsTestHelper::makeMDEW<2>(10, 0.0, 10.0);
    std::vector<Mantid::Geometry::MDDimensionExtents> ext;

    // If nothing in the workspace, the extents given are the dimensions in the workspace
    ext = ws->getMinimumExtents(2);
    TS_ASSERT_DELTA( ext[0].min, 0.0, 1e-5 );
    TS_ASSERT_DELTA( ext[0].max, 10.0, 1e-5 );
    TS_ASSERT_DELTA( ext[1].min, 0.0, 1e-5 );
    TS_ASSERT_DELTA( ext[1].max, 10.0, 1e-5 );

    std::vector< MDLeanEvent<2> > events;
    // Make an event in the middle of each box
    for (double x=4.0005; x < 7; x += 1.0)
      for (double y=4.0005; y < 7; y += 1.0)
      {
        double centers[2] = {x, y};
        events.push_back( MDLeanEvent<2>(2.0, 2.0, centers) );
      }
    // So it doesn't split
    ws->getBoxController()->setSplitThreshold(1000);
    ws->addManyEvents( events, NULL );
    ws->refreshCache();

    // Base extents
    ext = ws->getMinimumExtents(2);
    checkExtents(ext, 4, 7,  4, 7);

    // Start adding events to make the extents bigger
    addEvent(ws, 3.5, 5.0);
    ext = ws->getMinimumExtents(2);
    checkExtents(ext, 3, 7,  4, 7);

    addEvent(ws, 8.5, 7.9);
    ext = ws->getMinimumExtents(2);
    checkExtents(ext, 3, 9,  4, 8);

    addEvent(ws, 0.5, 0.9);
    ext = ws->getMinimumExtents(2);
    checkExtents(ext, 0, 9,  0, 8);

  }

//
//  //-------------------------------------------------------------------------------------
//  /** Tests that bad events are thrown out when using addEvents.
//   * */
//  void test_addManyEvents_Performance()
//  {
//    // This test is too slow for unit tests, so it is disabled except in debug mode.
//    if (!DODEBUG) return;
//
//    ProgressText * prog = new ProgressText(0.0, 1.0, 10, true);
//    prog->setNotifyStep(0.5); //Notify more often
//
//    typedef MDGridBox<MDLeanEvent<2>,2> box_t;
//    box_t * b = makeMDEW<2>(10, 0.0, 10.0);
//
//    // Manually set some of the tasking parameters
//    b->getBoxController()->m_addingEvents_eventsPerTask = 50000;
//    b->getBoxController()->m_addingEvents_numTasksPerBlock = 50;
//    b->getBoxController()->m_SplitThreshold = 1000;
//    b->getBoxController()->m_maxDepth = 6;
//
//    Timer tim;
//    std::vector< MDLeanEvent<2> > events;
//    double step_size = 1e-3;
//    size_t numPoints = (10.0/step_size)*(10.0/step_size);
//    std::cout << "Starting to write out " << numPoints << " events\n";
//    if (true)
//    {
//      // ------ Make an event in the middle of each box ------
//      for (double x=step_size; x < 10; x += step_size)
//        for (double y=step_size; y < 10; y += step_size)
//        {
//          double centers[2] = {x, y};
//          events.push_back( MDLeanEvent<2>(2.0, 3.0, centers) );
//        }
//    }
//    else
//    {
//      // ------- Randomize event distribution ----------
//      boost::mt19937 rng;
//      boost::uniform_real<float> u(0.0, 10.0); // Range
//      boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > gen(rng, u);
//
//      for (size_t i=0; i < numPoints; i++)
//      {
//        double centers[2] = {gen(), gen()};
//        events.push_back( MDLeanEvent<2>(2.0, 3.0, centers) );
//      }
//    }
//    TS_ASSERT_EQUALS( events.size(), numPoints);
//    std::cout << "..." << numPoints << " events were filled in " << tim.elapsed() << " secs.\n";
//
//    size_t numbad = 0;
//    TS_ASSERT_THROWS_NOTHING( numbad = b->addManyEvents( events, prog); );
//    TS_ASSERT_EQUALS( numbad, 0);
//    TS_ASSERT_EQUALS( b->getNPoints(), numPoints);
//    TS_ASSERT_EQUALS( b->getSignal(), numPoints*2.0);
//    TS_ASSERT_EQUALS( b->getErrorSquared(), numPoints*3.0);
//
//    std::cout << "addManyEvents() ran in " << tim.elapsed() << " secs.\n";
//  }


  void test_integrateSphere()
  {
    // 10x10x10 eventWorkspace
    MDEventWorkspace3Lean::sptr ws = MDEventsTestHelper::makeMDEW<3>(10, 0.0, 10.0, 1 /*event per box*/);
    TS_ASSERT_EQUALS( ws->getNPoints(), 1000);

    // The sphere transformation
    coord_t center[3] = {0,0,0};
    bool dimensionsUsed[3] = {true,true,true};
    CoordTransformDistance sphere(3, center, dimensionsUsed);

    signal_t signal = 0;
    signal_t errorSquared = 0;
    ws->getBox()->integrateSphere(sphere, 1.0, signal, errorSquared);

    //TODO:
//    TS_ASSERT_DELTA( signal, 1.0, 1e-5);
//    TS_ASSERT_DELTA( errorSquared, 1.0, 1e-5);


  }

  /*
  Generic masking checking helper method.
  */
  void doTestMasking(MDImplicitFunction* function, size_t expectedNumberMasked)
  {
    // 10x10x10 eventWorkspace
    MDEventWorkspace3Lean::sptr ws = MDEventsTestHelper::makeMDEW<3>(10, 0.0, 10.0, 1 /*event per box*/);

    ws->setMDMasking(function);

    size_t numberMasked = getNumberMasked(ws);
    TSM_ASSERT_EQUALS("Didn't perform the masking as expected", expectedNumberMasked, numberMasked);
  }
  
  void test_maskEverything()
  {
    std::vector<coord_t> min;
    std::vector<coord_t> max;

    min.push_back(0);
    min.push_back(0);
    min.push_back(0);
    max.push_back(10);
    max.push_back(10);
    max.push_back(10);

    //Create an function that encompases 1/4 of the total bins.
    MDImplicitFunction* function = new MDBoxImplicitFunction(min, max);

    doTestMasking(function, 1000); //1000 out of 1000 bins masked
  }

  void test_maskNULL()
  {
    //Should do nothing in terms of masking, but should not throw.
    doTestMasking(NULL, 0); //0 out of 1000 bins masked
  }

  void test_maskNothing()
  {
    std::vector<coord_t> min;
    std::vector<coord_t> max;

    //Make the box lay over a non-intersecting region of space.
    min.push_back(-1);
    min.push_back(-1);
    min.push_back(-1);
    max.push_back(-0.01f);
    max.push_back(-0.01f);
    max.push_back(-0.01f);

    //Create an function that encompases 1/4 of the total bins.
    MDImplicitFunction* function = new MDBoxImplicitFunction(min, max);

    doTestMasking(function, 0); //0 out of 1000 bins masked
  }

  void test_maskHalf()
  {
    std::vector<coord_t> min;
    std::vector<coord_t> max;

    //Make the box that covers half the bins in the workspace. 
    min.push_back(0);
    min.push_back(0);
    min.push_back(0);
    max.push_back(10);
    max.push_back(10);
    max.push_back(4.99f);

    //Create an function that encompases 1/4 of the total bins.
    MDImplicitFunction* function = new MDBoxImplicitFunction(min, max);

    doTestMasking(function, 500); //500 out of 1000 bins masked
  }

  void test_clearMasking()
  {
    //Create a function that masks everything.
    std::vector<coord_t> min;
    std::vector<coord_t> max;
    min.push_back(0);
    min.push_back(0);
    min.push_back(0);
    max.push_back(10);
    max.push_back(10);
    max.push_back(10);
    MDImplicitFunction* function = new MDBoxImplicitFunction(min, max);

    MDEventWorkspace3Lean::sptr ws = MDEventsTestHelper::makeMDEW<3>(10, 0.0, 10.0, 1 /*event per box*/);
    ws->setMDMasking(function);

    TSM_ASSERT_EQUALS("Everything should be masked.", 1000, getNumberMasked(ws));
    TS_ASSERT_THROWS_NOTHING(ws->clearMDMasking());
    TSM_ASSERT_EQUALS("Nothing should be masked.", 0, getNumberMasked(ws));
  }
};

class MDEventWorkspaceTestPerformance :    public CxxTest::TestSuite
{

private:

  MDEventWorkspace3Lean::sptr m_WidelyUnsplit_Original;
  MDEventWorkspace3Lean::sptr m_ConcentratedUnsplit_Original;

  MDEventWorkspace3Lean::sptr m_WidelyUnsplit;
  MDEventWorkspace3Lean::sptr m_ConcentratedUnsplit;

  //Helper to create an event list.
  std::vector<MDLeanEvent<3> > createEvents(const size_t& dimExtents)
  {
    std::vector<MDLeanEvent<3> > vecEvents(dimExtents*dimExtents*dimExtents);
    for(size_t i = 0; i < dimExtents; ++i)
    {
      for(size_t j = 0; j < dimExtents; ++j)
      {
        for(size_t k = 0; k < dimExtents; ++k)
        {
          double centers[3] = {(double)i, (double)j, (double)k};
          size_t index = i + j*dimExtents + k*dimExtents;
          vecEvents[index] = MDLeanEvent<3>(1, 1, centers);
        }
      }
    }
    return vecEvents;
  }

public:

  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static MDEventWorkspaceTestPerformance *createSuite() { return new MDEventWorkspaceTestPerformance(); }
  static void destroySuite( MDEventWorkspaceTestPerformance *suite ) { delete suite; }


  MDEventWorkspaceTestPerformance()
  {    
    const size_t dim_size = 100;
 
    //Create an MDWorkspace with new events scattered everywhere.
    m_WidelyUnsplit_Original = MDEventsTestHelper::makeMDEW<3>(10, 0.0, (Mantid::coord_t)dim_size, 10 /*event per box*/);
    m_WidelyUnsplit_Original->getBoxController()->setSplitThreshold(1);
    m_WidelyUnsplit_Original->addEvents(createEvents(dim_size));
    
    //Create a new workpace based on the original, but with a more concentrated distribution of events
    m_ConcentratedUnsplit_Original = MDEventWorkspace3Lean::sptr(new MDEventWorkspace3Lean(*m_WidelyUnsplit_Original));
    m_ConcentratedUnsplit_Original->splitAllIfNeeded(NULL);
    m_ConcentratedUnsplit_Original->addEvents(createEvents(dim_size/2));
  }

  void setUp()
  {
    m_WidelyUnsplit = MDEventWorkspace3Lean::sptr(new MDEventWorkspace3Lean(*m_WidelyUnsplit_Original));
    m_ConcentratedUnsplit = MDEventWorkspace3Lean::sptr(new MDEventWorkspace3Lean(*m_ConcentratedUnsplit_Original));
  }

  void test_splitting_performance_single_threaded_on_wide_distribution()
  {
    m_WidelyUnsplit->splitAllIfNeeded(NULL);
  }

  void test_splitting_performance_single_threaded_on_narrow_distribution()
  {
    m_ConcentratedUnsplit->splitAllIfNeeded(NULL);
  }

  void test_splitting_tracked_boxes_performance_single_threaded_on_wide_distribution()
  {
    m_WidelyUnsplit->splitTrackedBoxes(NULL);
  }

  void test_splitting_tracked_boxes_performance_single_threaded_on_narrow_distribution()
  {
    m_ConcentratedUnsplit->splitTrackedBoxes(NULL);
  }
};

#endif

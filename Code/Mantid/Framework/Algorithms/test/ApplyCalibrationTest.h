#ifndef APPLYCALIBRATIONTEST_H_
#define APPLYCALIBRATIONTEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidDataHandling/LoadInstrument.h"
#include "MantidAPI/IAlgorithm.h"
#include "MantidAlgorithms/ApplyCalibration.h"
#include "MantidAPI/Workspace.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "WorkspaceCreationHelperTest.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/ITableWorkspace.h"
#include "MantidAPI/TableRow.h"
#include "MantidKernel/V3D.h"
#include "MantidGeometry/Instrument.h"
#include "MantidGeometry/Instrument/Component.h"
#include <stdexcept>

using namespace Mantid::Algorithms;
using namespace Mantid::API;
using namespace Mantid::Kernel;
using namespace Mantid::DataObjects;

class ApplyCalibrationTest : public CxxTest::TestSuite
{
public:

  void testName()
  {
    TS_ASSERT_EQUALS( appCalib.name(), "ApplyCalibration" )
  }

  void testInit()
  {
    appCalib.initialize();
    TS_ASSERT( appCalib.isInitialized() )
  }

  void testExec()
  {
     // Create workspace with paremeterised instrument
     Workspace2D_sptr ws = WorkspaceCreationHelper::create2DWorkspaceWithFullInstrument(3, 10, true);

     // Create Calibration Table
     ITableWorkspace_sptr posTableWs = WorkspaceFactory::Instance().createTable();
     posTableWs->addColumn("int","Detector ID");
     posTableWs->addColumn("V3D","Detector Position");

     for(int i=0; i < 3; ++i)
     {
       TableRow row = posTableWs->appendRow();
       row << i << V3D(1.0,0.01*i,1.0); 
     }
     TS_ASSERT_THROWS_NOTHING(appCalib.setProperty<Workspace2D_sptr>("Workspace", ws ));
     TS_ASSERT_THROWS_NOTHING(appCalib.setProperty<ITableWorkspace_sptr>("PositionTable", posTableWs ));
     try 
     {
        TS_ASSERT_EQUALS(appCalib.execute(),true);
     }
     catch(std::runtime_error & e)
     {
        TS_FAIL(e.what());
     }
     TS_ASSERT( appCalib.isExecuted() );

  }

private:
  ApplyCalibration appCalib;


};

#endif /*APPLYCALIBRATIONTEST_H_*/

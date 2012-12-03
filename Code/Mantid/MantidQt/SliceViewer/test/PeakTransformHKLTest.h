#ifndef SLICE_VIEWER_PEAKTRANSFORMHKL_TEST_H_
#define SLICE_VIEWER_PEAKTRANSFORMHKL_TEST_H_

#include <cxxtest/TestSuite.h>
#include "MantidQtSliceViewer/PeakTransformHKL.h"
#include <boost/make_shared.hpp>
#include <gmock/gmock.h>

using namespace MantidQt::SliceViewer;
using namespace Mantid;
using Mantid::Kernel::V3D;
using namespace testing;

class PeakTransformHKLTest : public CxxTest::TestSuite
{

private:

  /*------------------------------------------------------------
  Mock Peak
  ------------------------------------------------------------*/
  class MockIPeak : public Mantid::API::IPeak 
  {
  public:
    MOCK_METHOD1(setInstrument,
      void(Geometry::Instrument_const_sptr inst));
    MOCK_CONST_METHOD0(getDetectorID,
      int());
    MOCK_METHOD1(setDetectorID,
      void(int m_DetectorID));
    MOCK_CONST_METHOD0(getDetector,
      Geometry::IDetector_const_sptr());
    MOCK_CONST_METHOD0(getInstrument,
      Geometry::Instrument_const_sptr());
    MOCK_CONST_METHOD0(getRunNumber,
      int());
    MOCK_METHOD1(setRunNumber,
      void(int m_RunNumber));
    MOCK_CONST_METHOD0(getMonitorCount,
      double());
    MOCK_METHOD1(setMonitorCount,
      void(double m_MonitorCount));
    MOCK_CONST_METHOD0(getH,
      double());
    MOCK_CONST_METHOD0(getK,
      double());
    MOCK_CONST_METHOD0(getL,
      double());
    MOCK_CONST_METHOD0(getHKL,
      Mantid::Kernel::V3D());
    MOCK_METHOD1(setH,
      void(double m_H));
    MOCK_METHOD1(setK,
      void(double m_K));
    MOCK_METHOD1(setL,
      void(double m_L));
    MOCK_METHOD3(setHKL,
      void(double H, double K, double L));
    MOCK_METHOD1(setHKL,
      void(Mantid::Kernel::V3D HKL));
    MOCK_CONST_METHOD0(getQLabFrame,
      Mantid::Kernel::V3D());
    MOCK_CONST_METHOD0(getQSampleFrame,
      Mantid::Kernel::V3D());
    MOCK_METHOD0(findDetector,
      bool());
    MOCK_METHOD2(setQSampleFrame,
      void(Mantid::Kernel::V3D QSampleFrame, double detectorDistance));
    MOCK_METHOD2(setQLabFrame,
      void(Mantid::Kernel::V3D QLabFrame, double detectorDistance));
    MOCK_METHOD1(setWavelength,
      void(double wavelength));
    MOCK_CONST_METHOD0(getWavelength,
      double());
    MOCK_CONST_METHOD0(getScattering,
      double());
    MOCK_CONST_METHOD0(getDSpacing,
      double());
    MOCK_CONST_METHOD0(getTOF,
      double());
    MOCK_CONST_METHOD0(getInitialEnergy,
      double());
    MOCK_CONST_METHOD0(getFinalEnergy,
      double());
    MOCK_METHOD1(setInitialEnergy,
      void(double m_InitialEnergy));
    MOCK_METHOD1(setFinalEnergy,
      void(double m_FinalEnergy));
    MOCK_CONST_METHOD0(getIntensity,
      double());
    MOCK_CONST_METHOD0(getSigmaIntensity,
      double());
    MOCK_METHOD1(setIntensity,
      void(double m_Intensity));
    MOCK_METHOD1(setSigmaIntensity,
      void(double m_SigmaIntensity));
    MOCK_CONST_METHOD0(getBinCount,
      double());
    MOCK_METHOD1(setBinCount,
      void(double m_BinCount));
    MOCK_CONST_METHOD0(getGoniometerMatrix,
      Mantid::Kernel::Matrix<double>());
    MOCK_METHOD1(setGoniometerMatrix,
      void(Mantid::Kernel::Matrix<double> m_GoniometerMatrix));
    MOCK_CONST_METHOD0(getBankName,
      std::string());
    MOCK_CONST_METHOD0(getRow,
      int());
    MOCK_CONST_METHOD0(getCol,
      int());
    MOCK_CONST_METHOD0(getDetPos,
      Mantid::Kernel::V3D());
    MOCK_CONST_METHOD0(getL1,
      double());
    MOCK_CONST_METHOD0(getL2,
      double());
  };

public:

  void test_throws_with_unknown_xLabel()
  {
    TS_ASSERT_THROWS(PeakTransformHKL("?", "K (Lattice)"), PeakTransformException);
  }

  void test_throws_with_unknown_yLabel()
  {
    TS_ASSERT_THROWS(PeakTransformHKL("H (Lattice)", "?"), PeakTransformException);
  }

  void test_maps_to_hkl_on_ipeak()
  {
    // Create a peak.
    MockIPeak mockPeak;
    EXPECT_CALL(mockPeak, getHKL()).WillOnce(Return(V3D())); // Should RUN getHKL!

    // Use the transform on the peak.
    PeakTransformHKL transform("H", "K");
    transform.transformPeak(mockPeak);

    // Check that the transform read the right coordinates off the peak object.
    TS_ASSERT(Mock::VerifyAndClearExpectations(&mockPeak));
  }

  void test_default_transform()
  {
    PeakTransformHKL transform; // Should be equivalent to constructing transform("H (Lattice)", "K (Lattice)")
    V3D original(0, 1, 2);
    V3D transformed = transform.transform(original);
    TS_ASSERT_EQUALS(transformed.X(), original.X());
    TS_ASSERT_EQUALS(transformed.Y(), original.Y());
    TS_ASSERT_EQUALS(transformed.Z(), original.Z());

    boost::regex_match("L (Lattice)", transform.getFreePeakAxisRegex());
  }

void test_transformHKL()
{
  PeakTransformHKL transform("H (Lattice)", "K (Lattice)");
  V3D original(0, 1, 2);
  V3D transformed = transform.transform(original);
  TS_ASSERT_EQUALS(transformed.X(), original.X());
  TS_ASSERT_EQUALS(transformed.Y(), original.Y());
  TS_ASSERT_EQUALS(transformed.Z(), original.Z());

  boost::regex_match("L (Lattice)", transform.getFreePeakAxisRegex());
}

void test_transformHLK()
{
  PeakTransformHKL transform("H (Lattice)", "L (Lattice)");
  V3D original(0, 1, 2);
  V3D transformed = transform.transform(original);
  TS_ASSERT_EQUALS(transformed.X(), original.X()); // X -> H
  TS_ASSERT_EQUALS(transformed.Y(), original.Z()); // Y -> L
  TS_ASSERT_EQUALS(transformed.Z(), original.Y()); // Z -> K

  boost::regex_match("K (Lattice)", transform.getFreePeakAxisRegex());
}

void test_transformLKH()
{
  PeakTransformHKL transform("L (Lattice)", "K (Lattice)");
  V3D original(0, 1, 2);
  V3D transformed = transform.transform(original);
  TS_ASSERT_EQUALS(transformed.X(), original.Z()); // X -> L
  TS_ASSERT_EQUALS(transformed.Y(), original.Y()); // Y -> K
  TS_ASSERT_EQUALS(transformed.Z(), original.X()); // Z -> H

  boost::regex_match("H (Lattice)", transform.getFreePeakAxisRegex());
}

void test_transformLHK()
{
  PeakTransformHKL transform("L (Lattice)", "H (Lattice)");
  V3D original(0, 1, 2);
  V3D transformed = transform.transform(original);
  TS_ASSERT_EQUALS(transformed.X(), original.Z()); // X -> L
  TS_ASSERT_EQUALS(transformed.Y(), original.X()); // Y -> H
  TS_ASSERT_EQUALS(transformed.Z(), original.Y()); // Z -> K

  boost::regex_match("K (Lattice)", transform.getFreePeakAxisRegex());
}

void test_transformKLH()
{
  PeakTransformHKL transform("K (Lattice)", "L (Lattice)");
  V3D original(0, 1, 2);
  V3D transformed = transform.transform(original);
  TS_ASSERT_EQUALS(transformed.X(), original.Y()); // X -> K
  TS_ASSERT_EQUALS(transformed.Y(), original.Z()); // Y -> L
  TS_ASSERT_EQUALS(transformed.Z(), original.X()); // Z -> H

  boost::regex_match("H (Lattice)", transform.getFreePeakAxisRegex());
}

void test_transformKHL()
{
  PeakTransformHKL transform("K (Lattice)", "H (Lattice)");
  V3D original(0, 1, 2);
  V3D transformed = transform.transform(original);
  TS_ASSERT_EQUALS(transformed.X(), original.Y()); // X -> K
  TS_ASSERT_EQUALS(transformed.Y(), original.X()); // Y -> H
  TS_ASSERT_EQUALS(transformed.Z(), original.Z()); // Z -> L

  boost::regex_match("L (Lattice)", transform.getFreePeakAxisRegex());
}

void test_copy_construction()
{
  PeakTransformHKL A("H", "L");
  PeakTransformHKL B(A);

  // Test indirectly via what the transformations produce.
  V3D productA = A.transform(V3D(0, 1, 2));
  V3D productB = B.transform(V3D(0, 1, 2));
  TS_ASSERT_EQUALS(productA, productB);  
  // Test indirectly via the free regex.
  boost::regex regexA = A.getFreePeakAxisRegex();
  boost::regex regexB = B.getFreePeakAxisRegex();
  TS_ASSERT_EQUALS(regexA, regexB);
 
}

void test_assigment()
{
  PeakTransformHKL A("H", "L");
  PeakTransformHKL B("K", "H");
  A = B;

  // Test indirectly via what the transformations produce.
  V3D productA = A.transform(V3D(0, 1, 2));
  V3D productB = B.transform(V3D(0, 1, 2));
  TS_ASSERT_EQUALS(productA, productB);  
  // Test indirectly via the free regex.
  boost::regex regexA = A.getFreePeakAxisRegex();
  boost::regex regexB = B.getFreePeakAxisRegex();
  TS_ASSERT_EQUALS(regexA, regexB);
}

void test_clone()
{
  PeakTransformHKL A("H", "L");
  PeakTransform_sptr clone = A.clone();

  TSM_ASSERT("Clone product is the wrong type.", boost::dynamic_pointer_cast<PeakTransformHKL>(clone) != NULL);

  // Test indirectly via what the transformations produce.
  V3D productA = A.transform(V3D(0, 1, 2));
  V3D productB = clone->transform(V3D(0, 1, 2));
  TS_ASSERT_EQUALS(productA, productB);  
  // Test indirectly via the free regex.
  boost::regex regexA = A.getFreePeakAxisRegex();
  boost::regex regexB = clone->getFreePeakAxisRegex();
  TS_ASSERT_EQUALS(regexA, regexB);
}

// Test the factory generated about this type.
void test_factory()
{
  // Create the benchmark.
  PeakTransform_sptr expectedProduct = boost::make_shared<PeakTransformHKL>("H", "K");

  // Use the factory to create a product.
  PeakTransformHKLFactory factory;
  PeakTransform_sptr product = factory.createDefaultTransform();

  // Check the type of the output product object.
  TSM_ASSERT("Factory product is the wrong type.", boost::dynamic_pointer_cast<PeakTransformHKL>(product) != NULL);

  // Now test that the benchmark and the factory product are equivalent.
  // Test indirectly via what the transformations produce.
  V3D productA = expectedProduct->transform(V3D(0, 1, 2));
  V3D productB = product->transform(V3D(0, 1, 2));
  TS_ASSERT_EQUALS(productA, productB);  
  // Test indirectly via the free regex.
  boost::regex regexA = expectedProduct->getFreePeakAxisRegex();
  boost::regex regexB = product->getFreePeakAxisRegex();
  TS_ASSERT_EQUALS(regexA, regexB);
}

private:
};
#endif

//end SLICE_VIEWER_PEAKTRANSFORMHKL_TEST_H_
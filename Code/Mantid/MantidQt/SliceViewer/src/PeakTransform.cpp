#include "MantidQtSliceViewer/PeakTransform.h"

namespace MantidQt
{
  namespace SliceViewer
  {
   PeakTransform::PeakTransform(const std::string& xPlotLabel, const std::string& yPlotLabel, const boost::regex& regexOne, const boost::regex& regexTwo, const boost::regex& regexThree) : 
    m_xPlotLabel(xPlotLabel),
    m_yPlotLabel(yPlotLabel),
    m_indexOfPlotX(0),
    m_indexOfPlotY(1),
    m_indexOfPlotZ(2),
    m_FirstRegex(regexOne),
    m_SecondRegex(regexTwo),
    m_ThirdRegex(regexThree)
  {
    const std::string& xLabel = m_xPlotLabel;
    const std::string& yLabel = m_yPlotLabel;

    const int FirstIndex = 0; // Maybe more appropriate to call this IndexZero etc.
    const int SecondIndex = 1;
    const int ThirdIndex = 2;

    Mantid::Kernel::V3D positionInCoordinateSystem;
    if(boost::regex_match(xLabel, m_FirstRegex) && boost::regex_match(yLabel, m_SecondRegex)) //HKL
    {
      m_indexOfPlotX = FirstIndex;
      m_indexOfPlotY = SecondIndex; 
      m_indexOfPlotZ = ThirdIndex; 
    }
    else if(boost::regex_match(xLabel, m_FirstRegex) && boost::regex_match(yLabel, m_ThirdRegex)) //HLK
    {
      m_indexOfPlotX = FirstIndex;
      m_indexOfPlotY = ThirdIndex; 
      m_indexOfPlotZ = SecondIndex; 
    }
    else if(boost::regex_match(xLabel, m_ThirdRegex) && boost::regex_match(yLabel, m_FirstRegex)) //LHK
    {
      m_indexOfPlotX = ThirdIndex;
      m_indexOfPlotY = FirstIndex; 
      m_indexOfPlotZ = SecondIndex; 
    }
    else if(boost::regex_match(xLabel, m_ThirdRegex) && boost::regex_match(yLabel, m_SecondRegex)) //LKH
    {
      m_indexOfPlotX = ThirdIndex;
      m_indexOfPlotY = SecondIndex; 
      m_indexOfPlotZ = FirstIndex; 
    }
    else if(boost::regex_match(xLabel, m_SecondRegex) && boost::regex_match(yLabel, m_ThirdRegex)) //KLH
    {
      m_indexOfPlotX = SecondIndex;
      m_indexOfPlotY = ThirdIndex; 
      m_indexOfPlotZ = FirstIndex; 
    }
    else if(boost::regex_match(xLabel, m_SecondRegex) && boost::regex_match(yLabel, m_FirstRegex)) //KHL
    {
      m_indexOfPlotX = SecondIndex;
      m_indexOfPlotY = FirstIndex; 
      m_indexOfPlotZ = ThirdIndex; 
    }
    else
    {
      throw PeakTransformException();
    }
  }

  PeakTransform::~PeakTransform()
  {
  }

  PeakTransform::PeakTransform(const PeakTransform& other):
  m_xPlotLabel(other.m_xPlotLabel),
    m_yPlotLabel(other.m_yPlotLabel),
    m_indexOfPlotX(other.m_indexOfPlotX),
    m_indexOfPlotY(other.m_indexOfPlotY),
    m_indexOfPlotZ(other.m_indexOfPlotZ),
    m_FirstRegex(other.m_FirstRegex),
    m_SecondRegex(other.m_SecondRegex),
    m_ThirdRegex(other.m_ThirdRegex)
  {
  }


  boost::regex PeakTransform::getFreePeakAxisRegex() const
  {
    switch(m_indexOfPlotZ)
    {
    case 0:
      return m_FirstRegex;
    case 1:
      return m_SecondRegex;
    default:
      return m_ThirdRegex;
    }
  }

  Mantid::Kernel::V3D PeakTransform::transform(const Mantid::Kernel::V3D& original) const
  {
    // Will have the plots x, y, and z aligned to the correct h, k, l value.
    Mantid::Kernel::V3D transformedPeakPosition;
    transformedPeakPosition.setX(original[m_indexOfPlotX]);
    transformedPeakPosition.setY(original[m_indexOfPlotY]);
    transformedPeakPosition.setZ(original[m_indexOfPlotZ]);
    return transformedPeakPosition;
  }

}
}
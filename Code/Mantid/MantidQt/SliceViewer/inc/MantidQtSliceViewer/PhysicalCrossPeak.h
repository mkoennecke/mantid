#ifndef MANTID_SLICEVIEWER_PHYSICALCROSSPEAK_H_
#define MANTID_SLICEVIEWER_PHYSICALCROSSPEAK_H_

#include "MantidKernel/System.h"
#include "MantidKernel/V3D.h"
#include "MantidKernel/ClassMacros.h"
#include "MantidQtSliceViewer/PeakTransform.h"

namespace MantidQt
{
  namespace SliceViewer
  {
    /**
    @class Cross peak drawing primitive information.
    */
    struct CrossPeakPrimitives 
    {
      int peakHalfCrossWidth;
      int peakHalfCrossHeight;
      int peakLineWidth;
      double peakOpacityAtDistance;
      Mantid::Kernel::V3D peakOrigin;
    };

    /**
    @class PhysicalCrossPeak
    Represents the spacial and physical aspects of a cross peak. Used to handle all physical interactions with other spacial objects.
    */
    class DLLExport PhysicalCrossPeak 
    {
    public:
      /// Constructor
      PhysicalCrossPeak(const Mantid::Kernel::V3D& origin, const double& maxZ, const double& minZ);
      /// Destructor
      ~PhysicalCrossPeak();
      /// Setter for the slice point.
      void setSlicePoint(const double&);
      /// Transform the coordinates.
      void movePosition(PeakTransform_sptr peakTransform);
      /// Draw
      CrossPeakPrimitives draw(const double& windowHeight, const double& windowWidth) const;
      /// Determine wheter the peak is viewable given the current slice position
      inline bool isViewable() const
      {
        return (m_opacityAtDistance != m_opacityMin);
      }
    private:
      /// Original origin x=h, y=k, z=l
      const Mantid::Kernel::V3D m_originalOrigin;
      /// Origin md-x, md-y, and md-z
      Mantid::Kernel::V3D m_origin;
      /// effective peak radius
      const double m_effectiveRadius;
      /// Max opacity
      const double m_opacityMax;
      /// Min opacity
      const double m_opacityMin;
      /// Cached opacity gradient
      const double m_opacityGradient;
      /// Cross size percentage in y a fraction of the current screen height.
      const double m_crossViewFraction;
      /// Cached opacity at the distance z from origin
      double m_opacityAtDistance;

      DISABLE_COPY_AND_ASSIGN(PhysicalCrossPeak)
    };

  }
}

#endif /* MANTID_SLICEVIEWER_PHYSICALCROSSPEAK_H_ */
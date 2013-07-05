#ifndef MANTID_MDEVENTS_CALCULATE_REFLECTOMETRYQ_BASE_H_
#define MANTID_MDEVENTS_CALCULATE_REFLECTOMETRYQ_BASE_H_

namespace Mantid
{
  namespace Geometry
  {
    namespace MDGeometry
    {
      class IMDDimension;
    }
  }

  namespace MDEvents
  {
    /**
    Base class for reflectometry Q transformations
    */
    class CalculateReflectometryQBase
    {
    protected:
      const double to_radians_factor;
      CalculateReflectometryQBase();

    protected:
      ~CalculateReflectometryQBase();
    };
  }
}

#endif

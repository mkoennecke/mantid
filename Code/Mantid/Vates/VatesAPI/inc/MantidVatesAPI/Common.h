#ifndef MANTID_VATES_COMMON_H_
#define MANTID_VATES_COMMON_H_
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

class vtkFieldData;

namespace Mantid
{
namespace Geometry
{
//Forward dec
class IMDDimension;
}

namespace VATES
{
/// Vector of IMDDimension shared pointers.
typedef std::vector<boost::shared_ptr<Mantid::Geometry::IMDDimension> > DimensionVec;

/// IMDDimension as shared pointer.
typedef boost::shared_ptr<Mantid::Geometry::IMDDimension> Dimension_sptr;

/// IMDDimension as const shared pointer. Note that IMDDimension is pure virtual.
typedef boost::shared_ptr<const Mantid::Geometry::IMDDimension> Dimension_const_sptr;

/// Flags what should be done on the current iteration.
enum RebinningIterationAction {
  UseCache, //There is no delta here. Use a cached vtkDataSet.
  RecalculateVisualDataSetOnly, // 4D data set has not altered so create a new visual 3D slice only.
  RecalculateAll, // Rebin and create 3D visualisation slice from 4D dataset.
  ReloadAndRecalculateAll // Reload the original workspace and then Rebin it.
};

std::string makeAxisTitle(Dimension_const_sptr dim);

void setAxisLabel(std::string metadataLabel,
                  std::string labelString,
                  vtkFieldData *fieldData);
/// Function to check for special values.
bool isSpecial(double value);

}

}

#endif

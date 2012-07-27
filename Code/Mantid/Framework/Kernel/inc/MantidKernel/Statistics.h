#ifndef MANTID_KERNEL_STATISTICS_H_
#define MANTID_KERNEL_STATISTICS_H_

#include "MantidKernel/DllConfig.h"
#include <vector>

namespace Mantid
{
  namespace Kernel
  {
    namespace Math
    {
      /**
       * Maps a "statistic" to a number
       */
      enum StatisticType { FirstValue, LastValue, Minimum, Maximum, Mean, TimeAveragedMean, Median };
    }

    /**
       Simple struct to store statistics.
       
       Copyright &copy; 2010-2012 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

       This file is part of Mantid.

       Mantid is free software; you can redistribute it and/or modify
       it under the terms of the GNU General Public License as published by
       the Free Software Foundation; either version 3 of the License, or
       (at your option) any later version.

       Mantid is distributed in the hope that it will be useful,
       but WITHOUT ANY WARRANTY; without even the implied warranty of
       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
       GNU General Public License for more details.

       You should have received a copy of the GNU General Public License
       along with this program.  If not, see <http://www.gnu.org/licenses/>.

       File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>.
       Code Documentation is available at: <http://doxygen.mantidproject.org>
     */
    struct Statistics
    {
      /// Minimum value
      double minimum;
      /// Maximum value
      double maximum;
      /// Mean value
      double mean;
      /// Median value
      double median;
      /// standard_deviation of the values
      double standard_deviation;
    };
  
    /// Return a statistics object for the given data set
    template<typename TYPE>
    Statistics getStatistics(const std::vector<TYPE>& data, const bool sorted=false);
    /// Return the Z score values for a dataset
    template<typename TYPE>
    std::vector<double> getZscore(const std::vector<TYPE>& data, const bool sorted=false);
    /// Return the modified Z score values for a dataset
    template<typename TYPE>
    std::vector<double> getModifiedZscore(const std::vector<TYPE>& data, const bool sorted=false);
  
  } // namespace Kernel
} // namespace Mantid
#endif /* STATISTICS_H_ */

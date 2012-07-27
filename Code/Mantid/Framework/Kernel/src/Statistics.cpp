// Includes
#include "MantidKernel/Statistics.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <cmath>
#include <numeric>
#include <string>

namespace Mantid
{
  namespace Kernel
  {

    using std::string;
    using std::vector;

    /**
     * Generate a Statistics object where all of the values are NaN. This is a good initial default.
     */
    Statistics getNanStatistics()
    {
      double nan = std::numeric_limits<double>::quiet_NaN();

      Statistics stats;
      stats.minimum = nan;
      stats.maximum = nan;
      stats.mean = nan;
      stats.median = nan;
      stats.standard_deviation = nan;

      return stats;
    }

    /**
     * There are enough special cases in determining the median where it useful to
     * put it in a single function.
     */
    template<typename TYPE>
    double getMedian(const vector<TYPE>& data, const size_t num_data, const bool sorted)
    {
      double left = 0.0;
      double right = 0.0;

      if (num_data == 1)
        return static_cast<double> (*(data.begin()));

      bool is_even = ((num_data % 2) == 0);
      if (is_even)
      {
        if (sorted)
        {
          // Just get the centre two elements.
          left = static_cast<double> (*(data.begin() + num_data / 2 - 1));
          right = static_cast<double> (*(data.begin() + num_data / 2));
        }
        else
        {
          // If the data is not sorted, make a copy we can mess with
          vector<TYPE> temp(data.begin(), data.end());
          // Get what the centre two elements should be...
          std::nth_element(temp.begin(), temp.begin() + num_data / 2 - 1, temp.end());
          left = static_cast<double> (*(temp.begin() + num_data / 2 - 1));
          std::nth_element(temp.begin(), temp.begin() + num_data / 2, temp.end());
          right = static_cast<double> (*(temp.begin() + num_data / 2));
        }
        // return the average
        return (left + right) / 2.;
      }
      else
      // Odd number
      {
        if (sorted)
        {
          // If sorted and odd, just return the centre value
          return static_cast<double> (*(data.begin() + num_data / 2));
        }
        else
        {
          // If the data is not sorted, make a copy we can mess with
          vector<TYPE> temp(data.begin(), data.end());
          // Make sure the centre value is in the correct position
          std::nth_element(temp.begin(), temp.begin() + num_data / 2, temp.end());
          // Now return the centre value
          return static_cast<double> (*(temp.begin() + num_data / 2));
        }
      }
    }
    /**
     * There are enough special cases in determining the Z score where it useful to
     * put it in a single function.
     */
    template<typename TYPE>
    std::vector<double> getZscore(const vector<TYPE>& data, const bool sorted)
    {
      if (data.size() < 3)
      {
    	  std::vector<double>Zscore(data.size(),0.);
    	  return Zscore;
      }
      std::vector<double> Zscore;
      double tmp;
      Statistics stats = getStatistics(data, sorted);
      if(stats.standard_deviation == 0.)
      {
    	  std::vector<double>Zscore(data.size(),0.);
    	  return Zscore;
      }
      typename vector<TYPE>::const_iterator it = data.begin();
      for (; it != data.end(); ++it)
      {
    	tmp = static_cast<double> (*it);
        Zscore.push_back(fabs((tmp - stats.mean) / stats.standard_deviation));
      }
      return Zscore;
    }
    /**
     * There are enough special cases in determining the modified Z score where it useful to
     * put it in a single function.
     */
    template<typename TYPE>
    std::vector<double> getModifiedZscore(const vector<TYPE>& data, const bool sorted)
    {
      if (data.size() < 3)
      {
    	  std::vector<double>Zscore(data.size(),0.);
    	  return Zscore;
      }
      std::vector<double>MADvec;
      double tmp;
      size_t num_data = data.size(); // cache since it is frequently used
      double median = getMedian(data, num_data, sorted);
      typename vector<TYPE>::const_iterator it = data.begin();
      for (; it != data.end(); ++it)
      {
    	tmp = static_cast<double> (*it);
        MADvec.push_back(fabs(tmp - median));
      }
      double MAD = getMedian(MADvec, num_data, sorted);
      if(MAD == 0.)
      {
    	  std::vector<double>Zscore(data.size(),0.);
    	  return Zscore;
      }
      MADvec.empty();
      std::vector<double> Zscore;
      it = data.begin();
      for (; it != data.end(); ++it)
      {
    	tmp = static_cast<double> (*it);
        Zscore.push_back(0.6745*fabs((tmp - median) / MAD));
      }
      return Zscore;
    }

    /**
     * Determine the statistics for a vector of data. If it is sorted then let the
     * function know so it won't make a copy of the data for determining the median.
     */
    template<typename TYPE>
    Statistics getStatistics(const vector<TYPE>& data, const bool sorted)
    {
      Statistics stats = getNanStatistics();
      size_t num_data = data.size(); // chache since it is frequently used

      if (num_data == 0)
      { // don't do anything
        return stats;
      }

      // calculate the mean
      stats.mean = std::accumulate(data.begin(), data.end(), 0., std::plus<double>());
      stats.mean /= (static_cast<double> (num_data));

      // calculate the standard deviation, min, max
      stats.minimum = stats.mean;
      stats.maximum = stats.mean;
      double stddev = 0.;
      double temp;
      typename vector<TYPE>::const_iterator it = data.begin();
      for (; it != data.end(); ++it)
      {
        temp = static_cast<double> (*it);
        stddev += ((temp - stats.mean) * (temp - stats.mean));
        if (temp > stats.maximum)
          stats.maximum = temp;
        if (temp < stats.minimum)
          stats.minimum = temp;
      }
      stats.standard_deviation = sqrt(stddev / (static_cast<double> (num_data)));

      // calculate the median
      stats.median = getMedian(data, num_data, sorted);

      return stats;
    }

    /// Getting statistics of a string array should just give a bunch of NaNs
    template<>
    DLLExport Statistics getStatistics<string> (const vector<string>& data, const bool sorted)
    {
      UNUSED_ARG(sorted);
      UNUSED_ARG(data);
      return getNanStatistics();
    }

    /// Getting statistics of a boolean array should just give a bunch of NaNs
    template<>
    DLLExport Statistics getStatistics<bool> (const vector<bool>& data, const bool sorted)
    {
      UNUSED_ARG(sorted);
      UNUSED_ARG(data);
      return getNanStatistics();
    }

    // -------------------------- Macro to instantiation concrete types --------------------------------
#define INSTANTIATE(TYPE) \
    template MANTID_KERNEL_DLL Statistics getStatistics<TYPE> (const vector<TYPE> &, const bool); \
    template MANTID_KERNEL_DLL std::vector<double> getZscore<TYPE> (const vector<TYPE> &, const bool); \
    template MANTID_KERNEL_DLL std::vector<double> getModifiedZscore<TYPE> (const vector<TYPE> &, const bool);

    // --------------------------- Concrete instantiations ---------------------------------------------
    INSTANTIATE(float);
    INSTANTIATE(double);
    INSTANTIATE(int);
    INSTANTIATE(long);
    INSTANTIATE(long long);
    INSTANTIATE(unsigned int);
    INSTANTIATE(unsigned long);
    INSTANTIATE(unsigned long long);

  } // namespace Kernel
} // namespace Mantid

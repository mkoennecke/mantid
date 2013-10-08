#ifndef MANTID_ALGORITHMS_RADIUSSUM_H_
#define MANTID_ALGORITHMS_RADIUSSUM_H_

#include "MantidKernel/System.h"
#include "MantidAPI/Algorithm.h"

namespace Mantid
{
namespace Algorithms
{


  /** RadiusSum : 

    Sum of all the counts inside a ring against the scattering angle for each Radius.
    
    @author Gesner Passos, ISIS

    Copyright &copy; 2013 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

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

    File change history is stored at: <https://github.com/mantidproject/mantid>
    Code Documentation is available at: <http://doxygen.mantidproject.org>
  */
  class DLLExport RadiusSum  : public API::Algorithm
  {
  public:
    RadiusSum();
    virtual ~RadiusSum();
    
    virtual const std::string name() const;
    virtual int version() const;
    virtual const std::string category() const;

  private:
    virtual void initDocs();
    void init();
    void exec();

    void cacheInputPropertyValues();


    double centre_x, centre_y, centre_z; 
    int num_bins; 
    bool normalize_flag; 
    double normalization_order;
    API::MatrixWorkspace_sptr inputWS;
    double min_radius, max_radius;     
  };


} // namespace Algorithms
} // namespace Mantid

#endif  /* MANTID_ALGORITHMS_RADIUSSUM_H_ */

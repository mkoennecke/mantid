#ifndef MANTID_ALGORITHMS_UNWRAP_H_
#define MANTID_ALGORITHMS_UNWRAP_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAlgorithms/UnwrapMonitor.h"
#include "MantidAPI/DeprecatedAlgorithm.h"

namespace Mantid
{
namespace Algorithms
{
/** Takes an input Workspace2D that contains 'raw' data, unwraps the data according to
    the reference flightpath provided and converts the units to wavelength.
    The output workspace will have common bins in the maximum theoretical wavelength range.

    Required Properties:
    <UL>
    <LI> InputWorkspace  - The name of the input workspace. </LI>
    <LI> OutputWorkspace - The name of the output workspace. </LI>
    <LI> LRef            - The 'reference' flightpath (in metres). </LI>
    </UL>

    @author Russell Taylor, Tessella Support Services plc
    @date 25/07/2008

    Copyright &copy; 2008-9 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

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
class DLLExport Unwrap : public UnwrapMonitor, public API::DeprecatedAlgorithm
{
public:
  Unwrap();

  virtual ~Unwrap();

  /// Algorithm's name for identification overriding a virtual method
  virtual const std::string name() const;

  /// Algorithm's version for identification overriding a virtual method
  virtual int version() const;
  /// category
  virtual const std::string category() const { return "Deprecated"; }
};

} // namespace Algorithm
} // namespace Mantid

#endif /* MANTID_ALGORITHMS_UNWRAP_H_ */

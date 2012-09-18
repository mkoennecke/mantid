#ifndef MANTID_CURVEFITTING_TABULATEDFUNCTION_H_
#define MANTID_CURVEFITTING_TABULATEDFUNCTION_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAPI/ParamFunction.h"
#include "MantidAPI/IFunction1D.h"
#include "MantidKernel/System.h"
#include <cmath>

namespace Mantid
{

//----------------------------------------------------------------------
// Forward declaration
//----------------------------------------------------------------------
namespace API
{
  class MatrixWorkspace;
}

namespace CurveFitting
{
/**

A function which takes its values from a file or a workspace. The values atr tabulated as
x,y pairs. Liear interpolation is used for points between the tabulated values. The function
returns zero for points outside the tabulated values.

The function has two attributes: FileName and Workspace. They define a data set to take the values from.
Setting one of the attributes clears the other. 

The files can be either ascii text files or nexus files. The ascii files must contain two column
of real numbers separated by spaces. The first column are the x-values and the second one is for y.

If a nexus file is used its first spectrum provides the data for the function. The same is true for 
a workspace which must be a MatrixWorkspace.

The function has a signle parameter - a scaling factor "Scaling". 

@author Roman Tolchenov, Tessella plc
@date 4/09/2012

Copyright &copy; 2007-8 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

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

File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>
Code Documentation is available at: <http://doxygen.mantidproject.org>
*/
class DLLExport TabulatedFunction : public API::ParamFunction, public API::IFunction1D
{
public:
  /// Constructor
  TabulatedFunction();

  /// overwrite IFunction base class methods
  std::string name()const{return "TabulatedFunction";}
  virtual const std::string category() const { return "General";}
  void function1D(double* out, const double* xValues, const size_t nData)const;
  ///  function derivatives
  void functionDeriv1D(API::Jacobian* out, const double* xValues, const size_t nData);

  /// Returns the number of attributes associated with the function
  size_t nAttributes()const{return 2;}
  /// Returns a list of attribute names
  std::vector<std::string> getAttributeNames()const;
  /// Return a value of attribute attName
  IFunction::Attribute getAttribute(const std::string& attName)const;
  /// Set a value to attribute attName
  void setAttribute(const std::string& attName,const IFunction::Attribute& value);
  /// Check if attribute attName exists
  bool hasAttribute(const std::string& attName)const;

private:

  /// Call the appropriate load function
  void load(const std::string& fname);

  /// Load the points from an ASCII file
  void loadAscii(const std::string& fname);

  /// Load the points from a NeXuS file
  void loadNexus(const std::string& fname);

  /// Load the points from a MatrixWorkspace
  void loadWorkspace(const std::string& wsName);

  /// Load the points from a MatrixWorkspace
  void loadWorkspace(boost::shared_ptr<API::MatrixWorkspace> ws);

  /// Size of the data
  size_t size()const{return m_yData.size();}

  /// Clear all data
  void clear();

  /// Evaluate the function for a list of arguments and given scaling factor
  void eval(double scaling, double* out, const double* xValues, const size_t nData)const;

  /// The file name
  std::string m_fileName;

  /// The workspace name
  std::string m_wsName;

  /// Stores x-values
  std::vector<double> m_xData;

  /// Stores y-values
  std::vector<double> m_yData;

  /// The first x
  double m_xStart;

  /// The lasst x
  double m_xEnd;

};

} // namespace CurveFitting
} // namespace Mantid

#endif /*MANTID_CURVEFITTING_TABULATEDFUNCTION_H_*/
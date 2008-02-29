#ifndef MANTIDAPI_HISTDATAVALUE_H
#define MANTIDAPI_HISTDATAVALUE_H

#include "MantidKernel/System.h"
#include "MantidAPI/IHistData.h"
#include "MantidAPI/PointDataValue.h"

namespace Mantid
{

  namespace API
  {

    //forward declaration
    class IErrorHelper;

    /**
    IDataItem of an X value, two error values E and E2 together with a pointer to an ErrorHelper and a specta number.
    Class maintians a type first/second/third triple
    similar to std::pair except all are identical

    \author N. Draper

    Copyright &copy; 2007-8 STFC Rutherford Appleton Laboratories

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
    */
    class DLLExport HistDataValue : public IHistData
    {
    public:

      const double& X() const;
      const double& E() const;
      const double& E2() const;
      const IErrorHelper* ErrorHelper() const;
      int SpectraNo() const;

      double& X();
      double& E();
      double& E2();

      const double& Y() const;
      double& Y();

      double xValue;        ///< value of X
      double yValue;        ///< value of Y
      double eValue;        ///< value of E
      double e2Value;       ///< value of E2
      int spectraNo;
      const IErrorHelper* errorHelper;

      const double& X2() const;
      double& X2();

      double x2Value;        ///< value of X2

      HistDataValue();
      HistDataValue(const HistDataValue&);
      HistDataValue(const IHistData&);
      HistDataValue& operator=(const HistDataValue&);
      HistDataValue& operator=(const IHistData&);
      HistDataValue& operator=(const IPointData&);
      virtual ~HistDataValue();

      int operator<(const HistDataValue&) const;
      int operator>(const HistDataValue&) const;
      int operator==(const HistDataValue&) const;
      int operator!=(const HistDataValue&) const;
    };

  }  // NAMESPACE API

}  // NAMESPACE Mantid

#endif //MANTIDAPI_HISTDATAVALUE_H

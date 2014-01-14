#ifndef MANTID_ICAT_CMYDATASEARCH_H_
#define MANTID_ICAT_CMYDATASEARCH_H_

#include "MantidAPI/Algorithm.h"
#include "MantidAPI/ITableWorkspace.h"

namespace Mantid
{
  namespace ICat
  {

    /** CatalogMyDataSearch is a class responsible for searching investigations of the logged in user.
     * This algorithm does Icat search and returns the investigations record

    Required Properties:
    <UL>
    <LI>  OutputWorkspace - name of the OutputWorkspace which contains my investigations search
    <LI>  isValid         - Boolean option used to check the validity of login session
    </UL>

    @author Sofia Antony, ISIS Rutherford Appleton Laboratory 
    @date 04/08/2010
    Copyright &copy; 2010 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

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

    File change history is stored at: <https://github.com/mantidproject/mantid>.
    Code Documentation is available at: <http://doxygen.mantidproject.org>

     */
    class DLLExport CatalogMyDataSearch: public API::Algorithm
    {
    public:
      ///constructor
      CatalogMyDataSearch():API::Algorithm(){}
      ///destructor
      ~CatalogMyDataSearch() {}
      /// Algorithm's name for identification overriding a virtual method
      virtual const std::string name() const { return "CatalogMyDataSearch"; }
      /// Algorithm's version for identification overriding a virtual method
      virtual int version() const { return 1; }
      /// Algorithm's category for identification overriding a virtual method
      virtual const std::string category() const { return "DataHandling\\Catalog"; }

    private:
      /// Sets documentation strings for this algorithm
      virtual void initDocs();
      /// Overwrites Algorithm init method.
      void init();
      /// Overwrites Algorithm exec method
      void exec();
    };
  }
}

#endif

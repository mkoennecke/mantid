#ifndef MANTID_DATAHANDLING_SNSDATAARCHIVEICAT2_H_
#define MANTID_DATAHANDLING_SNSDATAARCHIVEICAT2_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidKernel/System.h"
#include "MantidAPI/IArchiveSearch.h"

#include <string>


namespace Mantid
{

// Forward declaration
namespace Kernel
{
class Logger;
}

  namespace DataHandling
  {
  /**
   This class is for searching the SNS data archive

   @author Shelly Ren, ORNL
   @date 02/22/2012

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

    class  DLLExport SNSDataArchiveICAT2: public API::IArchiveSearch
    {
    public:
        std::string getArchivePath(const std::set<std::string>& filenames, const std::vector<std::string>& exts) const;
    private:
        // static reference to the logger class
        std::string getPath(const std::string& fName) const;
        static Mantid::Kernel::Logger & g_log;
    };

  }
}

#endif /* MANTID_DATAHANDLING_SNSDATAARCHIVEICAT2_H_ */
#ifndef MANTID_SINQ_WAITCANCEL_H_
#define MANTID_SINQ_WAITCANCEL_H_

#include "MantidKernel/System.h"
#include "MantidAPI/Algorithm.h"

namespace Mantid
{
namespace SINQ
{

  /* WaitCancel : This is a mini algorithm which keeps running until cancelled.
      Other code, such as the SINQCCDLiveListener can start this as a asynchronous
      child algorithm and detect cancellation by checking if this algorithm still
      runs.

      Mark Koennecke, April 2014
    
    Copyright &copy; 2014 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

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
  class DLLExport WaitCancel  : public API::Algorithm
  {
  public:
    WaitCancel();
    virtual ~WaitCancel();
    
    virtual const std::string name() const;
    virtual int version() const;
    virtual const std::string category() const;
    virtual const std::string summary() const;

  private:
    void init();
    void exec();


  };


} // namespace SINQ
} // namespace Mantid

#endif  /* MANTID_SINQ_WAITCANCEL_H_ */

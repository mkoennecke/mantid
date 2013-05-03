#ifndef MANTID_ALGORITHMS_DIVIDESPLITTERS_H_
#define MANTID_ALGORITHMS_DIVIDESPLITTERS_H_

#include "MantidKernel/System.h"
#include "MantidAPI/Algorithm.h"
#include "MantidDataObjects/SplittersWorkspace.h"
#include "MantidDataObjects/TableWorkspace.h"

namespace Mantid
{
namespace Algorithms
{

  /** DivideSplitters : Divide the splitters in a splitters workspace to a new splitters workspace
    
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
  class DLLExport DivideSplitters : public API::Algorithm
  {
  public:
    DivideSplitters();
    virtual ~DivideSplitters();

    /// Algorithm's name for identification overriding a virtual method
    virtual const std::string name() const { return "DivideSplitters";}
    /// Algorithm's version for identification overriding a virtual method
    virtual int version() const { return 1;}
    /// Algorithm's category for identification overriding a virtual method
    virtual const std::string category() const { return "Events\\EventFiltering";}

  private:
    /// Sets documentation strings for this algorithm
    virtual void initDocs();
    /// Declare properties
    void init();
    /// Main execution body
    void exec();

    /// Process properties and validate and create output workspace
    void processAlgorithmProperties();

    /// Divide splitters by even time segment
    void divideSplitters(int wsindex, int numsegments);

    /// Input splitter workspace
    DataObjects::SplittersWorkspace_sptr m_inpWS;
    /// Input information table workspace
    DataObjects::TableWorkspace_sptr m_infoWS;
    /// Output splitter workspace
    DataObjects::SplittersWorkspace_sptr m_outWS;
    /// Output splitter information workspace
    DataObjects::TableWorkspace_sptr m_outInfoWS;

    /// Index of the workspace spectrum to divide
    int m_wsIndex;
    /// Number of the segments to divide to
    int m_numSegments;

  };


} // namespace Algorithms
} // namespace Mantid

#endif  /* MANTID_ALGORITHMS_DIVIDESPLITTERS_H_ */

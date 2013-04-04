#ifndef MANTID_DATAHANDLING_LOADLOGSFROMSNSNEXUS_H_
#define MANTID_DATAHANDLING_LOADLOGSFROMSNSNEXUS_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAPI/Algorithm.h"
#include "MantidNexus/NexusClasses.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/Sample.h"
#include "MantidDataObjects/Workspace2D.h"
#include <nexus/NeXusFile.hpp>
#include <nexus/NeXusException.hpp>
#include "MantidAPI/DeprecatedAlgorithm.h"

namespace Mantid
{

  namespace Geometry
  {
    class CompAssembly;
    class Component;
    class Instrument;
  }

  namespace DataHandling
  {
    /** @class LoadLogsFromSNSNexus LoadLogsFromSNSNexus.h DataHandling/LoadLogsFromSNSNexus.h

    Load sample logs (single values and time series data) from a SNS NeXus format file.
    This is meant to be used as a Child Algorithm to other algorithms.

    Required Properties:
    <UL>
    <LI> Filename - The name of and path to the input NeXus file </LI>
    <LI> Workspace - The name of the workspace in which to use as a basis for any data to be added.</LI>
    </UL>

    @author Janik Zikovsky, SNS
    @date Sep 17, 2010

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

    File change history is stored at: <https://github.com/mantidproject/mantid>
    */
    class DLLExport LoadLogsFromSNSNexus : public API::Algorithm, public API::DeprecatedAlgorithm
    {

    public:
      /// Default constructor
      LoadLogsFromSNSNexus();

      /// Destructor
      virtual ~LoadLogsFromSNSNexus() {}

      /// Algorithm's name for identification overriding a virtual method
      virtual const std::string name() const { return "LoadLogsFromSNSNexus";};

      /// Algorithm's version for identification overriding a virtual method
      virtual int version() const { return 1;};

      /// Algorithm's category for identification overriding a virtual method
      virtual const std::string category() const { return "Deprecated";}

    private:
      /// Sets documentation strings for this algorithm
      virtual void initDocs();

      void init();

      void exec();

      void loadSampleLog(::NeXus::File& file, std::string entry_name, std::string entry_class);

      /// The name and path of the input file
      std::string m_filename;

      /// The workspace being filled out
      API::MatrixWorkspace_sptr WS;

    };

  } // namespace DataHandling
} // namespace Mantid

#endif /*MANTID_DATAHANDLING_LOADLOGSFROMSNSNEXUS_H_*/


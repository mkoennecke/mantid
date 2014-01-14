#ifndef MANTID_DATAHANDLING_LOADIDFROMNEXUS_H_
#define MANTID_DATAHANDLING_LOADIDFFROMNEXUS_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAPI/Algorithm.h"

namespace Mantid
{
  namespace DataHandling
  {
    /** @class LoadIDFFromNexus LoadInstrumentFromNexus.h DataHandling/LoadIDFFromNexus.h

    Load an IDF from a Nexus file, if found there.

    LoadIDFFromNexus is intended to be used as a child algorithm of
    other Loadxxx algorithms, rather than being used directly.
    A such it enables the loadxxx algorithm to take the instrument definition from an IDF located within
    a Nexus file if it is available.
    LoadIDFFromNexus is an algorithm and as such inherits
    from the Algorithm class, via DataHandlingCommand, and overrides
    the init() & exec()  methods.

    Required Properties:
    <UL>
    <LI> Filename - The name of and path to the input NEXUS file </LI>
    <LI> Workspace - The name of the workspace in which to use as a basis for any data to be added.</LI>
    </UL>

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
    */
    class DLLExport LoadIDFFromNexus : public API::Algorithm
    {
    public:
      /// Default constructor
      LoadIDFFromNexus();

      /// Destructor
      virtual ~LoadIDFFromNexus() {}

      /// Algorithm's name for identification overriding a virtual method
      virtual const std::string name() const { return "LoadIDFFromNexus";}

      /// Algorithm's version for identification overriding a virtual method
      virtual int version() const { return 1;}

      /// Algorithm's category for identification overriding a virtual method
      virtual const std::string category() const { return "DataHandling\\Instrument";}

    private:
      /// Sets documentation strings for this algorithm
      virtual void initDocs();
      /// Overwrites Algorithm method. Does nothing at present
      void init();
      /// Overwrites Algorithm method
      void exec();

      void runLoadParameterFile(const API::MatrixWorkspace_sptr & workspace);
    };

  } // namespace DataHandling
} // namespace Mantid

#endif /*MANTID_DATAHANDLING_LOADIDFFROMNEXUS_H_*/


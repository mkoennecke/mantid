#ifndef MANTID_DATAHANDLING_LOADILL_H_
#define MANTID_DATAHANDLING_LOADILL_H_

//---------------------------------------------------
// Includes
//---------------------------------------------------
#include "MantidAPI/IFileLoader.h"
#include "MantidNexus/NexusClasses.h"
#include "MantidDataHandling/LoadHelper.h"

namespace Mantid {
namespace DataHandling {
/**
 Loads an ILL nexus file into a Mantid workspace.

 Required properties:
 <UL>
 <LI> Filename - The ILL nexus file to be read </LI>
 <LI> Workspace - The name to give to the output workspace </LI>
 </UL>

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
 Code Documentation is available at: <http://doxygen.mantidproject.org>
 */
  class DLLExport LoadILL: public API::IFileLoader<Kernel::NexusDescriptor> 
  {
  public:
    /// Constructor
	LoadILL();	/// Virtual destructor
	virtual ~LoadILL() {
	}
	/// Algorithm's name
	virtual const std::string name() const {
		return "LoadILL";
	}
	/// Algorithm's version
	virtual int version() const {
		return (1);
	}
	/// Algorithm's category for identification
	virtual const std::string category() const {
		return "DataHandling";
	}

	/// Returns a confidence value that this algorithm can load a file
        int confidence(Kernel::NexusDescriptor & descriptor) const;

private:
	/// Sets documentation strings for this algorithm
	virtual void initDocs();
	// Initialisation code
	void init();
	// Execution code
	void exec();

	void loadInstrumentDetails(NeXus::NXEntry&);
	void initWorkSpace(NeXus::NXEntry& entry);
	void initInstrumentSpecific();
	void loadRunDetails(NeXus::NXEntry & entry);
	void loadExperimentDetails(NeXus::NXEntry & entry);
	int getDetectorElasticPeakPosition(const NeXus::NXInt &data);
	void loadTimeDetails(NeXus::NXEntry& entry);
	NeXus::NXData loadNexusFileData(NeXus::NXEntry& entry);
	void loadDataIntoTheWorkSpace(NeXus::NXEntry& entry, int vanaCalculatedDetectorElasticPeakPosition = -1);

	void runLoadInstrument();

	/// Calculate error for y
	static double calculateError(double in) {
		return sqrt(in);
	}
	int validateVanadium(const std::string &);

	API::MatrixWorkspace_sptr m_localWorkspace;

//	NeXus::NXRoot m_dataRoot;
//	NeXus::NXRoot m_vanaRoot;

	std::string m_instrumentName; ///< Name of the instrument
	std::string m_instrumentPath; ///< Name of the instrument path

	// Variables describing the data in the detector
	size_t m_numberOfTubes; // number of tubes - X
	size_t m_numberOfPixelsPerTube; //number of pixels per tube - Y
	size_t m_numberOfChannels; // time channels - Z
	size_t m_numberOfHistograms;

	/* Values parsed from the nexus file */
	int m_monitorElasticPeakPosition;
	double m_wavelength;
	double m_channelWidth;

	double m_l1; //=2.0;
	double m_l2; //=4.0;

	std::vector<std::string> m_supportedInstruments;
	LoadHelper m_loader;

};

} // namespace DataHandling
} // namespace Mantid

#endif /*MANTID_DATAHANDLING_LoadILL_H_*/

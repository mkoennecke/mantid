#ifndef MANTID_DATAHANDLING_LOADILL_H_
#define MANTID_DATAHANDLING_LOADILL_H_

//---------------------------------------------------
// Includes
//---------------------------------------------------
#include "MantidAPI/Algorithm.h"
#include "MantidAPI/IDataFileChecker.h"
#include "MantidNexus/NexusClasses.h"

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
class DLLExport LoadILL: public API::IDataFileChecker {
public:
	/// Constructor
	LoadILL() :
			API::IDataFileChecker(), m_instrumentName(""),
			//m_nexusInstrumentEntryName(""),
			m_wavelength(0), m_channelWidth(0) {

		supportedInstruments.push_back("IN4");
		supportedInstruments.push_back("IN5");
		supportedInstruments.push_back("IN6");

	}
	/// Virtual destructor
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
	///checks the file can be loaded by reading 1st 100 bytes and looking at the file extension.
	bool quickFileCheck(const std::string& filePath, size_t nread,
			const file_header& header);
	/// check the structure of the file and if this file can be loaded return a value between 1 and 100
	int fileCheck(const std::string& filePath);
private:
	/// Sets documentation strings for this algorithm
	virtual void initDocs();
	// Initialisation code
	void init();
	// Execution code
	void exec();

	void setInstrumentName(NeXus::NXEntry& entry);
	std::string getInstrumentName(NeXus::NXEntry& entry);
	void initWorkSpace(NeXus::NXEntry& entry);
	void initInstrumentSpecific();
	void loadRunDetails(NeXus::NXEntry & entry);
	void loadExperimentDetails(NeXus::NXEntry & entry);
	int getDetectorElasticPeakPosition(const NeXus::NXInt &data);
	void loadTimeDetails(NeXus::NXEntry& entry);
	NeXus::NXData loadNexusFileData(NeXus::NXEntry& entry);
	void loadDataIntoTheWorkSpace(NeXus::NXEntry& entry);

	double calculateTOF(double);
	void runLoadInstrument();

	std::string getDateTimeInIsoFormat(std::string dateToParse);
	// Load all the nexus file information

	API::MatrixWorkspace_sptr m_localWorkspace;

	std::string m_filename; ///< The file to load
	std::string m_instrumentName; ///< Name of the instrument

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

	std::vector<std::string> supportedInstruments;

	// Nexus instrument entry is of the format /entry0/<XXX>/
	// XXX changes from version to version
	std::string m_nexusInstrumentEntryName;

};

} // namespace DataHandling
} // namespace Mantid

#endif /*MANTID_DATAHANDLING_LoadILL_H_*/

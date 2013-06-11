#ifndef MANTID_DATAHANDLING_LOADSINQ_H_
#define MANTID_DATAHANDLING_LOADSINQ_H_

//---------------------------------------------------
// Includes
//---------------------------------------------------
#include "MantidAPI/Algorithm.h"
#include "MantidAPI/IDataFileChecker.h"
#include "MantidNexus/NexusClasses.h"

namespace Mantid {
namespace DataHandling {

/**
 Loads an PSI nexus file into a Mantid workspace.

 Required properties:
 <UL>
 <LI> Filename - The ILL nexus file to be read </LI>
 <LI> Workspace - The name to give to the output workspace </LI>
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
 Code Documentation is available at: <http://doxygen.mantidproject.org>
 */
class DLLExport LoadSINQ: public API::IDataFileChecker  {
public:
	LoadSINQ();
	virtual ~LoadSINQ();

	virtual const std::string name() const;
	virtual int version() const;
	virtual const std::string category() const;
	///checks the file can be loaded by reading 1st 100 bytes and looking at the file extension.
	bool quickFileCheck(const std::string& filePath, size_t nread,
			const file_header& header);
	/// check the structure of the file and if this file can be loaded return a value between 1 and 100
	int fileCheck(const std::string& filePath);

private:
	virtual void initDocs();
	void init();
	void exec();
	void setInstrumentName(NeXus::NXEntry& entry);
	std::string getInstrumentName(NeXus::NXEntry& entry);
	void initWorkSpace(NeXus::NXEntry&);
	void loadDataIntoTheWorkSpace(NeXus::NXEntry&);
	/// Calculate error for y
	static double calculateError(double in) {
		return sqrt(in);
	}
	void loadExperimentDetails(NeXus::NXEntry&);
	void loadRunDetails(NeXus::NXEntry &);
	void runLoadInstrument();

	std::vector<std::string> supportedInstruments;
	std::string m_nexusInstrumentEntryName;
	std::string m_instrumentName;
	API::MatrixWorkspace_sptr m_localWorkspace;
	size_t m_numberOfTubes; // number of tubes - X
	size_t m_numberOfPixelsPerTube; //number of pixels per tube - Y
	size_t m_numberOfChannels; // time channels - Z
	size_t m_numberOfHistograms;

};

} // namespace DataHandling
} // namespace Mantid

#endif  /* MANTID_DATAHANDLING_LOADSINQ_H_ */

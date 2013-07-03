#ifndef MANTID_DATAHANDLING_LOADILLSANS_H_
#define MANTID_DATAHANDLING_LOADILLSANS_H_

#include "MantidKernel/System.h"
#include "MantidAPI/Algorithm.h"
#include "MantidAPI/IDataFileChecker.h"
#include "MantidNexus/NexusClasses.h"
#include "MantidDataHandling/LoadHelper.h"

namespace Mantid {
namespace DataHandling {

/** LoadILLSANS

 To date this only supports ILL D33 SANS-TOF instrument

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

struct DetectorPosition {
	double distanceSampleRear;
	double distanceSampleBottomTop;
	double distanceSampleRightLeft;
	double shiftLeft;
	double shiftRight;
	double shiftUp;
	double shiftDown;
};

std::ostream& operator<<(std::ostream &strm, const DetectorPosition &p) {
	return strm << "DetectorPosition : " << "distanceSampleRear = "
			<< p.distanceSampleRear << ", " << "distanceSampleBottomTop = "
			<< p.distanceSampleBottomTop << ", " << "distanceSampleRightLeft = "
			<< p.distanceSampleRightLeft << ", " << "shiftLeft = "
			<< p.shiftLeft << ", " << "shiftRight = " << p.shiftRight << ", "
			<< "shiftUp = " << p.shiftUp << ", " << "shiftDown = "
			<< p.shiftDown << std::endl;
}

class DLLExport LoadILLSANS: public API::IDataFileChecker {
public:
	LoadILLSANS();
	virtual ~LoadILLSANS();

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
	void setInstrumentName(const NeXus::NXEntry&, const std::string&);
	DetectorPosition getDetectorPosition(const NeXus::NXEntry&,
			const std::string &);
	void initWorkSpace(NeXus::NXEntry&, const std::string&);
	void createEmptyWorkspace(int, int);
	size_t loadDataIntoWorkspaceFromHorizontalTubes(NeXus::NXInt &, const std::vector<double> &,
			size_t);
	size_t loadDataIntoWorkspaceFromVerticalTubes(NeXus::NXInt &, const std::vector<double> &,
				size_t);
	void runLoadInstrument();
	void moveDetectors(const DetectorPosition&);
	void moveDetectorDistance(double, const std::string&);
	void moveDetectorHorizontal(double, const std::string&);
	void moveDetectorVertical(double, const std::string&);
	Kernel::V3D getComponentPosition(const std::string& componentName);
	LoadHelper m_loader;
	std::string m_instrumentName; ///< Name of the instrument
	std::vector<std::string> supportedInstruments;
	API::MatrixWorkspace_sptr m_localWorkspace;

};

} // namespace DataHandling
} // namespace Mantid

#endif  /* MANTID_DATAHANDLING_LOADILLSANS_H_ */

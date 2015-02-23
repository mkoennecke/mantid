/*
 * SINQCCDListener.h
 *
 * This is a Mantid live data listener for a camera connected
 * to an EPICS area detector IOC.
 *
 * Original contributor: Mark Koennecke: mark.koennecke@psi.ch
 * 
 * Copyright &copy; 2013 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

 * This file is part of Mantid.

 * Mantid is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mantid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.

 * File change history is stored at: <https://github.com/mantidproject/mantid>
 * Code Documentation is available at: <http://doxygen.mantidproject.org>
 */

#ifndef EPICSCCDLISTENER_H_
#define EPICSCCDLISTENER_H_

#include "MantidSINQ/DllConfig.h"
#include "MantidKernel/Logger.h"
#include "MantidAPI/ILiveListener.h"
#include "MantidAPI/IMDHistoWorkspace.h"
#include "MantidGeometry/IDTypes.h"
#include "MantidGeometry/MDGeometry/MDTypes.h"
#include "MantidAPI/Algorithm.h"

using namespace Mantid;

class MANTID_SINQ_DLL EPICSCCDListener : public Mantid::API::ILiveListener
{
public:
	EPICSCCDListener();
	~EPICSCCDListener();

	std::string name() const {return "EPICSCCDListener";}
	bool supportsHistory() const {return false;}
	bool buffersEvents() const {return false;}

	bool connect(const Poco::Net::SocketAddress& address);
	void start(Mantid::Kernel::DateAndTime startTime = Mantid::Kernel::DateAndTime());
    boost::shared_ptr<Mantid::API::Workspace> extractData();
    bool isConnected();
    ILiveListener::RunStatus runStatus();
    int runNumber() const { return irunNumber; }

    void setSpectra(const std::vector<Mantid::specid_t>& specList);

    void setXDim(int xdim);
    void setYDim(int ydim);
    void setRunNumber(int num);
    int *getDataPtr(){
    	return data;
    };
    void imageUpdate(){
    	newImage = true;
    };

private:
    unsigned int imageX;
    unsigned int imageY;
    unsigned int imageCount;
    int irunNumber;
    void resize();
    bool connected;
    bool newImage;

    int *data;

  };

#endif /* EPICSCCDLISTENER_H_ */

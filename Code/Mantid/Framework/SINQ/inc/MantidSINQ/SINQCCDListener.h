/*
 * SINQCCDListener.h
 *
 * This is a Mantid live data listener for the HTTP based
 * CCD server used at SINQ(PSI) and especially at BOA
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

#ifndef SINQCCDLISTENER_H_
#define SINQCCDLISTENER_H_

#include "MantidSINQ/DllConfig.h"
#include "MantidKernel/Logger.h"
#include "MantidAPI/ILiveListener.h"
#include "MantidAPI/IMDHistoWorkspace.h"
#include "MantidGeometry/IDTypes.h"
#include "MantidGeometry/MDGeometry/MDTypes.h"
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPClientSession.h>
#include "MantidAPI/Algorithm.h"

using namespace Mantid;

class MANTID_SINQ_DLL SINQCCDListener : public Mantid::API::ILiveListener
{
public:
	SINQCCDListener();
	~SINQCCDListener();

	std::string name() const {return "SINQCCDListener";}
	bool supportsHistory() const {return false;}
	bool buffersEvents() const {return false;}

	bool connect(const Poco::Net::SocketAddress& address);
	void start(Mantid::Kernel::DateAndTime startTime = Mantid::Kernel::DateAndTime());
    boost::shared_ptr<Mantid::API::Workspace> extractData();
    bool isConnected();
    ILiveListener::RunStatus runStatus();
    int runNumber() const { return 0; }

    void setSpectra(const std::vector<Mantid::specid_t>& specList);

private:
    Poco::Net::HTTPClientSession httpcon;
    Poco::Net::HTTPResponse response;
    bool connected;
    std::string hmhost;
    int imageNo;
    /**
     * This is the synchronisation count used for waiting for
     * new images. There is a dependency here to CCD server which has
     * to maintain this number and not give new data until this has chnaged.
     */
    unsigned int imageCount;

    unsigned int getImageCount();

    Mantid::API::Algorithm *alg;
 };

#endif /* SINQCCDLISTENER_H_ */

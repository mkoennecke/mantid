/*
 * SINQHMListener.cpp
 *
 *  Created on: March, 25, 2014
 *      Author: mark.koennecke@psi.ch
 */
#include "MantidSINQ/SINQCCDListener.h"
#include "MantidAPI/LiveListenerFactory.h"
#include "MantidAPI/AlgorithmFactory.h"
#include "MantidAPI/AlgorithmManager.h"
#include "MantidMDEvents/MDHistoWorkspace.h"
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPBasicCredentials.h>
#include <Poco/StreamCopier.h>
#include <iostream>
#include <sstream>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/Timespan.h>
#include <Poco/ActiveResult.h>
#include <MantidSINQ/WaitCancel.h>

using namespace Mantid::API;
using namespace Mantid::Geometry;
using namespace Mantid;
using namespace Mantid::MDEvents;
using namespace Poco::Net;
using namespace Poco::XML;
using namespace Mantid::SINQ;

DECLARE_LISTENER(SINQCCDListener)


SINQCCDListener::SINQCCDListener() :ILiveListener(), httpcon(), response()
{
	connected = false;
	imageNo = -1;
	imageCount = 0;
    Mantid::API::IAlgorithm_sptr waitCancels;

	std::vector<IAlgorithm_const_sptr> wc = AlgorithmManager::Instance().runningInstancesOf("WaitCancel");
	if(wc.empty()){
		waitCancels = API::AlgorithmManager::Instance().create("WaitCancel",-1,false);
	    WaitCancel * waitCancel = dynamic_cast<WaitCancel*>(waitCancels.get());
	    alg = dynamic_cast<Mantid::API::Algorithm *>(waitCancels.get());

		if ( !waitCancel ) return;

	    waitCancel->initialize();
		try
		{
			waitCancel->executeAsync();
		}
		catch (std::runtime_error&)
		{
			alg->getLogger().information("Unable to successfully run WaitCancel Child Algorithm");
		}
	}

}

SINQCCDListener::~SINQCCDListener()
{

}

bool SINQCCDListener::connect(const Poco::Net::SocketAddress& address)
{
	Poco::Timespan ts(0,0,30,0,0);

	std::string host = address.toString();
	std::string::size_type i = host.find(':');
	if ( i != std::string::npos )
	{
		host.erase( i );
	}
	httpcon.setHost(host);
	httpcon.setPort(address.port());
	httpcon.setKeepAlive(true);
	httpcon.setKeepAliveTimeout(ts);
	httpcon.setTimeout(ts);
	connected = true;

	return true;
}

bool SINQCCDListener::isConnected()
{
	return connected;
}

ILiveListener::RunStatus SINQCCDListener::runStatus()
{

	return Running;

}
unsigned int SINQCCDListener::getImageCount()
{
	HTTPRequest req(HTTPRequest::HTTP_GET,"/ccd/imagecount", HTTPMessage::HTTP_1_1);
	req.setKeepAlive(true);
	HTTPBasicCredentials cred("spy","007");
	cred.authenticate(req);
	httpcon.reset();
	httpcon.sendRequest(req);
	std::istream& istr = httpcon.receiveResponse(response);
	if(response.getStatus() != HTTPResponse::HTTP_OK){
		printf("Failed reading CCD with %s\n", response.getReason().c_str());
		throw  std::runtime_error("Failed to get /ccd/imagecount with reason " + response.getReason());
	}
	std::string sCount;
	istr >> sCount;

	return (unsigned int)atoi(sCount.c_str());
}

boost::shared_ptr<Workspace> SINQCCDListener::extractData()
{
	int dim[2], imNo = -2;
	char *dimNames[] = {"x","y"};
	char request[132];
	Poco::Timespan polli(0,0,0,0,10);

	printf("Executing SINQCCDListener::extractData with %d\n", imageCount);

	while(getImageCount() == imageCount){
		std::vector<IAlgorithm_const_sptr> wc = AlgorithmManager::Instance().runningInstancesOf("WaitCancel");
		if(wc.empty()){
			throw std::runtime_error("SINQCCDListener Execution interrupted");
		}
		usleep(50);
	}
	printf("Detected new image %d versus %d\n", imageCount, getImageCount());

	snprintf(request,sizeof(request),"/ccd/waitdata?imageCount=%d", imageCount);
	HTTPRequest req(HTTPRequest::HTTP_GET,request, HTTPMessage::HTTP_1_1);
	req.setKeepAlive(true);
	httpcon.reset();
	HTTPBasicCredentials cred("spy","007");
	cred.authenticate(req);
	httpcon.sendRequest(req);

	printf("SINQCCDListener::extractData waiting for response\n");

	std::istream& istr = httpcon.receiveResponse(response);
	if(response.getStatus() != HTTPResponse::HTTP_OK){
		printf("Failed reading CCD with %s\n", response.getReason().c_str());
		throw  std::runtime_error("Failed to get /ccd/waitdata with reason " + response.getReason());
	}

	printf("In Loop: SINQCCDListener::extractData after reading HTTP\n");

	std::string ImageDim = response.get("ImageDim");
	unsigned pos = ImageDim.find("x");
	dim[0] = atoi(ImageDim.substr(0,pos).c_str());
	dim[1] = atoi(ImageDim.substr(pos+1,std::string::npos).c_str());
	unsigned int length = dim[0]*dim[1];

	std::vector<MDHistoDimension_sptr> dimensions;
	for(int i = 0; i < 2; i++){
		dimensions.push_back(MDHistoDimension_sptr(new MDHistoDimension(dimNames[i], dimNames[i], "", .0, coord_t(dim[i]), dim[i])));
	}
	MDHistoWorkspace_sptr ws (new MDHistoWorkspace(dimensions));
	ws->setTo(.0,.0,.0);

	//printf("In Loop: SINQCCDListener::extractData after creating ws\n");

	int *data = (int *)malloc(length*sizeof(int));
	if(data == NULL){
		throw std::runtime_error("Out of memory reading HM data");
	}
	istr.read((char *)data,length*sizeof(int));
	if(!istr.good()){
		std::cout << "Encountered Problem before reading all SINQCCD data" << std::endl;
		return ws;
	}
	for(unsigned int i = 0; i < length ; i++){
		data[i] = ntohl(data[i]);
		ws->setSignalAt(i,signal_t(data[i]));
		ws->setErrorSquaredAt(i,signal_t(data[i]));
	}

	printf("In Loop: SINQCCDListener::extractData after reading data\n");

	std::string ImageNo = response.get("ImageCount");
	imageCount = atoi(ImageNo.c_str());

	ImageNo = response.get("Scan-NP");
	imageNo = atoi(ImageNo.c_str());

	ws->addExperimentInfo((ExperimentInfo_sptr)new ExperimentInfo());
	ws->getExperimentInfo(0)->mutableRun().addProperty("Image-No",ImageNo, true);
	ws->setTitle(std::string("Image-NO: ") + ImageNo);

	alg->getLogger().information() << "Loaded SINQ CCD Live Image No " << imageNo << " imageCount " \
			    << imageCount << std::endl;

	return ws;

}

void SINQCCDListener::setSpectra(const std::vector<Mantid::specid_t>& /*specList*/)
{
	/**
	 * Nothing to do: we always go for the full data.
	 * SINQCCD would do subsampling but this cannot easily
	 * be expressed as a spectra map.
	 */
}

void SINQCCDListener::start(Mantid::Kernel::DateAndTime /*startTime */)
{
	// Nothing to do here
}




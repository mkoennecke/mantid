/*
 * EPICSCCDListener.cpp
 *
 *  Created on: February, 18, 2015
 *      Author: mark.koennecke@psi.ch
 */
#include "MantidSINQ/EPICSCCDListener.h"
#include "MantidAPI/LiveListenerFactory.h"
#include "MantidAPI/AlgorithmFactory.h"
#include "MantidAPI/AlgorithmManager.h"
#include "MantidMDEvents/MDHistoWorkspace.h"
#include <Poco/ActiveResult.h>
#include <MantidSINQ/WaitCancel.h>
#include <cadef.h>
#include <sstream>

using namespace Mantid::API;
using namespace Mantid::Geometry;
using namespace Mantid;
using namespace Mantid::MDEvents;
using namespace Mantid::SINQ;

DECLARE_LISTENER(EPICSCCDListener)

namespace {
/// static logger
Kernel::Logger g_log("EPICSCCDListener");
}

EPICSCCDListener::EPICSCCDListener() :ILiveListener()
{
	connected = false;
	imageCount = -1;
	newImage = false;

	imageX = imageY = 1024;
	data = NULL;
	resize();

    Mantid::API::IAlgorithm_sptr waitCancels;

	std::vector<IAlgorithm_const_sptr> wc = AlgorithmManager::Instance().runningInstancesOf("WaitCancel");
	if(wc.empty()){
		waitCancels = API::AlgorithmManager::Instance().create("WaitCancel",-1,false);
	    WaitCancel * waitCancel = dynamic_cast<WaitCancel*>(waitCancels.get());

		if ( !waitCancel ) return;

	    waitCancel->initialize();
		try
		{
			waitCancel->executeAsync();
		}
		catch (std::runtime_error&)
		{
			g_log.information("Unable to successfully run WaitCancel Child Algorithm");
		}
	}

}

void EPICSCCDListener::resize()
{
	if(data != NULL){
		delete[] data;
	}
	data = new int[imageX*imageY*sizeof(int)];
}
void EPICSCCDListener::setXDim(int x)
{
	imageX = x;
	resize();
}
void EPICSCCDListener::setYDim(int x)
{
	imageY = x;
	resize();
}
void EPICSCCDListener::setRunNumber(int x)
{
	imageCount = -1;
	irunNumber = x;
}


EPICSCCDListener::~EPICSCCDListener()
{
	if(data != NULL){
		delete[] data;
	}
}

static void epicsXDimCallback(struct event_handler_args args)
{
	int val;

	if(args.status == ECA_NORMAL){
		val = *(int *)args.dbr;
		EPICSCCDListener *obj = (EPICSCCDListener *)args.usr;
		obj->setXDim(val);
	}
}

static void epicsYDimCallback(struct event_handler_args args)
{
	int val;

	if(args.status == ECA_NORMAL){
		val = *(int *)args.dbr;
		EPICSCCDListener *obj = (EPICSCCDListener *)args.usr;
		obj->setYDim(val);
	}
}

static void epicsRunCallback(struct event_handler_args args)
{
	int val;

	if(args.status == ECA_NORMAL){
		val = *(int *)args.dbr;
		EPICSCCDListener *obj = (EPICSCCDListener *)args.usr;
		obj->setRunNumber(val);
	}
}

static void epicsDataCallback(struct event_handler_args args)
{

	printf("EPICS Data Callback with status %d\n", args.status);

	if(args.status == ECA_NORMAL){
		EPICSCCDListener *obj = (EPICSCCDListener *)args.usr;
		int *target = obj->getDataPtr();
		memcpy(target,args.dbr,args.count*sizeof(int));
		obj->imageUpdate();
	}
}


bool EPICSCCDListener::connect(const Poco::Net::SocketAddress& address)
{
	int status;
	chid pvchid;

	status = ca_context_create(ca_enable_preemptive_callback);
	//std::string prefix = address.toString();
	std::string prefix("SQBOA-CCD:cam1");

	std::string name = prefix + std::string(":ArraySizeX_RBV");
	status = ca_create_channel(name.c_str(),NULL,NULL,10,&pvchid);
	if(status != ECA_NORMAL){
		throw  std::runtime_error("Failed to locate " + name);
	}
	status = ca_pend_io(.2);
	if(status != ECA_NORMAL){
		throw  std::runtime_error("Timeout connecting to " + name);
	}
	status = ca_create_subscription(DBR_LONG,0,pvchid,
		DBE_VALUE|DBE_ALARM,epicsXDimCallback,this,NULL);

	name = prefix + std::string(":ArraySizeY_RBV");
	status = ca_create_channel(name.c_str(),NULL,NULL,10,&pvchid);
	if(status != ECA_NORMAL){
		throw  std::runtime_error("Failed to locate " + name);
	}
	status = ca_pend_io(.2);
	if(status != ECA_NORMAL){
		throw  std::runtime_error("Timeout connecting to " + name);
	}
	status = ca_create_subscription(DBR_LONG,0,pvchid,
		DBE_VALUE|DBE_ALARM,epicsYDimCallback,this,NULL);

	name = prefix + std::string(":RunNumber");
	status = ca_create_channel(name.c_str(),NULL,NULL,10,&pvchid);
	if(status == ECA_NORMAL){
		status = ca_pend_io(.2);
		status = ca_create_subscription(DBR_LONG,0,pvchid,
				DBE_VALUE|DBE_ALARM,epicsRunCallback,this,NULL);
	}

	/*
	 * Here we have a problem: I need the prefix without the camera part. Normally I would have to
	 * pass in two parameters for the address to cover that. I now use the convention that the EPICS address
	 * is a colon separated list and I get away with splitting off the last item.
	 */
	size_t idx = prefix.find_last_of(':');
	name = prefix.substr(0,idx) + std::string(":image1:ArrayData");
	status = ca_create_channel(name.c_str(),NULL,NULL,10,&pvchid);
	if(status != ECA_NORMAL){
		throw  std::runtime_error("Failed to locate " + name);
	}
	status = ca_pend_io(.2);
	if(status != ECA_NORMAL){
		throw  std::runtime_error("Timeout connecting to " + name);
	}
	status = ca_create_subscription(DBR_LONG,0,pvchid,
		DBE_VALUE|DBE_ALARM,epicsDataCallback,this,NULL);

	connected = true;

	return true;
}

bool EPICSCCDListener::isConnected()
{
	return connected;
}

ILiveListener::RunStatus EPICSCCDListener::runStatus()
{

	return Running;

}

boost::shared_ptr<Workspace> EPICSCCDListener::extractData()
{
	int dim[2];
	char *dimNames[] = {"x","y"};

	printf("Executing EPICSCCDListener::extractData with %d\n", imageCount);

	while(!newImage){
		std::vector<IAlgorithm_const_sptr> wc = AlgorithmManager::Instance().runningInstancesOf("WaitCancel");
		if(wc.empty()){
			throw std::runtime_error("EPICSCCDListener Execution interrupted");
		}
		//ca_pend_event(0.05);
		usleep(50);
	}
	imageCount++;
	printf("Detected new image %d\n", imageCount);

	dim[0] = imageX;
	dim[1] = imageY;
	unsigned int length = dim[0]*dim[1];

	std::vector<MDHistoDimension_sptr> dimensions;
	for(int i = 0; i < 2; i++){
		dimensions.push_back(MDHistoDimension_sptr(new MDHistoDimension(dimNames[i], dimNames[i], "", .0, coord_t(dim[i]), dim[i])));
	}
	MDHistoWorkspace_sptr ws (new MDHistoWorkspace(dimensions));
	ws->setTo(.0,.0,.0);

/*
	for(unsigned int i = 0; i < length ; i++){
		ws->setSignalAt(i,signal_t(data[i]));
		ws->setErrorSquaredAt(i,signal_t(data[i]));
	}
*/
	/*
	 * copy to WS, but mirror on the fly
	 */
	for(int y = 0; y < imageY; y++){
		int srcIdx = y*imageX;
		int targetIDX = (imageY-y-1)*imageX;
		for(int x = 0; x < imageX; x++){
			ws->setSignalAt(targetIDX+x, signal_t(data[srcIdx+x]));
			ws->setErrorSquaredAt(targetIDX+x, signal_t(data[srcIdx+x]));
		}

	}


	ws->addExperimentInfo((ExperimentInfo_sptr)new ExperimentInfo());
	ws->getExperimentInfo(0)->mutableRun().addProperty("Image-No",imageCount, true);
	std::ostringstream oss;
	oss << imageCount;
	ws->setTitle(std::string("Image-NO: ") + oss.str());

	g_log.information() << "Loaded EPICS CCD Live Image No " <<  imageCount  << std::endl;
	newImage = false;

	return ws;



}

void EPICSCCDListener::setSpectra(const std::vector<Mantid::specid_t>& /*specList*/)
{
	/**
	 * Nothing to do: we always go for the full data.
	 * EPICSCCD would do subsampling but this cannot easily
	 * be expressed as a spectra map.
	 */
}

void EPICSCCDListener::start(Mantid::Kernel::DateAndTime /*startTime */)
{
	// Nothing to do here
}




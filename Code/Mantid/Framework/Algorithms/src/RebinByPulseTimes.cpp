/*WIKI*
Rebins an EventWorkspace according to the pulse times of each event rather than the time of flight [[Rebin]]. The Params inputs may be expressed in an identical manner to the [[Rebin]] algorithm. 
Users may either provide a single value, which is interpreted as the ''step'' (in seconds), or three comma separated values ''start'', ''step'', ''end'', where all units are in seconds, and start and end are relative to the start of the run.

The x-axis is expressed in relative time to the start of the run in seconds.

This algorithm may be used to diagnose problems with the electronics or data collection. Typically, detectors should see a uniform distribution of the events 
generated between the start and end of the run. This algorithm allows anomalies to be detected.

== Example of Use ==

This diagnostic algorithm is particularly useful when coupled with the Instrument View. In the example below is a real-world usage example where we were able to highlight issues with data collection on the ISIS WISH instrument. Some blocks of tubes, where tubes are arranged vertically, are missing neutrons within large block of pulse time as a result of data-buffering. After running RebinByPulseTime, we were able to find both, which banks were affected, as well as the missing pulse times for each bank. The horizontal slider in the instrument view allows us to easily integrate over a section of pulse time and see the results as a colour map.


[[File:RebinByPulseTime.png]]


*WIKI*/

#include "MantidAlgorithms/RebinByPulseTimes.h"
#include "MantidDataObjects/EventWorkspace.h"
#include "MantidKernel/ArrayProperty.h"
#include "MantidKernel/RebinParamsValidator.h"
#include "MantidKernel/VectorHelper.h"
#include "MantidKernel/Unit.h"
#include <boost/make_shared.hpp>
#include <boost/assign/list_of.hpp>
#include <algorithm>

using namespace Mantid::Kernel;
using namespace Mantid::API;

namespace Mantid
{
namespace Algorithms
{

  /**
  Helper method to transform a MantidVector containing absolute times in nanoseconds to relative times in seconds given an offset.
  */
  class ConvertToRelativeTime : public std::unary_function<const MantidVec::value_type&, MantidVec::value_type>
  {
  private:
    double m_offSet;
  public: 
     ConvertToRelativeTime(const DateAndTime& offSet) : m_offSet(static_cast<double>(offSet.totalNanoseconds())*1e-9){}
     MantidVec::value_type operator()(const MantidVec::value_type& absTNanoSec)
     {
       return (absTNanoSec * 1e-9) - m_offSet;
     }
  };

  // Register the algorithm into the AlgorithmFactory
  DECLARE_ALGORITHM(RebinByPulseTimes)

  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  RebinByPulseTimes::RebinByPulseTimes()
  {
  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  RebinByPulseTimes::~RebinByPulseTimes()
  {
  }
  

  //----------------------------------------------------------------------------------------------
  /// Algorithm's name for identification. @see Algorithm::name
  const std::string RebinByPulseTimes::name() const { return "RebinByPulseTimes";};
  
  /// Algorithm's version for identification. @see Algorithm::version
  int RebinByPulseTimes::version() const { return 1;};
  
  /// Algorithm's category for identification. @see Algorithm::category
  const std::string RebinByPulseTimes::category() const { return "Transforms\\Rebin";}

  //----------------------------------------------------------------------------------------------
  /// Sets documentation strings for this algorithm
  void RebinByPulseTimes::initDocs()
  {
    this->setWikiSummary("Bins events according to pulse time. Binning parameters are specified relative to the start of the run.");
    this->setOptionalMessage("Bins events according to pulse time. Binning parameters are specified relative to the start of the run.");
  }

  //----------------------------------------------------------------------------------------------
  /** Initialize the algorithm's properties.
  */
  void RebinByPulseTimes::init()
  {
    declareProperty(new API::WorkspaceProperty<API::IEventWorkspace>("InputWorkspace","",Direction::Input), "An input workspace containing TOF events.");
    declareProperty(
      new ArrayProperty<double>("Params", boost::make_shared<RebinParamsValidator>()),
      "A comma separated list of first bin boundary, width, last bin boundary. Optionally\n"
      "this can be followed by a comma and more widths and last boundary pairs.");
    declareProperty(new API::WorkspaceProperty<API::MatrixWorkspace>("OutputWorkspace","",Direction::Output), "An output workspace.");
  }

  //----------------------------------------------------------------------------------------------
  /** Execute the algorithm.
  */
  void RebinByPulseTimes::exec()
  {
    using Mantid::DataObjects::EventWorkspace;
    IEventWorkspace_sptr inWS = getProperty("InputWorkspace");
    if(!boost::dynamic_pointer_cast<EventWorkspace>(inWS))
    {
      throw std::invalid_argument("RebinByPulseTimes requires an EventWorkspace as an input.");
    }

    MatrixWorkspace_sptr outputWS = getProperty("OutputWorkspace"); // TODO: MUST BE A HISTOGRAM WORKSPACE!

    // retrieve the properties
    const std::vector<double> inParams =getProperty("Params");
    std::vector<double> rebinningParams;

    // workspace independent determination of length
    const int histnumber = static_cast<int>(inWS->getNumberHistograms());
    
    const uint64_t nanoSecondsInASecond = static_cast<uint64_t>(1e9);
    const DateAndTime runStartTime = inWS->run().startTime();
    // The validator only passes parameters with size 1, or 3xn.  

    double tStep = 0; 
    if (inParams.size() >= 3)
    {
      // Use the start of the run to offset the times provided by the user. pulse time of the events are absolute.
      const DateAndTime startTime = runStartTime + inParams[0] ;
      const DateAndTime endTime = runStartTime + inParams[2] ;
      // Rebinning params in nanoseconds.
      rebinningParams.push_back(static_cast<double>(startTime.totalNanoseconds()));
      tStep = inParams[1] * nanoSecondsInASecond;
      rebinningParams.push_back(tStep);
      rebinningParams.push_back(static_cast<double>(endTime.totalNanoseconds()));
    } 
    else if (inParams.size() == 1)
    {
      const uint64_t xmin = inWS->getPulseTimeMin().totalNanoseconds();
      const uint64_t xmax = inWS->getPulseTimeMax().totalNanoseconds();

      rebinningParams.push_back(static_cast<double>(xmin));
      tStep = inParams[0] * nanoSecondsInASecond;
      rebinningParams.push_back(tStep);
      rebinningParams.push_back(static_cast<double>(xmax));
    }

    // Validate the timestep.
    if(tStep <= 0)
    {
      throw std::invalid_argument("Cannot have a timestep less than or equal to zero.");
    }

    //Initialize progress reporting.
    Progress prog(this,0.0,1.0, histnumber);
    
    MantidVecPtr XValues_new;
    // create new X axis, with absolute times in seconds.
    const int ntcnew = VectorHelper::createAxisFromRebinParams(rebinningParams, XValues_new.access());

    ConvertToRelativeTime transformToRelativeT(runStartTime);

    // Transform the output into relative times in seconds.
    MantidVec OutXValues_scaled(XValues_new->size());
    std::transform(XValues_new->begin(), XValues_new->end(), OutXValues_scaled.begin(), transformToRelativeT);

    outputWS = WorkspaceFactory::Instance().create("Workspace2D",histnumber,ntcnew,ntcnew-1);
    WorkspaceFactory::Instance().initializeFromParent(inWS, outputWS, true);

    //Go through all the histograms and set the data
    PARALLEL_FOR2(inWS, outputWS)
    for (int i=0; i < histnumber; ++i)
    {
      PARALLEL_START_INTERUPT_REGION

      const IEventList* el = inWS->getEventListPtr(i);
      MantidVec y_data, e_data;
      // The EventList takes care of histogramming.
      el->generateHistogramPulseTime(*XValues_new, y_data, e_data);

      //Set the X axis for each output histogram
      outputWS->setX(i, OutXValues_scaled);

      //Copy the data over.
      outputWS->dataY(i).assign(y_data.begin(), y_data.end());
      outputWS->dataE(i).assign(e_data.begin(), e_data.end());

      //Report progress
      prog.report(name());
      PARALLEL_END_INTERUPT_REGION
    }
    PARALLEL_CHECK_INTERUPT_REGION

    //Copy all the axes
    for (int i=1; i<inWS->axes(); i++)
    {
      outputWS->replaceAxis( i, inWS->getAxis(i)->clone(outputWS.get()) );
      outputWS->getAxis(i)->unit() = inWS->getAxis(i)->unit();
    }

    // X-unit is relative time since the start of the run.
    outputWS->getAxis(0)->unit() = boost::make_shared<Units::Time>();

    //Copy the units over too.
    for (int i=1; i < outputWS->axes(); ++i)
    {
      outputWS->getAxis(i)->unit() = inWS->getAxis(i)->unit();
    }
    outputWS->setYUnit(inWS->YUnit());
    outputWS->setYUnitLabel(inWS->YUnitLabel());

    // Assign it to the output workspace property
    setProperty("OutputWorkspace", outputWS);

    return;
  }

} // namespace Algorithms
} // namespace Mantid

/*WIKI* 

This algorithm is able to generate one or a series of events filters according to user's requirement.  These filters are stored in a [[SplittersWorkspace]] class object, which will be fed into algorithm [[FilterEvents]].  Each filter has an individual value of workspace index. 

This algorithm is one way to generate one filter or a series of filter, while it is designed for general-purpose as much as possible.  Combined with [[FilterEvents]], it will replace 
 * [[FilterByTime]]
 * [[FilterByLogValue]]

Moreover, the time resolution of these two algorithms is microseconds, i.e., the wall time of an (neutron) event.  While the original [[FilterByTime]] and [[FilterByLogValue]] are of the resolution of pulse time.

==== Functionalities ====  
Here are the types of event filters (i.e., [[SplittersWorkspace]]) that can be generated by this algorithm:
* A filter for one time interval.  

* A series of filters for multiple continuous time intervals, which have the same length of period.  Each of them has an individual workspace index associated.  These workspace indices are incremented by 1 from 0 along with their orders in time. 

* A filter containing one or multiple time intervals according to a specified log value.  Any log value of the time that falls into the selected time intervals is equal or within the tolerance of a user specified value. 

* A series filters containing one or multiple time intervals according to specified log values incremented by a constant value.  Any log value of the time that falls into the selected time intervals is equal or within the tolerance of the log value as v_0 + n x delta_v +/- tolerance_v.

==== Parameter: ''Centre'' ====
The input Boolean parameter ''centre'' is for filtering by log value(s).  If option ''centre'' is taken, then for each interval, 
 * starting time = log_time - tolerance_time;
 * stopping time = log_time - tolerance_time;
It is a shift to left.

==== About how log value is recorded ====
SNS DAS records log values upon its changing.  The frequency of log sampling is significantly faster than change of the log, i.e., sample environment devices.  Therefore, it is reasonable to assume that all the log value changes as step functions.

The option to do interpolation is not supported at this moment.


*WIKI*/
//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------

#include "MantidAlgorithms/GenerateEventsFilter.h"
#include "MantidKernel/System.h"
#include "MantidKernel/ListValidator.h"
#include "MantidAPI/TableRow.h"
#include "MantidAPI/WorkspaceFactory.h"
#include "MantidAPI/WorkspaceProperty.h"
#include "MantidAPI/Column.h"

using namespace Mantid;
using namespace Mantid::Kernel;
using namespace Mantid::API;

using namespace std;

namespace Mantid
{
namespace Algorithms
{
  DECLARE_ALGORITHM(GenerateEventsFilter)

  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  GenerateEventsFilter::GenerateEventsFilter()
  {
  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  GenerateEventsFilter::~GenerateEventsFilter()
  {
  }
  
  void GenerateEventsFilter::initDocs()
  {
    this->setWikiSummary("Generate one or a set of event filters according to time or specified log's value.");
  }

  /*
   * Define input
   */
  void GenerateEventsFilter::init()
  {

    // 0. Input/Output Workspaces
    declareProperty(
      new API::WorkspaceProperty<DataObjects::EventWorkspace>("InputWorkspace", "Anonymous", Direction::Input),
      "An input event workspace" );

    declareProperty(
      new API::WorkspaceProperty<DataObjects::SplittersWorkspace>("OutputWorkspace", "Splitters", Direction::Output),
      "The name to use for the output SplittersWorkspace object, i.e., the filter." );

    declareProperty(new API::WorkspaceProperty<API::ITableWorkspace>("InformationWorkspace", "SplitterInfo",
                                                                     Direction::Output),
                    "Optional output for the information of each splitter workspace index");

    // 1. Time
    declareProperty("StartTime", "",
        "The start time, in (a) seconds, (b) nanoseconds or (c) percentage of total run time\n"
        "since the start of the run. OR (d) absolute time. \n"
        "Events before this time are filtered out.");

    declareProperty("StopTime", "",
        "The stop time, in (2) seconds, (b) nanoseconds or (c) percentage of total run time\n"
        "since the start of the run. OR (d) absolute time. \n"
        "Events at or after this time are filtered out.");

    declareProperty("TimeInterval", -1.0,
        "Length of the time splices if filtered in time only.");

    std::vector<std::string> timeoptions;
    timeoptions.push_back("Seconds");
    timeoptions.push_back("Nanoseconds");
    timeoptions.push_back("Percent");
    declareProperty("UnitOfTime", "Seconds", boost::make_shared<Kernel::StringListValidator>(timeoptions),
                    "StartTime, StopTime and DeltaTime can be given in various unit."
                    "The unit can be second or nanosecond from run start time."
                    "They can also be defined as percentage of total run time.");

    // 2. Log value
    declareProperty("LogName", "",
        "Name of the sample log to use to filter.\n"
        "For example, the pulse charge is recorded in 'ProtonCharge'.");

    declareProperty("MinimumLogValue", EMPTY_DBL(), "Minimum log value for which to keep events.");

    declareProperty("MaximumLogValue", EMPTY_DBL(), "Maximum log value for which to keep events.");

    declareProperty("LogValueInterval", -1.0,
        "Delta of log value to be sliced into from min log value and max log value.\n"
        "If not given, then only value ");

    std::vector<std::string> filteroptions;
    filteroptions.push_back("Both");
    filteroptions.push_back("Increase");
    filteroptions.push_back("Decrease");
    declareProperty("FilterLogValueByChangingDirection", "Both",
                    boost::make_shared<Kernel::StringListValidator>(filteroptions),
                    "d(log value)/dt can be positive and negative.  They can be put to different splitters.");

    declareProperty("TimeTolerance", 0.0,
        "Tolerance in time for the event times to keep. It is used in the case to filter by single value.");

    vector<string> boundaryoptions;
    boundaryoptions.push_back("centre");
    boundaryoptions.push_back("edge");
    declareProperty("LogBoundary", "centre", boost::make_shared<StringListValidator>(boundaryoptions),
        "How to treat log values as being measured in the centre of time.");

    declareProperty("LogValueTolerance", EMPTY_DBL(),
        "Tolerance of the log value to be included in filter.  It is used in the case to filter by multiple values.");

    /* removed due to SNS hardware
    std::vector<std::string> logvalueoptions;
    logvalueoptions.push_back("StepFunction");
    logvalueoptions.push_back("LinearInterpolation");
    declareProperty("LogValueInterpolation", "StepFunction", boost::make_shared<Kernel::StringListValidator>(logvalueoptions),
        "How to treat the changing log value in multiple-value filtering.");
    */

    declareProperty("LogValueTimeSections", 1,
        "In one log value interval, it can be further divided into sections in even time slice.");

    // Output workspaces' title and name
    declareProperty("TitleOfSplitters", "",
                    "Title of output splitters workspace and information workspace.");

    return;
  }


  //----------------------------------------------------------------------------------------------
  /** Main execute body
   */
  void GenerateEventsFilter::exec()
  {
    // 1. Get general input and output
    mEventWS = this->getProperty("InputWorkspace");
    if (!mEventWS)
    {
      std::stringstream errss;
      errss << "GenerateEventsFilter does not get input workspace as an EventWorkspace.";
      g_log.error(errss.str());
      throw std::runtime_error(errss.str());
    }
    else
    {
      g_log.debug() << "DB9441 GenerateEventsFilter() Input Event WS = " << mEventWS->getName()
                    << ", Events = " << mEventWS->getNumberEvents() << std::endl;
    }

    Kernel::DateAndTime runstart(mEventWS->run().getProperty("run_start")->value());
    g_log.debug() << "DB9441 Log Run Start = " << runstart << " / " << runstart.totalNanoseconds()
                  << std::endl;

    std::string title = getProperty("TitleOfSplitters");
    if (title.size() == 0)
    {
      // Using default
      title = "Splitters";
    }

    // Output Splitters workspace
    mSplitters =  boost::shared_ptr<DataObjects::SplittersWorkspace>(new DataObjects::SplittersWorkspace());
    mSplitters->setTitle(title);

    // mFilterInfoWS = boost::shared_ptr<DataObjects::TableWorkspace>(new DataObjects::TableWorkspace);
    mFilterInfoWS = API::WorkspaceFactory::Instance().createTable("TableWorkspace");
    mFilterInfoWS->setTitle(title);

    mFilterInfoWS->addColumn("int", "workspacegroup");
    mFilterInfoWS->addColumn("str", "title");

    // 2. Get Time
    processInputTime();

    double prog = 0.1;
    progress(prog);

    // 3. Get Log
    std::string logname = this->getProperty("LogName");
    if (logname.empty())
    {
      // a) Set filter by time only
      setFilterByTimeOnly();
    }
    else
    {
      // b) Set filter by time and log
      setFilterByLogValue(logname);
    }

    this->setProperty("OutputWorkspace", mSplitters);
    this->setProperty("InformationWorkspace", mFilterInfoWS);

    return;
  }

  /** Process the input for time.  A smart but complicated default rule
    * (1) If the time is in unit of second, nanosecond or percent, then it is
    *     relative time to FIRST proton charge, but NOT run_start.
   */
  void GenerateEventsFilter::processInputTime()
  {
    // 1. Get input
    std::string s_inpt0 = this->getProperty("StartTime");
    std::string s_inptf = this->getProperty("StopTime");

    g_log.debug() << "Start time = " << s_inpt0 << endl;
    cout << "Start time = " << s_inpt0 << endl;

    // 2. Run info
    Kernel::TimeSeriesProperty<double>* protonchargelog =
        dynamic_cast<Kernel::TimeSeriesProperty<double> *>(mEventWS->run().getProperty("proton_charge"));
    Kernel::DateAndTime runstarttime = protonchargelog->firstTime();
    Kernel::DateAndTime runendtime = protonchargelog->lastTime();

    // int64_t runtime_ns = runend.totalNanoseconds()-runstarttime.totalNanoseconds();

    // 3. Set up time-convert unit
    std::string timeunit = this->getProperty("UnitOfTime");
    m_convertfactor = 1.0;
    if (timeunit.compare("Seconds") == 0)
    {
      // a) In unit of seconds
      m_convertfactor = 1.0E9;
    }
    else if (timeunit.compare("Nanoseconds") == 0)
    {
      // b) In unit of nano-seconds
      m_convertfactor = 1.0;
    }

    cout << "convert factor = " << m_convertfactor << " unit of time = " << timeunit << endl;

    // 4. Run start time
    if (s_inpt0.size() == 0)
    {
      // Default
      cout << "Use default." << endl;
      mStartTime = runstarttime;
    }
    else
    {
      cout << "Use input " << s_inpt0 << endl;
      bool istimestring = s_inpt0.find(':') == std::string::npos;
      cout << istimestring << endl;
      if (s_inpt0.find(':') != std::string::npos)
      {
        // In string format
        cout << "Branch 1\n";
        Kernel::DateAndTime t0(s_inpt0);
        mStartTime = t0;
      }
      else
      {
        cout << "Branch 2\n";
        // In double (second, nanosecond, or ...)
        double inpt0 = atof(s_inpt0.c_str());
        int64_t temptimens = 0;
        if (timeunit.compare("Percent"))
        {
          // Not in percent
          if (inpt0 >= 0)
          {
            temptimens = runstarttime.totalNanoseconds() + static_cast<int64_t>(inpt0*m_convertfactor);
            cout << "Branch 1A: " << temptimens << endl;
          }
          else
          {
            // Exception
            throw std::invalid_argument("Input StartTime cannot be negative!");
          }
        }
        else
        {
          // In percentage of run time
          int64_t runtime_ns = runendtime.totalNanoseconds()-runstarttime.totalNanoseconds();
          double runtime_ns_dbl = static_cast<double>(runtime_ns);
          temptimens = runstarttime.totalNanoseconds() + static_cast<int64_t>(inpt0*runtime_ns_dbl*0.01);
        }
        Kernel::DateAndTime temptime(temptimens);
        mStartTime = temptime;
      }
    } // ENDIFELSE: Starting time

    cout << "run time start = " << mStartTime.toSimpleString() << "\n";

    // 4. Run end time
    if (s_inptf.size() == 0)
    {
      // Default
      mStopTime = runendtime;
    }
    else
    {
      if (s_inptf.find(':') != std::string::npos)
      {
        // In string format
        Kernel::DateAndTime tf(s_inpt0);
        mStartTime = tf;
      }
      else
      {
        // In double (second, nanosecond, or ...)
        double inptf = atof(s_inptf.c_str());
        int64_t temptimens = 0;
        if (timeunit.compare("Percent"))
        {
          // Not in percent
          if (inptf >= 0)
          {
            temptimens = runstarttime.totalNanoseconds() + static_cast<int64_t>(inptf*m_convertfactor);
          }
          else
          {
            // Exception
            throw std::invalid_argument("Input StartTime cannot be negative!");
          }
        }
        else
        {
          // In percentage of run time
          int64_t runtime_ns = runendtime.totalNanoseconds()-runstarttime.totalNanoseconds();
          double runtime_ns_dbl = static_cast<double>(runtime_ns);
          temptimens = runstarttime.totalNanoseconds() + static_cast<int64_t>(inptf*runtime_ns_dbl*0.01);
        }
        Kernel::DateAndTime temptime(temptimens);
        mStopTime = temptime;
      }
    } // ENDIFELSE: Starting time

    // 5. Validate
    if (mStartTime.totalNanoseconds() >= mStopTime.totalNanoseconds())
    {
      stringstream errmsg;
      errmsg << "Use input starting time " << s_inpt0
             << " is equal or later than stoping time " << s_inptf << ".\n";
      throw runtime_error(errmsg.str());
    }
    else
    {
      g_log.notice() << "Start time = " << mStartTime.totalNanoseconds() << ", \t"
                     << "Stop  time = " << mStopTime.totalNanoseconds() << "\n";
    }

    return;
  }

  /*
   * Set splitters by time value / interval only
   */
  void GenerateEventsFilter::setFilterByTimeOnly()
  {
    double timeinterval = this->getProperty("TimeInterval");

    // Progress
    int64_t totaltime = mStopTime.totalNanoseconds()-mStartTime.totalNanoseconds();
    int64_t timeslot = 0;

    if (timeinterval <= 0.0)
    {
      int wsindex = 0;
      // 1. Default and thus just one interval
      Kernel::SplittingInterval ti(mStartTime, mStopTime, 0);
      mSplitters->addSplitter(ti);

      API::TableRow row = mFilterInfoWS->appendRow();
      std::stringstream ss;
      ss << "Time Interval From " << mStartTime << " to " << mStopTime;
      row << wsindex << ss.str();
    }
    else
    {
      // 2. Use N time interval
      int64_t deltatime_ns = static_cast<int64_t>(timeinterval*m_convertfactor);

      int64_t curtime_ns = mStartTime.totalNanoseconds();
      int wsindex = 0;
      while (curtime_ns < mStopTime.totalNanoseconds())
      {
        // a) Calculate next.time
        int64_t nexttime_ns = curtime_ns + deltatime_ns;
        if (nexttime_ns > mStopTime.totalNanoseconds())
          nexttime_ns = mStopTime.totalNanoseconds();

        // b) Create splitter
        Kernel::DateAndTime t0(curtime_ns);
        Kernel::DateAndTime tf(nexttime_ns);
        Kernel::SplittingInterval spiv(t0, tf, wsindex);
        mSplitters->addSplitter(spiv);

        // c) Information
        API::TableRow row = mFilterInfoWS->appendRow();
        std::stringstream ss;
        ss << "Time Interval From " << t0 << " to " << tf;
        row << wsindex << ss.str();

        // d) Update loop variable
        curtime_ns = nexttime_ns;
        wsindex ++;

        // e) Update progress
        int64_t newtimeslot = (curtime_ns-mStartTime.totalNanoseconds())*90/totaltime;
        if (newtimeslot > timeslot)
        {
          // There is change and update progress
          timeslot = newtimeslot;
          double prog = 0.1+double(timeslot)/100.0;
          progress(prog);
        }

      } // END-WHILE

    } // END-IF-ELSE

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Generate filters by log values.
    */
  void GenerateEventsFilter::setFilterByLogValue(std::string logname)
  {
    // 1. Process inputs
    Kernel::TimeSeriesProperty<double>* mLog =
        dynamic_cast<Kernel::TimeSeriesProperty<double>* >(mEventWS->run().getProperty(logname));
    if (!mLog)
    {
      g_log.error() << "Log " << logname << " does not exist or is not TimeSeriesProperty in double." << std::endl;
      throw std::invalid_argument("User specified log is not correct");
    }

    // a) Clear duplicate value
    mLog->eliminateDuplicates();

    double minvalue = this->getProperty("MinimumLogValue");
    double maxvalue = this->getProperty("MaximumLogValue");
    double deltaValue = this->getProperty("LogValueInterval");

    if (minvalue == EMPTY_DBL())
    {
      minvalue = mLog->minValue();
    }
    if (maxvalue == EMPTY_DBL())
    {
      maxvalue = mLog->maxValue();
    }

    if (minvalue > maxvalue)
    {
      g_log.error() << "Error: Input minimum log value " << minvalue <<
          " is larger than maximum log value " << maxvalue << std::endl;
      throw std::invalid_argument("Input minimum value is larger than maximum value");
    }

    std::string filterdirection = this->getProperty("FilterLogValueByChangingDirection");
    bool filterIncrease;
    bool filterDecrease;
    if (filterdirection.compare("Both") == 0)
    {
      filterIncrease = true;
      filterDecrease = true;
    }
    else if (filterdirection.compare("Increase") == 0)
    {
      filterIncrease = true;
      filterDecrease = false;
    }
    else
    {
      filterIncrease = false;
      filterDecrease = true;
    }

    bool toProcessSingleValueFilter = false;
    if (deltaValue <= 0)
    {
      toProcessSingleValueFilter = true;
    }

    // 2. Generate filters
    if (toProcessSingleValueFilter)
    {
      // a) Generate a filter for a single log value
      processSingleValueFilter(mLog, minvalue, maxvalue, filterIncrease, filterDecrease);
    }
    else
    {
      // b) Generate filters for a series of log value
      processMultipleValueFilters(mLog, minvalue, maxvalue, filterIncrease, filterDecrease);
    }

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Generate filters by single log value
    */
  void GenerateEventsFilter::processSingleValueFilter(Kernel::TimeSeriesProperty<double>* mlog,
                                                      double minvalue, double maxvalue,
                                                      bool filterincrease, bool filterdecrease)
  {
    // 1. Validity & value
    double timetolerance = this->getProperty("TimeTolerance");
    int64_t timetolerance_ns = static_cast<int64_t>(timetolerance*m_convertfactor);

    std::string logboundary = this->getProperty("LogBoundary");

    // 2. Generate filter
    std::vector<Kernel::SplittingInterval> splitters;
    int wsindex = 0;
    makeFilterByValue(mlog, splitters, minvalue, maxvalue, static_cast<double>(timetolerance_ns)*1.0E-9,
                      logboundary.compare("centre")==0,
                      filterincrease, filterdecrease, mStartTime, mStopTime, wsindex);

    // 3. Add to output
    for (size_t isp = 0; isp < splitters.size(); isp ++)
    {
      mSplitters->addSplitter(splitters[isp]);
    }

    // 4. Add information
    API::TableRow row = mFilterInfoWS->appendRow();
    std::stringstream ss;
    ss << "Log " << mlog->name() << " From " << minvalue << " To " << maxvalue << "  Value-change-direction ";
    if (filterincrease && filterdecrease)
    {
      ss << " both ";
    }
    else if (filterincrease)
    {
      ss << " increase";
    }
    else
    {
      ss << " decrease";
    }
    row << 0 << ss.str();

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Generate filters from multiple values
   */
  void GenerateEventsFilter::processMultipleValueFilters(Kernel::TimeSeriesProperty<double>* mlog, double minvalue,
                                                         double maxvalue,
                                                         bool filterincrease, bool filterdecrease)
  {
    // 1. Read more input
    double valueinterval = this->getProperty("LogValueInterval");
    if (valueinterval <= 0)
      throw std::invalid_argument("Multiple values filter must have LogValueInterval larger than ZERO.");
    double valuetolerance = this->getProperty("LogValueTolerance");

    if (valuetolerance == EMPTY_DBL())
      valuetolerance = 0.5*valueinterval;
    else if (valuetolerance < 0.0)
      throw std::runtime_error("LogValueTolerance cannot be less than zero.");

    // 2. Create log value interval (low/up boundary) list and split information workspace
    std::map<size_t, int> indexwsindexmap;
    std::vector<double> logvalueranges;
    int wsindex = 0;
    size_t index = 0;

    double curvalue = minvalue;
    while (curvalue-valuetolerance < maxvalue)
    {
      indexwsindexmap.insert(std::make_pair(index, wsindex));

      // Log interval/value boundary
      double lowbound = curvalue - valuetolerance ;
      double upbound = curvalue + valueinterval - valuetolerance;
      logvalueranges.push_back(lowbound);
      logvalueranges.push_back(upbound);

      // Workgroup information
      std::stringstream ss;
      ss << "Log " << mlog->name() << " From " << lowbound << " To " << upbound << "  Value-change-direction ";
      if (filterincrease && filterdecrease)
      {
        ss << " both ";
      }
      else if (filterincrease)
      {
        ss << " increase";
      }
      else
      {
        ss << " decrease";
      };
      API::TableRow newrow = mFilterInfoWS->appendRow();
      newrow << wsindex << ss.str();

      curvalue += valueinterval;
      wsindex ++;
      ++index;
    } // ENDWHILE

    if (logvalueranges.size() < 2)
    {
      g_log.warning() << "There is no log value interval existing." << std::endl;
      return;
    }

    double upperboundinterval0 = logvalueranges[1];
    double lowerboundlastinterval = logvalueranges[logvalueranges.size()-2];
    double minlogvalue = mlog->minValue();
    double maxlogvalue = mlog->maxValue();
    if (minlogvalue > upperboundinterval0 || maxlogvalue < lowerboundlastinterval)
    {
      g_log.warning() << "User specifies log interval from " << minvalue-valuetolerance
                      << " to " << maxvalue-valuetolerance << " with interval size = " << valueinterval
                      << "; Log " << mlog->name() << " has range " << minlogvalue << " to " << maxlogvalue
                      << ".  Therefore some workgroup index may not have any splitter." << std::endl;
    }

    // 3. Call
    Kernel::TimeSplitterType splitters;
    std::string logboundary = this->getProperty("LogBoundary");

    makeMultipleFiltersByValues(mlog, splitters, indexwsindexmap, logvalueranges,
                                logboundary.compare("centre") == 0,
                                filterincrease, filterdecrease, mStartTime, mStopTime);

    // 4. Put to SplittersWorkspace
    for (size_t i = 0; i < splitters.size(); i ++)
      mSplitters->addSplitter(splitters[i]);

    return;
  }


  //-----------------------------------------------------------------------------------------------
  /**
   * Fill a TimeSplitterType that will filter the events by matching
   * SINGLE log values >= min and < max. Creates SplittingInterval's where
   * times match the log values, and going to index==0.
   *
   * @param mlog :: Log.
   * @param filterIncrease :: As log value increase, and within (min, max), include this range in the filter.
   * @param filterDecrease :: As log value increase, and within (min, max), include this range in the filter.
   * @param startTime :: Start time.
   * @param stopTime :: Stop time.
   * @param wsindex :: Workspace index.
   * @param split :: Splitter that will be filled.
   * @param min :: Min value.
   * @param max :: Max value.
   * @param TimeTolerance :: Offset added to times in seconds.
   * @param centre :: Whether the log value time is considered centred or at the beginning.
   */
  void GenerateEventsFilter::makeFilterByValue(Kernel::TimeSeriesProperty<double>* mlog,
      Kernel::TimeSplitterType& split, double min, double max, double TimeTolerance, bool centre,
      bool filterIncrease, bool filterDecrease, Kernel::DateAndTime startTime, Kernel::DateAndTime stopTime,
      int wsindex)
  {
    // 1. Do nothing if the log is empty.
    if (mlog->size() == 0)
    {
      g_log.warning() << "There is no entry in this property " << this->name() << std::endl;
      return;
    }

    // 2. Do the rest
    bool lastGood = false;
    bool isGood = false;;
    time_duration tol = DateAndTime::durationFromSeconds( TimeTolerance );
    int numgood = 0;
    DateAndTime lastTime, t;
    DateAndTime start, stop;

    size_t progslot = 0;

    for (int i = 0; i < mlog->size(); i ++)
    {
      lastTime = t;
      //The new entry
      t = mlog->nthTime(i);
      double val = mlog->nthValue(i);

      // A good value?
      if (filterIncrease && filterDecrease)
      {
        // a) Including both sides
        isGood = ((val >= min) && (val < max)) && t >= startTime && t <= stopTime;
      }
      else if (filterIncrease)
      {
        if (i == 0)
          isGood = false;
        else
          isGood = ((val >= min) && (val < max)) && t >= startTime && t <= stopTime && val-mlog->nthValue(i-1) > 0;
      }
      else if (filterDecrease)
      {
        if (i == 0)
          isGood = false;
        else
          isGood = ((val >= min) && (val < max)) && t >= startTime && t <= stopTime && val-mlog->nthValue(i-1) < 0;
      }
      else
      {
        g_log.error() << "Neither increasing nor decreasing is selected.  It is empty!" << std::endl;
      }

      if (isGood)
        numgood++;

      if (isGood != lastGood)
      {
        //We switched from bad to good or good to bad

        if (isGood)
        {
          //Start of a good section
          if (centre)
            start = t - tol;
          else
            start = t;
        }
        else
        {
          //End of the good section
          if (centre)
          {
            stop = t - tol;
          }
          else
          {
            stop = t;
          }
          split.push_back( SplittingInterval(start, stop, wsindex) );

          /*
          if (numgood == 1)
          {
            //There was only one point with the value. Use the last time, - the tolerance, as the end time
            if (centre)
            {
              stop = t-tol;
              // stop = lastTime - tol;
            }
            else
            {
              stop = t;
            }
            split.push_back( SplittingInterval(start, stop, wsindex) );
          }
          else
          {
            //At least 2 good values. Save the end time
            XXX XXX
          }
          */

          //Reset the number of good ones, for next time
          numgood = 0;
        }
        lastGood = isGood;
      }

      // Progress bar..
      size_t tmpslot = i*90/mlog->size();
      if (tmpslot > progslot)
      {
        progslot = tmpslot;
        double prog = double(progslot)/100.0+0.1;
        progress(prog);
      }

    } // ENDFOR

    if (numgood > 0)
    {
      //The log ended on "good" so we need to close it using the last time we found
      if (centre)
        stop = t - tol;
      else
        stop = t;
      split.push_back( SplittingInterval(start, stop, wsindex) );
      numgood = 0;
    }

    return;
  }

  //-----------------------------------------------------------------------------------------------
  /** Fill a TimeSplitterType that will filter the events by matching
   * SINGLE log values >= min and < max. Creates SplittingInterval's where
   * times match the log values, and going to index==0.
   *
   * @param mlog :: Log.
   * @param split :: Splitter that will be filled.
   * @param indexwsindexmap :: Index.
   * @param logvalueranges ::  A vector of double. Each 2i and 2i+1 pair is one individual log value range.
   * @param centre :: Whether the log value time is considered centred or at the beginning.
   * @param filterIncrease :: As log value increase, and within (min, max), include this range in the filter.
   * @param filterDecrease :: As log value increase, and within (min, max), include this range in the filter.
   * @param startTime :: Start time.
   * @param stopTime :: Stop time.
   */
  void GenerateEventsFilter::makeMultipleFiltersByValues(Kernel::TimeSeriesProperty<double>* mlog,
      Kernel::TimeSplitterType& split, std::map<size_t, int> indexwsindexmap, std::vector<double> logvalueranges,
      bool centre, bool filterIncrease, bool filterDecrease, Kernel::DateAndTime startTime, Kernel::DateAndTime stopTime)
  {
    // 0. Set up
    double timetolerance = 0.0;
    if (centre)
    {
      timetolerance = this->getProperty("TimeTolerance");
    }
    time_duration tol = DateAndTime::durationFromSeconds( timetolerance );

    // 1. Do nothing if the log is empty.
    if (mlog->size() == 0)
    {
      g_log.warning() << "There is no entry in this property " << mlog->name() << std::endl;
      return;
    }

    // 2. Go through the whole log to set up time intervals
    Kernel::DateAndTime ZeroTime(0);
    int lastindex = -1;
    int currindex = -1;
    DateAndTime lastTime, currTime;
    DateAndTime start, stop;
    double currValue = 0.0;
    size_t progslot = 0;

    int logsize = mlog->size();

    for (int i = 0; i < logsize; i ++)
    {
      // a) Initialize status flags and new entry
      lastTime = currTime;  // for loop i, currTime is not defined.
      bool breakloop = false;
      bool completehalf = false;
      bool newsplitter = false;

      currTime = mlog->nthTime(i);
      currValue = mlog->nthValue(i);

      // b) Filter out by time and direction (optional)
      bool intime = false;
      if (currTime < startTime)
      {
        // case i.  Too early, do nothing
        completehalf = false;
      }
      else if (currTime > stopTime)
      {
        // case ii. Too later.  Put to splitter if half of splitter is done.  But still within range
        breakloop = true;
        stop = currTime;
        if (start.totalNanoseconds() > 0)
        {
          completehalf = true;
        }
      }
      else
      {
        // case iii. In the range to generate filters
        intime = true;
      }

      // c) Filter in time
      if (intime)
      {
        // c1) Determine direction
        bool correctdir = true;

        if (filterIncrease && filterDecrease)
        {
          // Both direction is fine
          correctdir = true;
        }
        else
        {
          // Filter out one direction
          int direction = 0;
          if ( mlog->nthValue(i)-mlog->nthValue(i-1) > 0)
            direction = 1;
          else
            direction = -1;
          if (filterIncrease && direction > 0)
            correctdir = true;
          else if (filterDecrease && direction < 0)
            correctdir = true;
          else
            correctdir = false;

          // Condition to generate a Splitter (close parenthesis)
          if (!correctdir && start.totalNanoseconds() > 0)
          {
            stop = currTime;
            completehalf = true;
          }
        } // END-IF-ELSE: Direction

        // c2) See whether this value falls into any range
        if (correctdir)
        {
          size_t index = searchValue(logvalueranges, currValue);
          g_log.debug() << "DBx257 Examine Log Index " << i << ", Value = " << currValue
                        << ", Data Range Index = " << index
                        << "/Group Index = " << indexwsindexmap[index/2] << std::endl;

          bool valuewithin2boundaries = true;
          if (index > logvalueranges.size())
          {
            // Out of range
            valuewithin2boundaries = false;
          }

          if (index%2 == 0 && valuewithin2boundaries)
          {
            // c1) Falls in the interval
            currindex = indexwsindexmap[index/2];

            if (currindex != lastindex && start.totalNanoseconds() == 0)
            {
              // i.   A new region!
              newsplitter = true;
            }
            else if (currindex != lastindex && start.totalNanoseconds() > 0)
            {
              // ii.  Time to close a region and new a region
              stop = currTime;
              completehalf = true;
              newsplitter = true;
            }
            else if (currindex == lastindex && start.totalNanoseconds() > 0)
            {
              // iii. Still in the same zone
              if (i == logsize-1)
              {
                // Last entry in the log.  Need to flag to close the pair
                stop = currTime;
                completehalf = true;
                newsplitter = false;
              }
              else
              {
                // Do nothing
                ;
              }
            }
            else
            {
              // iv.  It is impossible
              std::stringstream errmsg;
              errmsg << "Impossible to have currindex == lastindex == " << currindex
                     << ", while start is not init.  Log Index = " << i << "\t value = "
                     << currValue << "\t, Index = " << index
                     << " in range " << logvalueranges[index] << ", " << logvalueranges[index+1];

              g_log.error(errmsg.str());
              throw std::runtime_error(errmsg.str());
            }
          }
          else if (valuewithin2boundaries)
          {
            // Out of a range.
            if (start.totalNanoseconds() > 0)
            {
              // End situation
              stop = currTime;
              completehalf = true;
            }
            else
            {
              // No operation required
              ;
            }

            // c2) Fall out of interval
            g_log.debug() << "DBOP Log Index " << i << "  Falls Out b/c value range... " << std::endl;
          }
          else
          {
            // log value falls out of min/max: do nothing
            ;
          }
        } // ENDIF NO breakloop AND Correction Direction
        else
        {
          g_log.debug() << "DBOP Log Index " << i << " Falls out b/c out of wrong direction" << std::endl;
        }
      }
      else
      {
        // Wrong direction
        g_log.debug() << "DBOP Log Index " << i << "  Falls Out b/c out of time range... " << std::endl;
      }

      // d) Create Splitter
      if (completehalf)
      {
        if (centre)
        {
          split.push_back( SplittingInterval(start-tol, stop-tol, lastindex) );
        }
        else
        {
          split.push_back( SplittingInterval(start, stop, lastindex) );
        }
        g_log.debug() << "DBx250 Add Splitter " << split.size()-1 << ":  " << start.totalNanoseconds() << ", "
                      << stop.totalNanoseconds() << ", Delta T = "
                      << static_cast<double>(stop.totalNanoseconds()-start.totalNanoseconds())*1.0E-9
                      << "(s), Workgroup = " << lastindex << std::endl;

        // reset
        start = ZeroTime;
      }

      // e) Start new splitter: have to be here due to start cannot be updated before a possible splitter generated
      if (newsplitter)
      {
        start = currTime;
      }

      // f) Break
      if (breakloop)
        break;

      // e) Update loop variable
      lastindex = currindex;

      // f) Progress
      // Progress bar..
      size_t tmpslot = i*90/mlog->size();
      if (tmpslot > progslot)
      {
        progslot = tmpslot;
        double prog = double(progslot)/100.0+0.1;
        progress(prog);
      }

    } // For each log value

    progress(1.0);

    return;

  }

  //----------------------------------------------------------------------------------------------
  /** Do a binary search in the following list
   * Warning: if the vector is not sorted, the error will happen.
   * This algorithm won't guarantee for it
   *
   * @param sorteddata :: Sorted data.
   * @param value :: Value to look up.
   *
   * return:  if value is out of range, then return datarange.size() + 1
   */
  size_t GenerateEventsFilter::searchValue(std::vector<double> sorteddata, double value)
  {
    size_t outrange = sorteddata.size()+1;

    // std::cout << "DB450  Search Value " << value << std::endl;

    // 1. Extreme case
    if (value < sorteddata[0] || value > sorteddata.back())
      return outrange;
    if (sorteddata.size() == 0)
      return outrange;

    // 2. Binary search
    bool found = false;
    size_t start = 0;
    size_t stop = sorteddata.size()-1;

    while (!found)
    {
      if (start == stop || start+1 == stop)
      {
        // a) Found
        if (value == sorteddata[stop])
        {
          // std::cout << "DB450  Found @ A " << dataranges[stop] << "  Index = " << stop << std::endl;
          return stop;
        }
        else
        {
          // std::cout << "DB450  Found @ B " << dataranges[start] << "  Index = " << start << std::endl;
          return start;
        }
      }

      size_t mid = (start+stop)/2;
      if (value < sorteddata[mid])
      {
        stop = mid;
      }
      else
      {
        start = mid;
      }
    }

    return 0;
  }

} // namespace Mantid
} // namespace Algorithms

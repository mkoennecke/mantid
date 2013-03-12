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
#include "MantidKernel/EnabledWhenProperty.h"
#include "MantidKernel/VisibleWhenProperty.h"
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

    declareProperty("LogName", "",
        "Name of the sample log to use to filter. \n"
        "If left empty, then algorithm will do filter by log value.");

    declareProperty("Interval", EMPTY_DBL(), "Time interval in case filtering by time, or log "
                    "interval if filtering by log value.");

    // Time
    declareProperty("StartTime", "",
        "The start time, in (a) seconds, (b) nanoseconds or (c) percentage of total run time\n"
        "since the start of the run. OR (d) absolute time. \n"
        "Events before this time are filtered out. Default is the first time of proton charge. ");

    declareProperty("StopTime", "",
        "The stop time, in (2) seconds, (b) nanoseconds or (c) percentage of total run time\n"
        "since the start of the run. OR (d) absolute time. \n"
        "Events at or after this time are filtered out. Default is the last time of proton charge.");
    std::vector<std::string> timeoptions;
    timeoptions.push_back("Seconds");
    timeoptions.push_back("Nanoseconds");
    timeoptions.push_back("Percent");
    declareProperty("UnitOfTime", "Seconds", boost::make_shared<Kernel::StringListValidator>(timeoptions),
                    "StartTime, StopTime and DeltaTime can be given in various unit."
                    "The unit can be second or nanosecond from run start time."
                    "They can also be defined as percentage of total run time.");

    // Log value
    declareProperty("MinimumLogValue", EMPTY_DBL(), "Minimum log value for which to keep events.");
    this->setPropertySettings("MinimumLogValue",
                              new VisibleWhenProperty("LogName", IS_NOT_EQUAL_TO,  ""));

    declareProperty("MaximumLogValue", EMPTY_DBL(), "Maximum log value for which to keep events.");

    std::vector<std::string> filteroptions;
    filteroptions.push_back("Both");
    filteroptions.push_back("Increase");
    filteroptions.push_back("Decrease");
    declareProperty("FilterLogValueByChangingDirection", "Both",
                    boost::make_shared<Kernel::StringListValidator>(filteroptions),
                    "d(log value)/dt can be positive and negative.  They can be put to different splitters.");

    declareProperty("TimeTolerance", 0.0,
        "Tolerance in time for the event times to keep. It is used in the case to filter by single value.");

    declareProperty("LogValueTolerance", EMPTY_DBL(),
        "Tolerance of the log value to be included in filter.  It is used in the case to filter by multiple values.");

    vector<string> boundaryoptions;
    boundaryoptions.push_back("Centre");
    boundaryoptions.push_back("Edge");
    declareProperty("LogBoundary", "Centre", boost::make_shared<StringListValidator>(boundaryoptions),
        "How to treat log values as being measured in the centre of time.");

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

    // 2. Run info
    Kernel::TimeSeriesProperty<double>* protonchargelog =
        dynamic_cast<Kernel::TimeSeriesProperty<double> *>(mEventWS->run().getProperty("proton_charge"));
    Kernel::DateAndTime runstarttime = protonchargelog->firstTime();
    Kernel::DateAndTime runendtime = protonchargelog->lastTime();

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

    // 4. Run start time
    if (s_inpt0.size() == 0)
    {
      // Default
      mStartTime = runstarttime;
    }
    else
    {
      if (s_inpt0.find(':') != std::string::npos)
      {
        // In string format
        Kernel::DateAndTime t0(s_inpt0);
        mStartTime = t0;
      }
      else
      {
        // In double (second, nanosecond, or ...)
        double inpt0 = atof(s_inpt0.c_str());
        int64_t temptimens = 0;
        if (timeunit.compare("Percent"))
        {
          // Not in percent
          if (inpt0 >= 0)
          {
            temptimens = runstarttime.totalNanoseconds() + static_cast<int64_t>(inpt0*m_convertfactor);
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
        Kernel::DateAndTime tf(s_inptf);
        mStopTime = tf;
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
      errmsg << "Use input starting time " << s_inpt0 << " / " << mStartTime.toSimpleString()
             << " is equal or later than stoping time " << s_inptf << "/ " << mStopTime.toSimpleString() << ".\n";
      throw runtime_error(errmsg.str());
    }
    else
    {
      g_log.information() << "Start time = " << mStartTime.totalNanoseconds() << ", \t"
                          << "Stop  time = " << mStopTime.totalNanoseconds() << "\n";
    }

    return;
  }

  /** Set splitters by time value / interval only
   */
  void GenerateEventsFilter::setFilterByTimeOnly()
  {
    double timeinterval = this->getProperty("Interval");

    // Progress
    int64_t totaltime = mStopTime.totalNanoseconds()-mStartTime.totalNanoseconds();
    int64_t timeslot = 0;

    if (timeinterval <= 0.0 || timeinterval == EMPTY_DBL())
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
    Kernel::TimeSeriesProperty<double>* logtofilter =
        dynamic_cast<Kernel::TimeSeriesProperty<double>* >(mEventWS->run().getProperty(logname));
    if (!logtofilter)
    {
      g_log.error() << "Log " << logname << " does not exist or is not TimeSeriesProperty in double." << std::endl;
      throw std::invalid_argument("User specified log is not correct");
    }

    // a) Clear duplicate value
    logtofilter->eliminateDuplicates();

    double minvalue = this->getProperty("MinimumLogValue");
    double maxvalue = this->getProperty("MaximumLogValue");

    if (minvalue == EMPTY_DBL())
    {
      minvalue = logtofilter->minValue();
    }
    if (maxvalue == EMPTY_DBL())
    {
      maxvalue = logtofilter->maxValue();
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

    processMultipleValueFilters(logtofilter, minvalue, maxvalue, filterIncrease, filterDecrease);

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Generate filters from multiple values
    * @param logtofilter:  the time series log to filter with
    * @param minvalue: minimum log value
    * @param maxvalue: maximum log value
    * @param filterincrease: include events corresponding to the increasing log value
    * @param filterdecrease: include events corresponding to the decreasing log value
   */
  void GenerateEventsFilter::processMultipleValueFilters(Kernel::TimeSeriesProperty<double>* logtofilter, double minvalue,
                                                         double maxvalue, bool filterincrease, bool filterdecrease)
  {
    // 1. Process input
    double valueinterval = this->getProperty("Interval");
    if (valueinterval <= 0)
    {
      // Invalid
      throw runtime_error("Multiple values filter must have (log value) Interval larger than ZERO.");
    }

    double valuetolerance = this->getProperty("LogValueTolerance");
    if (valuetolerance == EMPTY_DBL())
    {
      // Default
      valuetolerance = 0.5*valueinterval;
    }
    else if (valuetolerance < 0.0)
    {
      // Invalid
      throw runtime_error("LogValueTolerance cannot be less than zero.");
    }

    // Default log interval: guarantee there is one and only one interval. replacing single value filter
    if (valueinterval == EMPTY_DBL())
    {
      valueinterval = 2.0*(maxvalue - minvalue);
      valuetolerance = 0.0;
    }

    // 2. Create log value interval (low/up boundary) list and split information workspace
    std::map<size_t, int> indexwsindexmap;
    std::vector<double> logvalueranges;
    int wsindex = 0;
    size_t rangeindex = 0;

    double curvalue = minvalue;
    while (curvalue-valuetolerance < maxvalue)
    {
      indexwsindexmap.insert(std::make_pair(rangeindex, wsindex));

      // Log interval/value boundary
      double lowbound = curvalue - valuetolerance ;
      if (lowbound < minvalue)
      {
        lowbound = minvalue;
      }

      double upbound = curvalue + valueinterval - valuetolerance;
      if (upbound > maxvalue)
        upbound = maxvalue;

      logvalueranges.push_back(lowbound);
      logvalueranges.push_back(upbound);

      // Workgroup information
      std::stringstream ss;
      ss << "Log " << logtofilter->name() << " From " << lowbound << " To " << upbound << "  Value-change-direction ";
      if (filterincrease && filterdecrease)
      {
        ss << " Both ";
      }
      else if (filterincrease)
      {
        ss << " Increase";
      }
      else
      {
        ss << " Decrease";
      }
      ss << ".  Workspace-index = " << wsindex;
      API::TableRow newrow = mFilterInfoWS->appendRow();
      newrow << wsindex << ss.str();

      g_log.debug() << "Add filter range " << rangeindex << ": " << ss.str() << "\n";

      // Update loop variables
      curvalue += valueinterval;
      ++ wsindex;
      ++ rangeindex;
    } // ENDWHILE

    if (logvalueranges.size() < 2)
    {
      g_log.warning() << "There is no log value interval existing." << std::endl;
      return;
    }

    double upperboundinterval0 = logvalueranges[1];
    double lowerboundlastinterval = logvalueranges[logvalueranges.size()-2];
    double minlogvalue = logtofilter->minValue();
    double maxlogvalue = logtofilter->maxValue();
    if (minlogvalue > upperboundinterval0 || maxlogvalue < lowerboundlastinterval)
    {
      g_log.warning() << "User specifies log interval from " << minvalue-valuetolerance
                      << " to " << maxvalue-valuetolerance << " with interval size = " << valueinterval
                      << "; Log " << logtofilter->name() << " has range " << minlogvalue << " to " << maxlogvalue
                      << ".  Therefore some workgroup index may not have any splitter." << std::endl;
    }

    // 3. Call
    Kernel::TimeSplitterType splitters;
    std::string logboundary = this->getProperty("LogBoundary");

    makeMultipleFiltersByValues(logtofilter, splitters, indexwsindexmap, logvalueranges,
                                logboundary.compare("Centre") == 0,
                                filterincrease, filterdecrease, mStartTime, mStopTime);

    // 4. Put to SplittersWorkspace
    for (size_t i = 0; i < splitters.size(); i ++)
      mSplitters->addSplitter(splitters[i]);

    return;
  }

  //-----------------------------------------------------------------------------------------------
  /** Fill a TimeSplitterType that will filter the events by matching
   * SINGLE log values >= min and < max. Creates SplittingInterval's where
   * times match the log values, and going to index==0.
   *
   * @param mlog :: Log.
   * @param split :: (In/Out) Splitter that will be filled.
   * @param indexwsindexmap :: Index.
   * @param logvalueranges ::  A vector of double. Each 2i and 2i+1 pair is one individual log value range.
   * @param centre :: Whether the log value time is considered centred or at the beginning.
   * @param filterIncrease :: As log value increase, and within (min, max), include this range in the filter.
   * @param filterDecrease :: As log value increase, and within (min, max), include this range in the filter.
   * @param startTime :: Start time.
   * @param stopTime :: Stop time.
   */
  void GenerateEventsFilter::makeMultipleFiltersByValues(Kernel::TimeSeriesProperty<double>* mlog,
                                                         Kernel::TimeSplitterType& split, std::map<size_t, int> indexwsindexmap,
                                                         std::vector<double> logvalueranges,
                                                         bool centre, bool filterIncrease, bool filterDecrease,
                                                         Kernel::DateAndTime startTime, Kernel::DateAndTime stopTime)
  {
    // 1. Set up
    double timetolerance = 0.0;
    if (centre)
    {
      timetolerance = this->getProperty("TimeTolerance");
    }
    time_duration tol = DateAndTime::durationFromSeconds( timetolerance );

    // 2. Return w/o doing anything, if the log is empty.
    if (mlog->size() == 0)
    {
      g_log.warning() << "There is no entry in this property " << mlog->name() << std::endl;
      return;
    }

    // 3. Go through the whole log to set up time intervals
    //    Use start.totalNanoseconds as the flag for in splitter or out of splitter
    Kernel::DateAndTime ZeroTime(0);
    DateAndTime lastTime, currTime;
    DateAndTime start, stop;
    double currValue = 0.0;
    size_t progslot = 0;

    int currdatarangeindex = -1;
    int prevdatarangeindex = -1;

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
      if (currTime < startTime)
      {
        // case i.  Too early, do nothing
        completehalf = false;
      }
      else if (currTime > stopTime)
      {
        // case ii. Too later.  Put to splitter if half of splitter is done.  But still within range
        breakloop = true;
        if (start.totalNanoseconds() > 0)
        {
          completehalf = true;
        }
      }
      else
      {
        // case iii. In the range to generate filters
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

        } // END-IF-ELSE: Direction

        // c2) See whether this value falls into any range
        if (!correctdir && start.totalNanoseconds() > 0)
        {
          // Condition to generate a Splitter (close parenthesis)
          stop = currTime;
          completehalf = true;
        }
        else if (correctdir)
        {
          // Check log values to determine
          prevdatarangeindex = currdatarangeindex;
          currdatarangeindex = searchValue(logvalueranges, currValue);

          stringstream dbmsg;
          dbmsg << "DBx257 Examine Log Index " << i << ", Value = " << currValue
                << ", Current data range index   = " << currdatarangeindex
                << ", Previous data arange index = " << prevdatarangeindex
                << ", Start Time = " << start.totalNanoseconds();
          g_log.debug(dbmsg.str());

          bool valuewithin2boundaries = currdatarangeindex < static_cast<int>(logvalueranges.size());

          if (currdatarangeindex%2 == 0 && valuewithin2boundaries)
          {
            // i) Falls into allowed data interval
            if (currdatarangeindex != prevdatarangeindex)
            {
              // ia) A new region. and might close the old parenthesis
              newsplitter = true;
              // cout << "DB1007A: New splitter is to be started.\n";
              if (start.totalNanoseconds() > 0)
              {
                completehalf = true;
                // cout << "DB1007B: Existing splitter is to generate. \n";
              }
              else
              {
                completehalf = false;
                // cout << "DB1007C: No half-generated splitter does exist.\n";
              }
            }
            else
            {
              // ib) Same as previous one. do nothing
              ;
            }
          }
          else
          {
            // Out to between 2 boundaries or out of boundary
            if (start.totalNanoseconds() > 0)
            {
              completehalf = true;
              // cout << "DB1007D: Outside of boundary/interval.  Complete a splitter.\n";
            }
            else
            {
              completehalf = false;
              // cout << "DB1007D: Outside of boundary/interval.  Do nothing.\n";
            }
          }
        } // ENDIF... Direction
      } // ENDIF... Time range

      // d) Create Splitter
      if (completehalf)
      {
        size_t dataindex = static_cast<size_t>(prevdatarangeindex/2);
        stop = currTime;

        map<size_t, int>::iterator mapiter = indexwsindexmap.find(dataindex);
        int wsindex = 0;
        if (mapiter == indexwsindexmap.end())
          throw runtime_error("Impossible to have a section index with no workspace index in pair.");
        else
        {
          wsindex = mapiter->second;
        }

        if (centre)
        {
          split.push_back( SplittingInterval(start-tol, stop-tol, wsindex) );
        }
        else
        {
          split.push_back( SplittingInterval(start, stop, wsindex) );
        }
        stringstream msgx;
        msgx << "DBx250 Add Splitter " << split.size()-1 << ":  " << start.totalNanoseconds() << ", "
             << stop.totalNanoseconds() << ", Delta T = "
             << static_cast<double>(stop.totalNanoseconds()-start.totalNanoseconds())*1.0E-9
             << "(s), Workgroup = " << wsindex;
        g_log.debug(msgx.str());
        // cout << msgx.str() << "\n";

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

    // 1. Extreme case
    if (value < sorteddata[0] || value > sorteddata.back())
      return outrange;
    if (sorteddata.size() == 0)
      return outrange;

    // 2. Binary search
    vector<double>::iterator fiter = lower_bound(sorteddata.begin(), sorteddata.end(), value);
    size_t findex = static_cast<size_t>(fiter-sorteddata.begin());
    if (findex >= sorteddata.size())
      throw runtime_error("This situation is weird. ");
    if (findex >= 1)
      findex -= 1;

    return findex;
  }

} // namespace Mantid
} // namespace Algorithms

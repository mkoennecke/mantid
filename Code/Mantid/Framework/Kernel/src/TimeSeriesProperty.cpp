//
#include "MantidKernel/TimeSeriesProperty.h"
#include "MantidKernel/Exception.h"


namespace Mantid
{
  namespace Kernel
  {

    /**
     * Constructor
     *  @param name :: The name to assign to the property
     */
    template <typename TYPE>
    TimeSeriesProperty<TYPE>::TimeSeriesProperty(const std::string &name) :
    Property(name, typeid(std::vector<TimeValueUnit<TYPE> >)), mP(), m_size(), mPropSortedFlag(), mFilterApplied()
    {
    }

    /// Virtual destructor
    template <typename TYPE>
    TimeSeriesProperty<TYPE>::~TimeSeriesProperty()
    {
    }

    /**
     * "Virtual" copy constructor
     */
    template <typename TYPE>
    TimeSeriesProperty<TYPE>* TimeSeriesProperty<TYPE>::clone() const
    {
      return new TimeSeriesProperty<TYPE>(*this);
    }

    /**
     * Return the memory used by the property, in bytes
     * */
    template <typename TYPE>
    size_t TimeSeriesProperty<TYPE>::getMemorySize() const
    {
      //Rough estimate
      return mP.size() * (sizeof(TYPE) + sizeof(DateAndTime));
    }

    /**
     * Just returns the property (*this) unless overridden
     *  @param rhs a property that is merged in some descendent classes
     *  @return a property with the value
     */
    template <typename TYPE>
    TimeSeriesProperty<TYPE>& TimeSeriesProperty<TYPE>::merge(Property * rhs)
    {
      return operator+=(rhs);
    }

    /**
     * Add the value of another property
     * @param right the property to add
     * @return the sum
     */
    template <typename TYPE>
    TimeSeriesProperty<TYPE>& TimeSeriesProperty<TYPE>::operator+=( Property const * right )
    {
      TimeSeriesProperty<TYPE> const * rhs = dynamic_cast< TimeSeriesProperty<TYPE> const * >(right);

      if (rhs)
      {
        if (this->operator!=(*rhs))
        {
          mP.insert(mP.end(), rhs->mP.begin(), rhs->mP.end());
          mPropSortedFlag = false;
        }
        else
        {
          // Do nothing if appending yourself to yourself. The net result would be the same anyway
          ;
        }

        //Count the REAL size.
        m_size = static_cast<int>(mP.size());

      }
      else
        g_log.warning() << "TimeSeriesProperty " << this->name() << " could not be added to another property of the same name but incompatible type.\n";

      return *this;
    }

    /**
     * Deep comparison.
     * @param right The other property to compare to.
     * @return true if the are equal.
     */
    template<typename TYPE>
    bool TimeSeriesProperty<TYPE>::operator==( const TimeSeriesProperty<TYPE> & right ) const
    {
      sort();

      if (this->name() != right.name()) // should this be done?
      {
        return false;
      }

      if (this->m_size != right.m_size)
      {
        return false;
      }

      { // so vectors can go out of scope
        std::vector<DateAndTime> lhsTimes = this->timesAsVector();
        std::vector<DateAndTime> rhsTimes = right.timesAsVector();
        if (!std::equal(lhsTimes.begin(), lhsTimes.end(), rhsTimes.begin()))
        {
          return false;
        }
      }

      {
        // so vectors can go out of scope
        std::vector<TYPE> lhsValues = this->valuesAsVector();
        std::vector<TYPE> rhsValues = right.valuesAsVector();
        if (!std::equal(lhsValues.begin(), lhsValues.end(), rhsValues.begin()))
        {
          return false;
        }
      }

      return true;
    }

    /**
     * Deep comparison (not equal).
     * @param right The other property to compare to.
     * @return true if the are not equal.
     */
    template <typename TYPE>
    bool TimeSeriesProperty<TYPE>::operator!=(const TimeSeriesProperty<TYPE> & right ) const
    {
      return !(*this == right);
    }

    /*
     * Set name of the property
     */
    template <typename TYPE>
    void TimeSeriesProperty<TYPE>::setName(const std::string name)
    {
      m_name = name;
    }

    /**
     * Filter out a run by time. Takes out any TimeSeriesProperty log entries outside of the given
     *  absolute time range.
     * EXCEPTION: If there is only one entry in the list, it is considered to mean
     * "constant" so the value is kept even if the time is outside the range.
     *
     * @param start :: Absolute start time. Any log entries at times >= to this time are kept.
     * @param stop  :: Absolute stop time. Any log entries at times < than this time are kept.
     */
    template <typename TYPE>
    void TimeSeriesProperty<TYPE>::filterByTime(const Kernel::DateAndTime start, const Kernel::DateAndTime stop)
    {
      // 0. Sort
      sort();

      // 1. Do nothing for single (constant) value
      if (mP.size() <= 1)
        return;

      // 2. Determine index for start and remove  Note erase is [...)
      int istart = this->findIndex(start);
      if (mP[istart].time() != start)
      {
        // increment by 1 to insure mP[istart] be deleted
        istart ++;
      }
      typename std::vector<TimeValueUnit<TYPE> >::iterator iterhead;
      iterhead = mP.begin()+istart;
      mP.erase(mP.begin(), iterhead);

      // 3. Determine index for end and remove  Note erase is [...)
      int iend = this->findIndex(stop);
      if (static_cast<size_t>(iend) < mP.size())
      {
        // Delete from [iend to mp.end)
        iterhead = mP.begin() + iend;
        mP.erase(iterhead, mP.end());
      }

      m_size = static_cast<int>(mP.size());
    }


    /**
     * Filter by a range of times. If current property has a single value it remains unaffected
     * @param splittervec :: A list of intervals to split filter on
     */
    template <typename TYPE>
    void TimeSeriesProperty<TYPE>::filterByTimes(const Kernel::TimeSplitterType & splittervec)
    {
      // 1. Sort
      sort();

      // 2. Return for single value
      if (mP.size() <= 1)
      {
        return;
      }

      // 3. Prepare a copy
      std::vector<TimeValueUnit<TYPE> > mp_copy;

      g_log.debug() << "DB541  mp_copy Size = " << mp_copy.size() << "  Original MP Size = " << mP.size() << std::endl;

      // 4. Create new
      for (size_t isp = 0; isp < splittervec.size(); ++isp)
      {
        Kernel::SplittingInterval splitter = splittervec[isp];
        Kernel::DateAndTime t_start = splitter.start();
        Kernel::DateAndTime t_stop = splitter.stop();

        int tstartindex = findIndex(t_start);
        if (tstartindex < 0)
        {
          // The splitter is not well defined, and use the first
          tstartindex = 0;
        }
        else if (tstartindex >= int(mP.size()))
        {
          // The splitter is not well defined, adn use the last
          tstartindex = int(mP.size())-1;
        }

        int tstopindex = findIndex(t_stop);

        if (tstopindex < 0)
        {
          tstopindex = 0;
        }
        else if (tstopindex >= int(mP.size()))
        {
          tstopindex = int(mP.size())-1;
        }
        else
        {
          if (t_stop == mP[size_t(tstopindex)].time() && size_t(tstopindex) > 0)
          {
            tstopindex --;
          }
        }

        /* Check */
        if (tstartindex < 0 || tstopindex >= int(mP.size()))
        {
          g_log.warning() << "Memory Leak In SplitbyTime!" << std::endl;
        }

        if (tstartindex == tstopindex)
        {
          TimeValueUnit<TYPE> temp(t_start, mP[tstartindex].value());
          mp_copy.push_back(temp);
        }
        else
        {
          mp_copy.push_back(TimeValueUnit<TYPE>(t_start, mP[tstartindex].value()));
          for (size_t im = size_t(tstartindex+1); im <= size_t(tstopindex); ++im)
          {
            mp_copy.push_back(TimeValueUnit<TYPE>(mP[im].time(), mP[im].value()));
          }
        }
      } // ENDFOR

      g_log.debug() << "DB530  Filtered Log Size = " << mp_copy.size() << "  Original Log Size = " << mP.size() << std::endl;

      // 5. Clear
      mP.clear();
      mP = mp_copy;
      mp_copy.clear();

      m_size = static_cast<int>(mP.size());

      return;
    }

    /**
     * Split out a time series property by time intervals.
     *
     * NOTE: If the input TSP has a single value, it is assumed to be a constant
     *  and so is not split, but simply copied to all outputs.
     *
     * @param splitter :: a TimeSplitterType object containing the list of intervals and destinations.
     * @param outputs  :: A vector of output TimeSeriesProperty pointers of the same type.
     */
    template <typename TYPE>
    void TimeSeriesProperty<TYPE>::splitByTime(TimeSplitterType& splitter, std::vector< Property * > outputs) const
    {
      // 0. Sort if necessary
      sort();

      size_t numOutputs = outputs.size();
      if (numOutputs <= 0)
        return;

      std::vector< TimeSeriesProperty<TYPE> *> outputs_tsp;

      // 1. Clear the outputs before you start
      for (size_t i=0; i < numOutputs; i++)
      {
        TimeSeriesProperty<TYPE> * myOutput = dynamic_cast< TimeSeriesProperty<TYPE> * >(outputs[i]);
        if (myOutput)
        {
          outputs_tsp.push_back(myOutput);
          if (this->mP.size() == 1)
          {
            // Special case for TSP with a single entry = just copy.
            myOutput->mP = this->mP;
            myOutput->m_size = 1;
          }
          else
          {
            myOutput->mP.clear();
            myOutput->m_size=0;
          }
        }
        else
        {
          outputs_tsp.push_back( NULL );
        }
      }

      // 2. Special case for TSP with a single entry = just copy.
      if (this->mP.size() == 1)
        return;

      // 3. We will be iterating through all the entries in the the map/vector
      size_t ip = 0;

      //    And at the same time, iterate through the splitter
      Kernel::TimeSplitterType::iterator itspl = splitter.begin();

      //Info of each splitter
      DateAndTime start, stop;
      int index;

      while (itspl != splitter.end())
      {
        //Get the splitting interval times and destination
        start = itspl->start();
        stop = itspl->stop();
        index = itspl->index();

        // Skip the events before the start of the time
        // TODO  Algorithm here can be refactored for better performance
        while (ip < this->mP.size() && mP[ip].time() < start)
          ip ++;

        //Go through all the events that are in the interval (if any)
        // while ((it != this->m_propertySeries.end()) && (it->first < stop))
        while (ip < this->mP.size() && mP[ip].time() < stop)
        {
          if ((index >= 0) && (index < static_cast<int>(numOutputs)))
          {
            TimeSeriesProperty<TYPE> * myOutput = outputs_tsp[index];
            //Copy the log out to the output
            if (myOutput)
              myOutput->addValue(mP[ip].time(), mP[ip].value());
          }
          ++ip;
        }

        //Go to the next interval
        itspl++;
        //But if we reached the end, then we are done.
        if (itspl==splitter.end())
          break;

        //No need to keep looping through the filter if we are out of events
        if (ip == this->mP.size())
          break;

      } //Looping through entries in the splitter vector

      //Make sure all entries have the correct size recorded in m_size.
      for (std::size_t i=0; i < numOutputs; i++)
      {
        TimeSeriesProperty<TYPE> * myOutput = dynamic_cast< TimeSeriesProperty<TYPE> * >(outputs[i]);
        if (myOutput)
          myOutput->m_size = myOutput->realSize();
      }
    }

    /**
     * Fill a TimeSplitterType that will filter the events by matching
     * log values >= min and < max. Creates SplittingInterval's where
     * times match the log values, and going to index==0.
     *
     * @param split :: Splitter that will be filled.
     * @param min :: min value
     * @param max :: max value
     * @param TimeTolerance :: offset added to times in seconds.
     * @param centre :: Whether the log value time is considered centred or at the beginning.
     */
    template<typename TYPE>
    void TimeSeriesProperty<TYPE>::makeFilterByValue(TimeSplitterType& split, TYPE min, TYPE max, double TimeTolerance, bool centre)
    {
      //Do nothing if the log is empty.
      if (mP.size() == 0)
        return;

      // 1. Sort
      sort();

      // 2. Do the rest
      bool lastGood = false;
      bool isGood;
      time_duration tol = DateAndTime::durationFromSeconds( TimeTolerance );
      int numgood = 0;
      DateAndTime lastTime, t;
      DateAndTime start, stop;

      for (size_t i = 0; i < mP.size(); i ++)
      {
        lastTime = t;
        //The new entry
        t =mP[i].time();
        TYPE val = mP[i].value();

        //A good value?
        isGood = ((val >= min) && (val < max));
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
            if (numgood == 1)
            {
              //There was only one point with the value. Use the last time, + the tolerance, as the end time
              stop = lastTime + tol;
              split.push_back( SplittingInterval(start, stop, 0) );
            }
            else
            {
              //At least 2 good values. Save the end time
              stop = lastTime + tol;
              split.push_back( SplittingInterval(start, stop, 0) );
            }
            //Reset the number of good ones, for next time
            numgood = 0;
          }
          lastGood = isGood;
        }
      }

      if (numgood > 0)
      {
        //The log ended on "good" so we need to close it using the last time we found
        stop = t + tol;
        split.push_back( SplittingInterval(start, stop, 0) );
        numgood = 0;
      }

      return;
    }

    /**
     *  Return the time series as a correct C++ map<DateAndTime, TYPE>. All values
     * are included.
     *
     * @return time series property values as map
     */
    template<typename TYPE>
    std::map<DateAndTime, TYPE> TimeSeriesProperty<TYPE>::valueAsCorrectMap() const
    {
      // 1. Sort if necessary
      sort();

      // 2. Data Strcture
      std::map<DateAndTime, TYPE> asMap;

      if (mP.size() > 0)
      {
        for (size_t i = 0; i < mP.size(); i ++)
          asMap[mP[i].time()] = mP[i].value();
      }

      return asMap;
    }

    /**
     *  Return the time series's values as a vector<TYPE>
     *  @return the time series's values as a vector<TYPE>
     */
    template<typename TYPE>
    std::vector<TYPE> TimeSeriesProperty<TYPE>::valuesAsVector() const
    {
      sort();

      std::vector<TYPE> out;
      out.reserve(mP.size());

      for (size_t i = 0; i < mP.size(); i ++)
        out.push_back(mP[i].value());

      return out;
    }

    /**
     * Return the time series's times as a vector<DateAndTime>
     * @return A vector of DateAndTime objects
     */
    template<typename TYPE>
    std::vector<DateAndTime> TimeSeriesProperty<TYPE>::timesAsVector() const
    {
      sort();

      std::vector<DateAndTime> out;
      out.reserve(mP.size());

      for (size_t i = 0; i < mP.size(); i ++)
      {
        out.push_back(mP[i].time());
      }

      return out;
    }

    /**
     * @return Return the series as list of times, where the time is the number of seconds since the start.
     */
    template<typename TYPE>
    std::vector<double> TimeSeriesProperty<TYPE>::timesAsVectorSeconds() const
    {
      // 1. Sort if necessary
      sort();

      // 2. Output data structure
      std::vector<double> out;
      out.reserve(mP.size());

      Kernel::DateAndTime start = mP[0].time();
      for (size_t i = 0; i < mP.size(); i ++)
      {
        out.push_back( DateAndTime::secondsFromDuration(mP[i].time() - start) );
      }

      return out;
    }

    /**
     * Add a value to the map
     *  @param time :: The time as a boost::posix_time::ptime value
     *  @param value :: The associated value
     *  @return True if insertion successful (i.e. identical time not already in map
     */
    template<typename TYPE>
    void TimeSeriesProperty<TYPE>::addValue(const Kernel::DateAndTime &time, const TYPE value)
    {

      TimeValueUnit<TYPE> newvalue(time, value);
      mP.push_back(newvalue);

      m_size ++;
      if (m_size == 1 || ( mPropSortedFlag && !(*mP.rbegin() < *(mP.rbegin()+1)) ) )
        mPropSortedFlag = false;
      else
        mPropSortedFlag = false;
      mFilterApplied = false;

      return;
    }

    /** Add a value to the map
     *  @param time :: The time as a string in the format: (ISO 8601) yyyy-mm-ddThh:mm:ss
     *  @param value :: The associated value
     *  @return True if insertion successful (i.e. identical time not already in map
     */
    template<typename TYPE>
    void TimeSeriesProperty<TYPE>::addValue(const std::string &time, const TYPE value)
    {
      return addValue(Kernel::DateAndTime(time), value);
    }

    /**
     * Add a value to the map using a time_t
     *  @param time :: The time as a time_t value
     *  @param value :: The associated value
     *  @return True if insertion successful (i.e. identical time not already in map
     */
    template<typename TYPE>
    void TimeSeriesProperty<TYPE>::addValue(const std::time_t &time, const TYPE value)
    {
      Kernel::DateAndTime dt;
      dt.set_from_time_t(time);
      return addValue(dt, value);
    }

    /** Adds vectors of values to the map. Should be much faster than repeated calls to addValue.
     *  @param times :: The time as a boost::posix_time::ptime value
     *  @param values :: The associated value
     */
    template<typename TYPE>
    void TimeSeriesProperty<TYPE>::addValues(const std::vector<Kernel::DateAndTime> &times,
        const std::vector<TYPE> & values)
        {
      for (size_t i = 0; i < times.size(); i ++)
      {
        if (i >= values.size())
          break;
        else
        {
          mP.push_back(TimeValueUnit<TYPE>(times[i], values[i]));
          m_size ++;
        }
      }

      if (values.size() > 0)
        mPropSortedFlag = false;

      return;
        }

    /**
     * Returns the last time
     * @return Value
     */
    template<typename TYPE>
    DateAndTime TimeSeriesProperty<TYPE>::lastTime() const
    {
      if (mP.size()==0)
        throw std::runtime_error("TimeSeriesProperty is empty");

      sort();

      return mP.rbegin()->time();
    }

    /** Returns the first value regardless of filter
     *  @return Value
     */
    template<typename TYPE>
    TYPE TimeSeriesProperty<TYPE>::firstValue() const
    {
      if (mP.size() == 0)
        throw std::runtime_error("TimeSeriesProperty is empty");

      sort();

      return mP[0].value();
    }

    /** Returns the first time regardless of filter
     *  @return Value
     */
    template<typename TYPE>
    DateAndTime TimeSeriesProperty<TYPE>::firstTime() const
    {
      if (mP.size()==0)
        throw std::runtime_error("TimeSeriesProperty is empty");

      sort();

      return mP[0].time();
    }

    /**
     * Returns the last value
     *  @return Value
     */
    template<typename TYPE>
    TYPE TimeSeriesProperty<TYPE>::lastValue() const
    {
      if (mP.size() == 0)
        throw std::runtime_error("TimeSeriesProperty is empty");

      sort();

      return mP.rbegin()->value();
    }


    /// Returns the number of values at UNIQUE time intervals in the time series
    /// @returns The number of unique time interfaces
    template<typename TYPE>
    int TimeSeriesProperty<TYPE>::size() const
    {
      return m_size;
    }

    /**
     * Returns the real size of the time series property map:
     * the number of entries, including repeated ones.
     */
    template<typename TYPE>
    int TimeSeriesProperty<TYPE>::realSize() const
    {
      return static_cast<int>(mP.size());
    }


    /*
     * Get the time series property as a string of 'time  value'
     * @return time series property as a string
     */
    template<typename TYPE>
    std::string TimeSeriesProperty<TYPE>::value() const
    {
      sort();

      std::stringstream ins;
      for (size_t i = 0; i < mP.size(); i ++)
      {
        try
        {
          ins << mP[i].time().toSimpleString();
          ins << "  " << mP[i].value() << std::endl;
        }
        catch (...)
        {
          //Some kind of error; for example, invalid year, can occur when converting boost time.
          ins << "Error Error" << std::endl;
        }
      }

      return ins.str();
    }

    /**  New method to return time series value pairs as std::vector<std::string>
     *
     * @return time series property values as a string vector "<time_t> value"
     */
    template<typename TYPE>
    std::vector<std::string> TimeSeriesProperty<TYPE>::time_tValue() const
    {
      sort();

      std::vector<std::string> values;
      values.reserve(mP.size());

      for (size_t i = 0; i < mP.size(); i ++)
      {
        std::stringstream line;
        line << mP[i].time().toSimpleString() << " " << mP[i].value();
        values.push_back(line.str());
      }

      return values;
    }

    /**
     * Return the time series as a C++ map<DateAndTime, TYPE>
     *
     * WARNING: THIS ONLY RETURNS UNIQUE VALUES, AND SKIPS ANY REPEATED VALUES!
     *   USE AT YOUR OWN RISK! Try valueAsCorrectMap() instead.
     * @return time series property values as map
     */
    template<typename TYPE>
    std::map<DateAndTime, TYPE> TimeSeriesProperty<TYPE>::valueAsMap() const
    {
      // 1. Sort if necessary
      sort();

      // 2. Build map

      std::map<DateAndTime, TYPE> asMap;
      if (mP.size() == 0)
        return asMap;

      TYPE d = mP[0].value();
      asMap[mP[0].time()] = d;

      for (size_t i = 1; i < mP.size(); i ++)
      {
        if (mP[i].value() != d)
        {
          // Only put entry with different value from last entry to map
          asMap[mP[i].time()] = mP[i].value();
          d = mP[i].value();
        }
      }
      return asMap;
    }

    /**
     * Set the property from a string value. Throws a NotImplementedError
     *  @throw Exception::NotImplementedError Not yet implemented
     * @return Nothing in this case
     */
    template<typename TYPE>
    std::string TimeSeriesProperty<TYPE>::setValue(const std::string&)
    {
      throw Exception::NotImplementedError("TimeSeriesProperty<TYPE>::setValue - Cannot extract TimeSeries from a std::string");
    }

    /**
     * @throw Exception::NotImplementedError Not yet implemented
     * @return Nothing in this case
     */
    template<typename TYPE>
    std::string TimeSeriesProperty<TYPE>::setDataItem(const boost::shared_ptr<DataItem>)
    {
      throw Exception::NotImplementedError("TimeSeriesProperty<TYPE>::setValue - Cannot extract TimeSeries from DataItem");
    }

    /**
     * Clears and creates a TimeSeriesProperty from these parameters:
     *  @param start_time :: The reference time as a boost::posix_time::ptime value
     *  @param time_sec :: A vector of time offset (from start_time) in seconds.
     *  @param new_values :: A vector of values, each corresponding to the time offset in time_sec.
     *    Vector sizes must match.
     */
    template<typename TYPE>
    void TimeSeriesProperty<TYPE>::create(const Kernel::DateAndTime &start_time, const std::vector<double> & time_sec,
        const std::vector<TYPE> & new_values)
        {
      if (time_sec.size() != new_values.size())
        throw std::invalid_argument("TimeSeriesProperty::create: mismatched size for the time and values vectors.");

      // Make the times(as seconds) into a vector of DateAndTime in one go.
      std::vector<DateAndTime> times;
      DateAndTime::createVector(start_time, time_sec, times);

      this->create(times, new_values);
        }

    /** Clears and creates a TimeSeriesProperty from these parameters:
     *
     * @param new_times :: A vector of DateAndTime.
     * @param new_values :: A vector of values, each corresponding to the time off set in new_time.
     *                      Vector sizes must match.
     */
    template<typename TYPE>
    void TimeSeriesProperty<TYPE>::create(const std::vector<DateAndTime> & new_times, const std::vector<TYPE> & new_values)
    {
      if (new_times.size() != new_values.size())
        throw std::invalid_argument("TimeSeriesProperty::create: mismatched size for the time and values vectors.");

      m_size = 0;
      mP.clear();
      mP.reserve(new_times.size());

      std::size_t num = new_values.size();
      for (std::size_t i=0; i < num; i++)
      {
        // By providing a guess iterator to the insert method, it speeds inserting up by a good amount.
        TimeValueUnit<TYPE> newentry(new_times[i], new_values[i]);
        mP.push_back(newentry);
      }

      // reset the size
      m_size = static_cast<int>(mP.size());

      mPropSortedFlag = false;
      mFilterApplied = false;

      return;
    }

    /** Returns the value at a particular time
     *  @param t :: time
     *  @return Value at time \a t
     */
    template<typename TYPE>
    TYPE TimeSeriesProperty<TYPE>::getSingleValue(const DateAndTime& t) const
    {
      if (mP.size() == 0)
        throw std::runtime_error("Property is empty.  Cannot return any value");

      // 1. Get sorted
      sort();

      // 2.
      TYPE value;
      if (t < mP[0].time())
      {
        // 1. Out side of lower bound
        value = mP[0].value();
      }
      else if (t >= mP.back().time())
      {
        // 2. Out side of upper bound
        value = mP.back().value();
      }
      else
      {
        // 3. Within boundary
        int index = this->findIndex(t);

        if (index < 0 || index >= int(mP.size()))
          throw std::logic_error("findIndex() returns index outside range. It is not supposed to be. ");

        value = mP[static_cast<size_t>(index)].value();
      }

      return value;
    } // END-DEF getSinglevalue()

    /** Returns the value at a particular time
     *  @param t :: time
     *  @return Value at time \a t
     */
    template<typename TYPE>
    TYPE TimeSeriesProperty<TYPE>::getSingleValue(const DateAndTime& t, int& index) const
    {
      if (mP.size() == 0)
        throw std::runtime_error("Property is empty.  Cannot return any value");

      // 1. Get sorted
      sort();

      // 2.
      TYPE value;
      if (t < mP[0].time())
      {
        // 1. Out side of lower bound
        value = mP[0].value();
        index = 0;
      }
      else if (t >= mP.back().time())
      {
        // 2. Out side of upper bound
        value = mP.back().value();
        index = int(mP.size())-1;
      }
      else
      {
        // 3. Within boundary
        index = this->findIndex(t);

        if (index < 0 || index >= int(mP.size()))
          throw std::logic_error("findIndex() returns index outside range. It is not supposed to be. ");

        value = mP[static_cast<size_t>(index)].value();
      }

      return value;
    } // END-DEF getSinglevalue()

    /**
     * Returns total value, added up for all times regardless of filter
     *  @return Total value from all times
     */
    template<typename TYPE>
    TYPE TimeSeriesProperty<TYPE>::getTotalValue() const
    {
      TYPE total = 0;

      for (size_t i = 0; i < mP.size(); ++i)
      {
        total += mP[i].value();
      }
      return total;
    }

    /** Returns n-th valid time interval, in a very inefficient way.
     *
     * Here are some special cases
     *  (1) If empty property, throw runtime_error
     *  (2) If double or more entries, skip!
     *  (3) If n = size of property, use dt from last interval
     *  (4) If n > size of property, return Interval = 0
     *  @param n :: index
     *  @return n-th time interval
     */
    template<typename TYPE>
    TimeInterval TimeSeriesProperty<TYPE>::nthInterval(int n) const
    {
      // 0. Throw exception
      if (mP.size() == 0)
        throw std::runtime_error("TimeSeriesProperty is empty (nthInterval)");

      // 1. Sort
      sort();

      // 2. Calculate time interval

      Kernel::TimeInterval deltaT;

      if (mFilter.size() == 0)
      {
        // I. No filter
        if (n >= static_cast<int>(mP.size()))
        {
          // 1. Out of bound
          ;
        }
        else if (n == static_cast<int>(mP.size())-1)
        {
          // 2. Last one by making up an end time.
          time_duration d = mP.rbegin()->time() - (mP.rbegin()+1)->time();
          DateAndTime endTime = mP.rbegin()->time() + d;
          Kernel::TimeInterval dt(mP.rbegin()->time(), endTime);
          deltaT = dt;
        }
        else
        {
          // 3. Regular
          DateAndTime startT = mP[n].time();
          DateAndTime endT = mP[n+1].time();
          TimeInterval dt(startT, endT);
          deltaT = dt;
        }
      }
      else
      {
        // II. Filter
        // II.0 apply Filter
        this->applyFilter();

        if (static_cast<size_t>(n) > mFilterQuickRef.back().second+1)
        {
          // 1. n > size of the allowed region, do nothing to dt
          ;
        }
        else if (static_cast<size_t>(n) == mFilterQuickRef.back().second+1)
        {
          // 2. n = size of the allowed region, duplicate the last one
          size_t ind_t1 = mFilterQuickRef.back().first;
          size_t ind_t2 = ind_t1-1;
          Kernel::DateAndTime t1 = (mP.begin()+ind_t1)->time();
          Kernel::DateAndTime t2 = (mP.begin()+ind_t2)->time();
          time_duration d = t1-t2;
          Kernel::DateAndTime t3 = t1+d;
          Kernel::TimeInterval dt(t1, t3);
          deltaT = dt;
        }
        else
        {
          // 3. n < size
          Kernel::DateAndTime t0;
          Kernel::DateAndTime tf;

          size_t refindex = this->findNthIndexFromQuickRef(n);
          if (refindex + 3 >= mFilterQuickRef.size())
            throw std::logic_error("nthInterval:  Haven't considered this case.");

          int diff = n - static_cast<int>(mFilterQuickRef[refindex].second);
          if (diff < 0)
            throw std::logic_error("nthInterval:  diff cannot be less than 0.");

          // i) start time
          Kernel::DateAndTime ftime0 = mFilter[mFilterQuickRef[refindex].first].first;
          size_t iStartIndex = mFilterQuickRef[refindex+1].first+static_cast<size_t>(diff);
          Kernel::DateAndTime ltime0 = mP[iStartIndex].time();
          if (iStartIndex == 0 && ftime0 < ltime0)
          {
            // a) Special case that True-filter time starts before log time
            t0 = ltime0;
          }
          else if (diff == 0)
          {
            // b) First entry... usually start from filter time
            t0 = ftime0;
          }
          else
          {
            // c) Not the first entry.. usually in the middle of TRUE filter period.  use log time
            t0 = ltime0;
          }

          // ii) end time
          size_t iStopIndex = iStartIndex + 1;
          if (iStopIndex >= mP.size())
          {
            // a) Last log entry is for the start
            Kernel::DateAndTime ftimef = mFilter[mFilterQuickRef[refindex+3].first].first;
            tf = ftimef;
          }
          else
          {
            // b) Using the earlier value of next log entry and next filter entry
            Kernel::DateAndTime ltimef = mP[iStopIndex].time();
            Kernel::DateAndTime ftimef = mFilter[mFilterQuickRef[refindex+3].first].first;
            if (ltimef < ftimef)
              tf = ltimef;
            else
              tf = ftimef;
          }

          Kernel::TimeInterval dt(t0, tf);
          deltaT = dt;
        } // END-IF-ELSE Cases
      }

      return deltaT;
    }

    //-----------------------------------------------------------------------------------------------
    /** Returns n-th value of n-th interval in an incredibly inefficient way.
     *  The algorithm is migrated from mthInterval()
     *  @param n :: index
     *  @return Value
     */
    template<typename TYPE>
    TYPE TimeSeriesProperty<TYPE>::nthValue(int n) const
    {
      TYPE value;

      // 1. Throw error if property is empty
      if (mP.size() == 0)
        throw std::runtime_error("TimeSeriesProperty is empty");

      // 2. Sort and apply filter
      sort();

      if (mFilter.size() == 0)
      {
        // 3. Situation 1:  No filter
        if (static_cast<size_t>(n) < mP.size())
        {
          TimeValueUnit<TYPE> entry = mP[n];
          value = entry.value();
        }
        else
        {
          TimeValueUnit<TYPE> entry = mP[m_size-1];
          value = entry.value();
        }
      }
      else
      {
        // 4. Situation 2: There is filter
        this->applyFilter();

        if (static_cast<size_t>(n) > mFilterQuickRef.back().second+1)
        {
          // 1. n >= size of the allowed region
          size_t ilog = (mFilterQuickRef.rbegin()+1)->first;
          value = mP[ilog].value();
        }
        else
        {
          // 2. n < size
          Kernel::DateAndTime t0;
          Kernel::DateAndTime tf;

          size_t refindex = findNthIndexFromQuickRef(n);
          if (refindex+3 >= mFilterQuickRef.size())
          {
            throw std::logic_error("Not consider out of boundary case here. ");
          }
          size_t ilog = mFilterQuickRef[refindex+1].first + (n-mFilterQuickRef[refindex].second);
          value = mP[ilog].value();
        } // END-IF-ELSE Cases
      }

      return value;
    }

    /** Returns n-th time. NOTE: Complexity is order(n)! regardless of filter
     *  Special cases: There is no special cases
     *  @param n :: index
     *  @return DateAndTime
     */
    template<typename TYPE>
    Kernel::DateAndTime TimeSeriesProperty<TYPE>::nthTime(int n) const
    {
      sort();

      if (mP.size() == 0)
        throw std::runtime_error("TimeSeriesProperty is empty");

      if (n < 0 || n >= static_cast<int>(mP.size()))
        n = static_cast<int>(mP.size())-1;

      return mP[n].time();
    }

    /* Divide the property into  allowed and disallowed time intervals according to \a filter.
     * (Repeated time-value pairs (two same time and value entries) mark the start of a gap in the values.)
     * If any time-value pair is repeated, it means that this entry is in disallowed region.
     * The gap ends and an allowed time interval starts when a single time-value is met.
       The disallowed region will be hidden for countSize() and nthInterval()
       Boundary condition
       ?. If filter[0].time > log[0].time, then all log before filter[0] are considered TRUE
       2. If filter[-1].time < log[-1].time, then all log after filter[-1] will be considered same as filter[-1]

       @param filter :: The filter mask to apply
     */
    template<typename TYPE>
    void TimeSeriesProperty<TYPE>::filterWith(const TimeSeriesProperty<bool>* filter)
    {
      // 1. Clear the current
      mFilter.clear();
      mFilterQuickRef.clear();

      if (filter->size() == 0)
      {
        // if filter is empty, return
        return;
      }

      // 2. Construct mFilter
      std::vector<Kernel::DateAndTime> filtertimes = filter->timesAsVector();
      std::vector<bool> filtervalues = filter->valuesAsVector();
      mFilter.reserve(filtertimes.size()+1);

      bool lastIsTrue = false;
      for (size_t i = 0; i < filtertimes.size(); i ++)
      {
        if (filtervalues[i] && !lastIsTrue)
        {
          // Get a true in filter but last recorded value is for false
          mFilter.push_back(std::make_pair(filtertimes[i], true));
          lastIsTrue = true;
        }
        else if (!filtervalues[i] && lastIsTrue)
        {
          // Get a False in filter but last recorded value is for TRUE
          mFilter.push_back(std::make_pair(filtertimes[i], false));
          lastIsTrue = false;
        }
      }

      // 2b) Get a clean finish
      if (filtervalues.back())
      {
        DateAndTime lastTime, nextLastT;
        if (mP.back().time() > filtertimes.back())
        {
          // Last log time is later than last filter time
          lastTime = mP.back().time();
          if ( mP[mP.size()-2].time() > filtertimes.back() )
            nextLastT = mP[mP.size()-2].time();
          else
            nextLastT = filtertimes.back();
        }
        else
        {
          // Last log time is no later than last filter time
          lastTime = filtertimes.back();
          if (mP.back().time() > filtertimes[filtertimes.size()-1])
          {
            nextLastT = mP.back().time();
          }
          else
          {
            nextLastT = *(filtertimes.rbegin()+1);
          }
        }

        time_duration dtime = lastTime - nextLastT;

        mFilter.push_back(std::make_pair(lastTime+dtime, false));
      }

      // 3. Reset flag and do filter
      mFilterApplied = false;
      applyFilter();

      return;
    }

    /**
     * Restores the property to the unsorted & unfiltered state
     */
    template<typename TYPE>
    void TimeSeriesProperty<TYPE>::clearFilter()
    {
      mFilter.clear();
      mFilterQuickRef.clear();

      return;
    }


    /**
     * Updates size()
     */
    template<typename TYPE>
    void TimeSeriesProperty<TYPE>::countSize() const
    {
      if (mFilter.size() == 0)
      {
        // 1. Not filter
        m_size = int(mP.size());
      }
      else
      {
        // 2. With Filter
        if (!mFilterApplied)
        {
          this->applyFilter();
        }
        m_size = int(mFilterQuickRef.back().second);
      }

      return;
    }

    /**  Check if str has the right time format
     *   @param str :: The string to check
     *   @return True if the format is correct, false otherwise.
     */
    template <typename TYPE>
    bool TimeSeriesProperty<TYPE>::isTimeString(const std::string &str)
    {
      if (str.size() < 19) return false;
      if (!isdigit(str[0])) return false;
      if (!isdigit(str[1])) return false;
      if (!isdigit(str[2])) return false;
      if (!isdigit(str[3])) return false;
      if (!isdigit(str[5])) return false;
      if (!isdigit(str[6])) return false;
      if (!isdigit(str[8])) return false;
      if (!isdigit(str[9])) return false;
      if (!isdigit(str[11])) return false;
      if (!isdigit(str[12])) return false;
      if (!isdigit(str[14])) return false;
      if (!isdigit(str[15])) return false;
      if (!isdigit(str[17])) return false;
      if (!isdigit(str[18])) return false;
      return true;
    }

    /**
     * This doesn't check anything -we assume these are always valid
     *  @returns an empty string ""
     */
    template<typename TYPE>
    std::string TimeSeriesProperty<TYPE>::isValid() const
    {
      return "";
    }

    /*
     * Not implemented in this class
     * @throw Exception::NotImplementedError Not yet implemented
     */
    template<typename TYPE>
    std::string TimeSeriesProperty<TYPE>::getDefault() const
    {
      throw Exception::NotImplementedError("TimeSeries properties don't have defaults");
    }

    /**
     * A TimeSeriesProperty never has a default
     */
    template<typename TYPE>
    bool TimeSeriesProperty<TYPE>::isDefault() const
    {
      return false;
    }

    /**
     * Return a TimeSeriesPropertyStatistics struct containing the
     * statistics of this TimeSeriesProperty object.
     */
    template<typename TYPE>
    TimeSeriesPropertyStatistics TimeSeriesProperty<TYPE>::getStatistics() const
    {
      TimeSeriesPropertyStatistics out;
      Mantid::Kernel::Statistics raw_stats
      = Mantid::Kernel::getStatistics(this->valuesAsVector());
      out.mean = raw_stats.mean;
      out.standard_deviation = raw_stats.standard_deviation;
      out.median = raw_stats.median;
      out.minimum = raw_stats.minimum;
      out.maximum = raw_stats.maximum;
      if (this->size() > 0)
      {
        out.duration = DateAndTime::secondsFromDuration(this->lastTime() - this->firstTime());
      }
      else
      {
        out.duration = std::numeric_limits<double>::quiet_NaN();
      }

      return out;
    }

    /*
     * Detects whether there are duplicated entries (of time) in property
     * If there is any, keep one of them
     */
    template<typename TYPE>
    void TimeSeriesProperty<TYPE>::eliminateDuplicates()
    {
      // 1. Sort if necessary
      sort();

      // 2. Detect and Remove Duplicated
      size_t numremoved = 0;

      typename std::vector<TimeValueUnit<TYPE> >::iterator vit;
      vit = mP.begin()+1;
      Kernel::DateAndTime prevtime = mP.begin()->time();
      while (vit != mP.end())
      {
        Kernel::DateAndTime currtime = vit->time();
        if (prevtime == currtime)
        {
          // Print out warning
          g_log.debug() << "Entry @ Time = " << prevtime << "has duplicate time stamp.  Remove entry with Value = " <<
              (vit-1)->value() << std::endl;

          // A duplicated entry!
          vit = mP.erase(vit-1);

          numremoved ++;
        }

        // b) progress
        prevtime = currtime;
        ++ vit;
      }

      // 3. Finish
      g_log.warning() << "Log " << this->name() << " has " << numremoved << " entries removed due to duplicated time. " << std::endl;

      return;
    }

    /*
     * Print the content to string
     */
    template <typename TYPE>
    std::string TimeSeriesProperty<TYPE>::toString() const
    {
      std::stringstream ss;
      for (size_t i = 0; i < mP.size(); ++i)
        ss << mP[i].time() << "\t\t" << mP[i].value() << std::endl;

      return ss.str();
    }


    //-------------------------------------------------------------------------
    // Private methods
    //-------------------------------------------------------------------------
    /*
     * Sort vector mP and set the flag
     */
    template <typename TYPE>
    void TimeSeriesProperty<TYPE>::sort() const
    {
      if (!mPropSortedFlag)
      {
        std::sort(mP.begin(), mP.end());
        mPropSortedFlag = true;
      }
    }

    /*
     * Find the index of the entry of time t in the mP vector (sorted)
     * Return @ if t is within log.begin and log.end, then the value equal or just smaller than t
     */
    template <typename TYPE>
    int TimeSeriesProperty<TYPE>::findIndex(Kernel::DateAndTime t) const
    {
      // 0. Return with an empty container
      if (mP.size() == 0)
        return 0;

      // 1. Sort
      sort();

      // 2. Extreme value
      if (t <= mP[0].time())
        return 0;
      else if (t >= mP.back().time())
        return (int(mP.size()-1));

      // 3. Find by lower_bound()
      typename std::vector<TimeValueUnit<TYPE> >::iterator fid;
      TimeValueUnit<TYPE> temp(t, mP[0].value());
      fid = std::lower_bound(mP.begin(), mP.end(), temp);

      int newindex = int(fid-mP.begin());
      if (fid->time() > t)
        newindex --;

      return newindex;
    }

    /*
     * Find the upper_bound of time t in container.
     * Search range:  begin+istart to begin+iend
     * Return C[ir] == t or C[ir] > t and C[ir-1] < t
     *        -1:          exceeding lower bound
     *        mP.size():   exceeding upper bound
     */
    template<typename TYPE>
    int TimeSeriesProperty<TYPE>::upperBound(Kernel::DateAndTime t, int istart, int iend) const
    {
      // 0. Check validity
      if (istart < 0)
      {
        throw std::invalid_argument("Start Index cannot be less than 0");
      }
      if (iend >= static_cast<int>(mP.size()))
      {
        throw std::invalid_argument("End Index cannot exceed the boundary");
      }
      if (istart > iend)
      {
        throw std::invalid_argument("Start index cannot be greater than end index");
      }

      // 1. Return instantly if it is out of boundary
      if (t < (mP.begin()+istart)->time())
      {
        return -1;
      }
      if (t > (mP.begin()+iend)->time())
      {
        return static_cast<int>(mP.size());
      }

      // 2. Sort
      sort();

      // 3. Construct the pair for comparison and do lower_bound()
      TimeValueUnit<TYPE> temppair(t, mP[0].value());
      typename std::vector<TimeValueUnit<TYPE> >::iterator fid;
      fid = std::lower_bound((mP.begin()+istart), (mP.begin()+iend+1), temppair);
      if (fid == mP.end())
        throw std::runtime_error("Cannot find data");

      // 4. Calculate return value
      size_t index = size_t(fid-mP.begin());

      return int(index);
    }

    /*
     * Apply filter
     * Requirement: There is no 2 consecutive 'second' values that are same in mFilter
     *
     * It only works with filter starting from TRUE AND having TRUE and FALSE altered
     */
    template<typename TYPE>
    void TimeSeriesProperty<TYPE>::applyFilter() const
    {
      // 1. Check and reset
      if (mFilterApplied)
        return;
      if (mFilter.size() == 0)
        return;

      mFilterQuickRef.clear();

      // 2. Apply filter
      int icurlog = 0;
      for (size_t ift = 0; ift < mFilter.size(); ift ++)
      {
        if (mFilter[ift].second)
        {
          // a) Filter == True: indicating the start of a quick reference region
          int istart = 0;
          if (icurlog > 0)
            istart = icurlog-1;

          if (icurlog < static_cast<int>(mP.size()))
            icurlog = this->upperBound(mFilter[ift].first, istart, static_cast<int>(mP.size())-1);

          if (icurlog < 0)
          {
            // i. If it is out of lower boundary, add filter time, add 0 time
            if (mFilterQuickRef.size() > 0)
              throw std::logic_error("return log index < 0 only occurs with the first log entry");

            mFilterQuickRef.push_back(std::make_pair(ift, 0));
            mFilterQuickRef.push_back(std::make_pair(0, 0));

            icurlog = 0;
          }
          else if (icurlog >= static_cast<int>(mP.size()))
          {
            // ii.  If it is out of upper boundary, still record it.  but make the log entry to mP.size()+1
            size_t ip = 0;
            if (mFilterQuickRef.size() >= 4)
              ip = mFilterQuickRef.back().second;
            mFilterQuickRef.push_back(std::make_pair(ift, ip));
            mFilterQuickRef.push_back(std::make_pair(mP.size()+1, ip));
          }
          else
          {
            // iii. The returned value is in the boundary.
            size_t numintervals = 0;
            if (mFilterQuickRef.size() > 0)
            {
              numintervals = mFilterQuickRef.back().second;
            }
            if (mFilter[ift].first < mP[icurlog].time())
            {
              if (icurlog == 0)
              {
                throw std::logic_error("In this case, icurlog won't be zero! ");
              }
              icurlog --;
            }
            mFilterQuickRef.push_back(std::make_pair(ift, numintervals));
            // Note: numintervals inherits from last filter
            mFilterQuickRef.push_back(std::make_pair(icurlog, numintervals));
          }
        } // Filter value is True
        else  if (mFilterQuickRef.size()%4 == 2)
        {
          // b) Filter == False: indicating the end of a quick reference region
          int ilastlog = icurlog;

          if (ilastlog < static_cast<int>(mP.size()))
          {
            // B1: Last TRUE entry is still within log
            icurlog = this->upperBound(mFilter[ift].first, icurlog, static_cast<int>(mP.size())-1);

            if (icurlog < 0)
            {
              // i.   Some false filter is before the first log entry.  The previous filter does not make sense
              if (mFilterQuickRef.size() != 2)
                throw std::logic_error("False filter is before first log entry.  QuickRef size must be 2.");
              mFilterQuickRef.pop_back();
              mFilterQuickRef.clear();
            }
            else
            {
              // ii.  Register the end of a valid log
              if (ilastlog < 0)
                throw std::logic_error("LastLog is not expected to be less than 0");

              int delta_numintervals = icurlog - ilastlog;
              if (delta_numintervals < 0)
                throw std::logic_error("Havn't considered delta numinterval can be less than 0.");

              size_t new_numintervals = mFilterQuickRef.back().second + static_cast<size_t>(delta_numintervals);

              mFilterQuickRef.push_back(std::make_pair(icurlog, new_numintervals));
              mFilterQuickRef.push_back(std::make_pair(ift, new_numintervals));
            }
          }
          else
          {
            // B2. Last TRUE filter's time is already out side of log.
            size_t new_numintervals = mFilterQuickRef.back().second+1;
            mFilterQuickRef.push_back(std::make_pair(icurlog-1, new_numintervals));
            mFilterQuickRef.push_back(std::make_pair(ift, new_numintervals));
          }
        } // Filter value is FALSE

      } // ENDFOR

      // 5. Change flag
      mFilterApplied = true;

      // 6. Re-count size
      countSize();

      return;
    }

    /*
     * A new algorithm sto find Nth index.  It is simple and leave a lot work to the callers
     *
     * Return: the index of the quick reference vector
     */
    template <typename TYPE>
    size_t TimeSeriesProperty<TYPE>::findNthIndexFromQuickRef(int n) const
    {
      size_t index = 0;

      // 1. Do check
      if (n < 0)
        throw std::invalid_argument("Unable to take into account negative index. ");
      else if (mFilterQuickRef.size() == 0)
        throw std::runtime_error("Quick reference is not established. ");

      // 2. Return...
      if (static_cast<size_t>(n) >= mFilterQuickRef.back().second)
      {
        // 2A.  Out side of boundary
        index = mFilterQuickRef.size();
      }
      else
      {
        // 2B. Inside
        for (size_t i = 0; i < mFilterQuickRef.size(); i+=4)
        {
          if (static_cast<size_t>(n) >= mFilterQuickRef[i].second && static_cast<size_t>(n) < mFilterQuickRef[i+3].second)
          {
            index = i;
            break;
          }
        }
      }

      return index;
    }

    /**
     * Set the value of the property via a reference to another property.
     * If the value is unacceptable the value is not changed but a string is returned.
     * The value is only accepted if the other property has the same type as this
     * @param right :: A reference to a property.
     */
    template <typename TYPE>
    std::string TimeSeriesProperty<TYPE>::setValueFromProperty( const Property& right )
    {
      auto prop = dynamic_cast<const TimeSeriesProperty<TYPE>*>(&right);
      if ( !prop )
      {
        return "Could not set value: properties have different type.";
      }
      mP = prop->mP;
      m_size = prop->m_size;
      mPropSortedFlag = prop->mPropSortedFlag;
      mFilter = prop->mFilter;
      mFilterQuickRef = prop->mFilterQuickRef;
      mFilterApplied = prop->mFilterApplied;
      return "";
    }


    /// @cond
    // -------------------------- Macro to instantiation concrete types --------------------------------
#define INSTANTIATE(TYPE) \
    template MANTID_KERNEL_DLL class TimeSeriesProperty<TYPE>;

    // -------------------------- Concrete instantiation -----------------------------------------------
    INSTANTIATE(int);
    INSTANTIATE(long);
    INSTANTIATE(long long);
    INSTANTIATE(unsigned int);
    INSTANTIATE(unsigned long);
    INSTANTIATE(unsigned long long);
    INSTANTIATE(float);
    INSTANTIATE(double);
    INSTANTIATE(std::string);
    INSTANTIATE(bool);

    /// @endcond

  } // namespace Kernel
} // namespace Mantid



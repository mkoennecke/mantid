/*WIKI* 


The LoadRaw algorithm stores data from the [[RAW_File | RAW]] file in a [[Workspace2D]], which will naturally contain histogram data with each spectrum going into a separate histogram. The time bin boundaries (X values) will be common to all histograms and will have their [[units]] set to time-of-flight. The Y values will contain the counts and will be unit-less (i.e. no division by bin width or normalisation of any kind). The errors, currently assumed Gaussian, will be set to be the square root of the number of counts in the bin.

=== Optional properties ===
If only a portion of the data in the [[RAW_File | RAW]] file is required, then the optional 'spectrum' properties can be set before execution of the algorithm. Prior to loading of the data the values provided are checked and the algorithm will fail if they are found to be outside the limits of the dataset.

=== Multiperiod data === 
If the RAW file contains multiple periods of data this will be detected and the different periods will be output as separate workspaces, which after the first one will have the period number appended (e.g. OutputWorkspace_period).
Each workspace will share the same [[Instrument]], SpectraToDetectorMap and [[Sample]] objects.
If the optional 'spectrum' properties are set for a multiperiod dataset, then they will ignored.

===Subalgorithms used===
LoadRaw runs the following algorithms as child algorithms to populate aspects of the output [[Workspace]]:
* [[LoadInstrument]] - Looks for an instrument definition file named XXX_Definition.xml, where XXX is the 3 letter instrument prefix on the RAW filename, in the directory specified by the "instrumentDefinition.directory" property given in the config file (or, if not provided, in the relative path ../Instrument/). If the instrument definition file is not found then the [[LoadInstrumentFromRaw]] algorithm will be run instead.
* [[LoadMappingTable]] - To build up the mapping between the spectrum numbers and the Detectors of the attached [[Instrument]].
* [[LoadLog]] - Will look for any log files in the same directory as the RAW file and load their data into the workspace's [[Sample]] object.


*WIKI*/
//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "LoadRaw/isisraw.h"
#include "MantidAPI/FileProperty.h"
#include "MantidAPI/SpectraDetectorMap.h"
#include "MantidDataHandling/LoadLog.h"
#include "MantidDataHandling/LoadRaw.h"
#include "MantidDataHandling/LoadRawHelper.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidKernel/ArrayProperty.h"
#include "MantidKernel/ConfigService.h"
#include "MantidKernel/TimeSeriesProperty.h"
#include "MantidKernel/UnitFactory.h"
#include "MantidKernel/BoundedValidator.h"
#include <boost/shared_ptr.hpp>
#include <cmath>
#include <cstdio> // Required for gcc 4.4
#include <Poco/Path.h>

namespace Mantid
{
  namespace DataHandling
  {
    // Register the algorithm into the algorithm factory
    DECLARE_ALGORITHM(LoadRaw)
    
    /// Sets documentation strings for this algorithm
    void LoadRaw::initDocs()
    {
      this->setWikiSummary("Loads a data file in ISIS [[RAW_File | RAW]] format and stores it in a 2D [[workspace]] ([[Workspace2D]] class). ");
      this->setOptionalMessage("Loads a data file in ISIS  RAW format and stores it in a 2D workspace (Workspace2D class).");
    }
    

    using namespace Kernel;
    using namespace API;

    /// Empty default constructor
    LoadRaw::LoadRaw() :
      Algorithm(), m_filename(), m_numberOfSpectra(0), m_numberOfPeriods(0),
      m_list(false), m_interval(false), m_spec_list(), m_spec_min(0), m_spec_max(Mantid::EMPTY_INT())
    {}

    /// Initialisation method.
    void LoadRaw::init()
    {
      // Extension checking is not case sensitive
      // MG 20/07/09: I've had to change these extensions so that the native Windows file dialog can recognise
      // the file types correctly
      std::vector<std::string> exts;
      exts.push_back(".raw");
      exts.push_back(".s*");
      exts.push_back(".add");

      declareProperty(new FileProperty("Filename", "", FileProperty::Load, exts),
                      "The name of the RAW file to read, including its full or relative\n"
                      "path. (N.B. case sensitive if running on Linux).");
      declareProperty(new WorkspaceProperty<DataObjects::Workspace2D>("OutputWorkspace","",Direction::Output),
        "The name of the workspace that will be created, filled with the\n"
        "read-in data and stored in the Analysis Data Service.  If the input\n"
        "RAW file contains multiple periods higher periods will be stored in\n"
        "separate workspaces called OutputWorkspace_PeriodNo.");

      auto mustBePositive = boost::make_shared<BoundedValidator<int> >();
      mustBePositive->setLower(1);
      declareProperty(new PropertyWithValue<specid_t>("SpectrumMin", 1, mustBePositive),
        "The index number of the first spectrum to read.  Only used if\n"
        "spectrum_max is set.");
      declareProperty(new PropertyWithValue<specid_t>("SpectrumMax", Mantid::EMPTY_INT(), mustBePositive),
        "The number of the last spectrum to read. Only used if explicitly\n"
        "set.");

      declareProperty(new ArrayProperty<specid_t>("SpectrumList"),
        "A comma-separated list of individual spectra to read.  Only used if\n"
        "explicitly set.");
    }

    /** Executes the algorithm. Reading in the file and creating and populating
     *  the output workspace
     *
     *  @throw Exception::FileError If the RAW file cannot be found/opened
     *  @throw std::invalid_argument If the optional properties are set to invalid values
     */
    void LoadRaw::exec()
    {
      // Retrieve the filename from the properties
      m_filename = getPropertyValue("Filename");

      LoadRawHelper *helper = new LoadRawHelper;
      FILE* file = helper->openRawFile(m_filename);
      ISISRAW iraw;
      iraw.ioRAW(file, true);

      std::string title(iraw.r_title, 80);
      g_log.information("**** Run title: "+title+ "***");
      
      // Read in the number of spectra in the RAW file
      m_numberOfSpectra = iraw.t_nsp1;
      // Read the number of periods in this file
      m_numberOfPeriods = iraw.t_nper;
      // Need to extract the user-defined output workspace name
      Property *ws = getProperty("OutputWorkspace");
      std::string localWSName = ws->value();

      // Call private method to validate the optional parameters, if set
      checkOptionalProperties();

      // Read the number of time channels (i.e. bins) from the RAW file
      const int channelsPerSpectrum = iraw.t_ntc1;
      // Read in the time bin boundaries
      const int lengthIn = channelsPerSpectrum + 1;
      float* timeChannels = new float[lengthIn];
      iraw.getTimeChannels(timeChannels, lengthIn);
      // Put the read in array into a vector (inside a shared pointer)
      boost::shared_ptr<MantidVec> timeChannelsVec
                          (new MantidVec(timeChannels, timeChannels + lengthIn));
      // Create an array to hold the read-in data
      int* spectrum = new int[lengthIn];

      // Calculate the size of a workspace, given its number of periods & spectra to read
      specid_t total_specs;
      if( m_interval || m_list)
      {
        total_specs = static_cast<specid_t>(m_spec_list.size());
        if (m_interval)
        {
          total_specs += (m_spec_max-m_spec_min+1);
          m_spec_max += 1;
        }
      }
      else
      {
        total_specs = m_numberOfSpectra;
        // In this case want all the spectra, but zeroth spectrum is garbage so go from 1 to NSP1
        m_spec_min = 1;
        m_spec_max = m_numberOfSpectra + 1;
      }

      double histTotal = static_cast<double>(total_specs * m_numberOfPeriods);
      int32_t histCurrent = -1;

      // Create the 2D workspace for the output
      DataObjects::Workspace2D_sptr localWorkspace = boost::dynamic_pointer_cast<DataObjects::Workspace2D>
               (WorkspaceFactory::Instance().create("Workspace2D",total_specs,lengthIn,lengthIn-1));
      localWorkspace->getAxis(0)->unit() = UnitFactory::Instance().create("TOF");
      localWorkspace->setTitle(title);
      // Run parameters
      helper->loadRunParameters(localWorkspace, &iraw);
      delete helper;
      helper = NULL;


      // Loop over the number of periods in the raw file, putting each period in a separate workspace
      for (int period = 0; period < m_numberOfPeriods; ++period) {

        if ( period > 0 ) localWorkspace =  boost::dynamic_pointer_cast<DataObjects::Workspace2D>
                                              (WorkspaceFactory::Instance().create(localWorkspace));

        specid_t counter = 0;
        for (specid_t i = m_spec_min; i < m_spec_max; ++i)
        {
          // Shift the histogram to read if we're not in the first period
          int32_t histToRead = i + period*total_specs;
          loadData(timeChannelsVec,counter,histToRead,iraw,lengthIn,spectrum,localWorkspace );
          counter++;
          if (++histCurrent % 100 == 0) progress(static_cast<double>(histCurrent)/histTotal);
          interruption_point();
        }
        // Read in the spectra in the optional list parameter, if set
        if (m_list)
        {
          for(size_t i=0; i < m_spec_list.size(); ++i)
          {
            loadData(timeChannelsVec,counter,m_spec_list[i],iraw,lengthIn,spectrum, localWorkspace );
            counter++;
            if (++histCurrent % 100 == 0) progress(static_cast<double>(histCurrent)/histTotal);
            interruption_point();
          }
        }
        // Just a sanity check
        assert(counter == total_specs);

        std::string outputWorkspace = "OutputWorkspace";
        if (period == 0)
        {
          // Only run the sub-algorithms once
          runLoadInstrument(localWorkspace );
          runLoadMappingTable(localWorkspace );
          runLoadLog(localWorkspace );
          const int period_number = 1;
          Property* log=createPeriodLog(period_number);
          if(log)
          {
            localWorkspace->mutableRun().addLogData(log);
            localWorkspace->mutableRun().addLogData(createCurrentPeriodLog(period_number));
          }
          // Set the total proton charge for this run
          // (not sure how this works for multi_period files)
          localWorkspace->mutableRun().setProtonCharge(iraw.rpb.r_gd_prtn_chrg);
        }
        else   // We are working on a higher period of a multiperiod raw file
        {
          // Create a WorkspaceProperty for the new workspace of a higher period
          // The workspace name given in the OutputWorkspace property has _periodNumber appended to it
          //                (for all but the first period, which has no suffix)
          std::stringstream suffix;
          suffix << (period+1);
          outputWorkspace += suffix.str();
          std::string WSName = localWSName + "_" + suffix.str();
          declareProperty(new WorkspaceProperty<DataObjects::Workspace2D>(outputWorkspace,WSName,Direction::Output));
          g_log.information() << "Workspace " << WSName << " created. \n";
        }

        if (localWorkspace)
          localWorkspace->updateSpectraUsingMap();

        // Assign the result to the output workspace property
        setProperty(outputWorkspace,localWorkspace);

      } // loop over periods

      // Clean up
      delete[] timeChannels;
      delete[] spectrum;
    }

    /** Creates a TimeSeriesProperty<bool> showing times when a particular period was active.
    *  @param period :: The data period
    *  @return the times when requested period was active
    */
    Kernel::Property*  LoadRaw::createPeriodLog(int period)const
    {
      Kernel::TimeSeriesProperty<int>* periods = dynamic_cast< Kernel::TimeSeriesProperty<int>* >(m_perioids.get());
      if(!periods) return 0;
      std::ostringstream ostr;
      ostr<<period;
      Kernel::TimeSeriesProperty<bool>* p = new Kernel::TimeSeriesProperty<bool> ("period "+ostr.str());
      std::map<Kernel::DateAndTime, int> pMap = periods->valueAsMap();
      std::map<Kernel::DateAndTime, int>::const_iterator it = pMap.begin();
      if (it->second != period)
        p->addValue(it->first,false);
      for(;it!=pMap.end();++it)
        p->addValue(it->first, (it->second == period) );

      return p;
    }

    /**
    Create a log vale for the current period.
    @param period: The period number to create the log entry for.
    */
    Kernel::Property* LoadRaw::createCurrentPeriodLog(const int& period) const
    {
      Kernel::PropertyWithValue<int>* currentPeriodProperty = new Kernel::PropertyWithValue<int>("current_period", period);
      return currentPeriodProperty;
    }


    /**
     * Check if a file is a text file
     * @param filename :: The file path to check
     * @returns true if the file an ascii text file, false otherwise
     */
    bool LoadRaw::isAscii(const std::string & filename) const
    {
      bool nonAscii = true;
      FILE* file = fopen(filename.c_str(), "rb");
      char data[256];
      char *pend = &data[fread(data, 1, sizeof(data), file)];
      /*
       * Call it a binary file if we find a non-ascii character in the 
       * first 256 bytes of the file.
       */
      for( char *p = data;  p < pend; ++p )
      {
        unsigned long ch = (unsigned long)*p;
        if( !(ch <= 0x7F) )
        {
          nonAscii = false;
          break;
          //return false;
        }
      }

      fclose(file);
      return nonAscii;
    }

    /// Validates the optional 'spectra to read' properties, if they have been set
    void LoadRaw::checkOptionalProperties()
    {
      //read in the data supplied to the algorithm
      m_spec_list = getProperty("SpectrumList");
      m_spec_max = getProperty("SpectrumMax");
      //check that data
      m_list = !(m_spec_list.empty());
      m_interval = !(m_spec_max == Mantid::EMPTY_INT());
      if ( m_spec_max == Mantid::EMPTY_INT() ) m_spec_max = 0;

      // Check validity of spectra list property, if set
      if ( m_list )
      {
        m_list = true;
        const int64_t minlist = *min_element(m_spec_list.begin(),m_spec_list.end());
        const int64_t maxlist = *max_element(m_spec_list.begin(),m_spec_list.end());
        if ( maxlist > m_numberOfSpectra || minlist == 0)
        {
          g_log.error("Invalid list of spectra");
          throw std::invalid_argument("Inconsistent properties defined");
        }
      }

      // Check validity of spectra range, if set
      if ( m_interval )
      {
        m_interval = true;
        m_spec_min = getProperty("SpectrumMin");
        if ( m_spec_max < m_spec_min || m_spec_max > m_numberOfSpectra )
        {
          g_log.error("Invalid Spectrum min/max properties");
          throw std::invalid_argument("Inconsistent properties defined");
        }
      }
    }

    /** Read in a single spectrum from the raw file
     *  @param tcbs ::     The vector containing the time bin boundaries
     *  @param hist ::     The workspace index
     *  @param i ::        The spectrum number
     *  @param iraw ::     A reference to the ISISRAW object
     *  @param lengthIn :: The number of elements in a spectrum
     *  @param spectrum :: Pointer to the array into which the spectrum will be read
     *  @param localWorkspace :: A pointer to the workspace in which the data will be stored
     */
    void LoadRaw::loadData(const MantidVecPtr::ptr_type& tcbs, int32_t hist, specid_t& i, ISISRAW& iraw, const int& lengthIn, int* spectrum, DataObjects::Workspace2D_sptr localWorkspace)
    {
      // Read in a spectrum
      memcpy(spectrum, iraw.dat1 + i * lengthIn, lengthIn * sizeof(int));
      // Put it into a vector, discarding the 1st entry, which is rubbish
      // But note that the last (overflow) bin is kept
      MantidVec& Y = localWorkspace->dataY(hist);
      Y.assign(spectrum + 1, spectrum + lengthIn);
      // Create and fill another vector for the errors, containing sqrt(count)
      MantidVec& E = localWorkspace->dataE(hist);
      std::transform(Y.begin(), Y.end(), E.begin(), dblSqrt);
      // Populate the workspace. Loop starts from 1, hence i-1
      localWorkspace->setX(hist, tcbs);

      localWorkspace->getAxis(1)->spectraNo(hist)= i;
      // NOTE: Raw numbers go straight into the workspace
      //     - no account taken of bin widths/units etc.
    }

    /// Run the sub-algorithm LoadInstrument (or LoadInstrumentFromRaw)
    void LoadRaw::runLoadInstrument(DataObjects::Workspace2D_sptr localWorkspace)
    {
      // instrument ID
      const std::string::size_type stripPath = m_filename.find_last_of("\\/");
      std::string instrumentID = m_filename.substr(stripPath+1,3);  // get the 1st 3 letters of filename part

      IAlgorithm_sptr loadInst = createSubAlgorithm("LoadInstrument");

      // Now execute the sub-algorithm. Catch and log any error, but don't stop.
      bool executionSuccessful(true);
      try
      {
        loadInst->setPropertyValue("InstrumentName", instrumentID);
        loadInst->setProperty<MatrixWorkspace_sptr> ("Workspace", localWorkspace);
        loadInst->setProperty("RewriteSpectraMap", false);
        loadInst->execute();
      }
      catch( std::invalid_argument&)
      {
        g_log.information("Invalid argument to LoadInstrument sub-algorithm");
        executionSuccessful = false;
      }
      catch (std::runtime_error&)
      {
        g_log.information("Unable to successfully run LoadInstrument sub-algorithm");
        executionSuccessful = false;
      }

      // If loading instrument definition file fails, run LoadInstrumentFromRaw instead
      if( !executionSuccessful )
      {
        g_log.information() << "Instrument definition file " 
          << " not found. Attempt to load information about \n"
          << "the instrument from raw data file.\n";
        runLoadInstrumentFromRaw(localWorkspace);
      }
    }

    /// Run LoadInstrumentFromRaw as a sub-algorithm (only if loading from instrument definition file fails)
    void LoadRaw::runLoadInstrumentFromRaw(DataObjects::Workspace2D_sptr localWorkspace)
    {
      IAlgorithm_sptr loadInst = createSubAlgorithm("LoadInstrumentFromRaw");
      loadInst->setPropertyValue("Filename", m_filename);
      // Set the workspace property to be the same one filled above
      loadInst->setProperty<MatrixWorkspace_sptr>("Workspace",localWorkspace);

      // Now execute the sub-algorithm. Catch and log any error, but don't stop.
      try
      {
        loadInst->execute();
      }
      catch (std::runtime_error&)
      {
        g_log.error("Unable to successfully run LoadInstrumentFromRaw sub-algorithm");
      }

      if ( ! loadInst->isExecuted() ) g_log.error("No instrument definition loaded");
    }

    /// Run the LoadMappingTable sub-algorithm to fill the SpectraToDetectorMap
    void LoadRaw::runLoadMappingTable(DataObjects::Workspace2D_sptr localWorkspace)
    {
      // Now determine the spectra to detector map calling sub-algorithm LoadMappingTable
      // There is a small penalty in re-opening the raw file but nothing major.
      IAlgorithm_sptr loadmap= createSubAlgorithm("LoadMappingTable");
      loadmap->setPropertyValue("Filename", m_filename);
      loadmap->setProperty<MatrixWorkspace_sptr>("Workspace",localWorkspace);
      try
      {
        loadmap->execute();
      }
      catch (std::runtime_error&)
      {
          g_log.error("Unable to successfully execute LoadMappingTable sub-algorithm");
      }

      if ( ! loadmap->isExecuted() ) g_log.error("LoadMappingTable sub-algorithm is not executed");
    }

    /// Run the LoadLog sub-algorithm
    void LoadRaw::runLoadLog(DataObjects::Workspace2D_sptr localWorkspace)
    {
      IAlgorithm_sptr loadLog = createSubAlgorithm("LoadLog");
      // Pass through the same input filename
      loadLog->setPropertyValue("Filename",m_filename);
      // Set the workspace property to be the same one filled above
      loadLog->setProperty<MatrixWorkspace_sptr>("Workspace",localWorkspace);

      // Now execute the sub-algorithm. Catch and log any error, but don't stop.
      try
      {
        loadLog->execute();
      }
      catch (std::runtime_error&)
      {
        g_log.error("Unable to successfully run LoadLog sub-algorithm");
      }

      if ( ! loadLog->isExecuted() ) g_log.error("Unable to successfully run LoadLog sub-algorithm");
    }

    double LoadRaw::dblSqrt(double in)
    {
      return sqrt(in);
    }

  } // namespace DataHandling
} // namespace Mantid

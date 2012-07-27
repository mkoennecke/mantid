//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAPI/AlgorithmManager.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/FrameworkManager.h"
#include "MantidAPI/InstrumentDataService.h"
#include "MantidAPI/MemoryManager.h"
#include "MantidAPI/WorkspaceGroup.h"

#include "MantidKernel/Exception.h"
#include "MantidKernel/LibraryManager.h"
#include "MantidKernel/Memory.h"
#include <cstdarg>
#include <napi.h>

#ifdef _WIN32
#include <winsock2.h>
#endif

namespace Mantid
{
namespace API
{

  /** This is a function called every time NeXuS raises an error.
   * This swallows the errors and outputs nothing.
   *
   * @param data :: data passed in NXMSetError (will be NULL)
   * @param text :: text of the error.
   */
  void NexusErrorFunction(void *data, char *text)
  {
    UNUSED_ARG(data);
    UNUSED_ARG(text);
    // Do nothing.
  }


/// Default constructor
FrameworkManagerImpl::FrameworkManagerImpl() : g_log(Kernel::Logger::get("FrameworkManager"))
#ifdef MPI_BUILD
      , m_mpi_environment()
#endif
{
  // Mantid only understands English...
  setGlobalLocaleToAscii();
  // Setup memory allocation scheme
  Kernel::MemoryOptions::initAllocatorOptions();

#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

#ifdef _MSC_VER
  // This causes the exponent to consist of two digits (Windows Visual Studio normally 3, Linux default 2), where two digits are not sufficient I presume it uses more
  _set_output_format(_TWO_DIGIT_EXPONENT);
#endif

  Kernel::ConfigServiceImpl& config = Kernel::ConfigService::Instance();
  // Load plugin libraries if possible
  std::string pluginDir = config.getString("plugins.directory");
  if (pluginDir.length() > 0)
  {
    Mantid::Kernel::LibraryManager::Instance().OpenAllLibraries(pluginDir, false);
  }
  // Load Paraview plugin libraries if possible
  if(config.quickParaViewCheck())
  {
    const std::string pvPluginDir = config.getString("pvplugins.directory");
    if (pvPluginDir.length() > 0)
    {
      this->g_log.debug("Loading PV plugin libraries");
      Mantid::Kernel::LibraryManager::Instance().OpenAllLibraries(pvPluginDir, false);
    }
    else
    {
      this->g_log.notice("No PV plugin library directory");
    }
  }
  else
  {
    this->g_log.debug("Cannot load paraview libraries");
  }

  // Disable reporting errors from Nexus (they clutter up the output).
  NXMSetError(NULL, NexusErrorFunction);

  g_log.debug() << "FrameworkManager created." << std::endl;
}

/// Destructor
FrameworkManagerImpl::~FrameworkManagerImpl()
{
}

/**
 * Set the global locale for all C++ stream operations to use simple ASCII characters.
 * If the system supports it UTF-8 encoding will be used, otherwise the 
 * classic C locale is used
 */
void FrameworkManagerImpl::setGlobalLocaleToAscii()
{
  // This ensures that all subsequent stream operations interpret everything as simple
  // ASCII. On systems in the UK and US having this as the system default is not an issue.
  // However, systems that have their encoding set differently can see unexpected behavour when
  // translating from string->numeral values. One example is floating-point interpretation in 
  // German where a comma is used instead of a period.
  std::locale::global(std::locale::classic());
}

/** Clears all memory associated with the AlgorithmManager
 *  and with the Analysis & Instrument data services.
 */
void FrameworkManagerImpl::clear()
{
  clearAlgorithms();
  clearInstruments();
  clearData();
}

/**
 * Clear memory associated with the AlgorithmManager
 */
void FrameworkManagerImpl::clearAlgorithms()
{
  AlgorithmManager::Instance().clear();
}

/**
 * Clear memory associated with the ADS
 */
void FrameworkManagerImpl::clearData()
{
  AnalysisDataService::Instance().clear();
  Mantid::API::MemoryManager::Instance().releaseFreeMemory();
}

/**
 * Clear memory associated with the IDS
 */
void FrameworkManagerImpl::clearInstruments()
{
  InstrumentDataService::Instance().clear();
}

/** Creates and initialises an instance of an algorithm
 * 
 *  @param algName :: The name of the algorithm required
 *  @param version :: The version of the algorithm
 *  @return A pointer to the created algorithm.
 *          WARNING! DO NOT DELETE THIS POINTER, because it is owned
 *          by a shared pointer in the AlgorithmManager.
 * 
 *  @throw NotFoundError Thrown if algorithm requested is not registered
 */
IAlgorithm* FrameworkManagerImpl::createAlgorithm(const std::string& algName, const int& version)
{ 
   IAlgorithm* alg = AlgorithmManager::Instance().create(algName,version).get();
   return alg;
}

/** Creates an instance of an algorithm and sets the properties provided
 * 
 *  @param algName :: The name of the algorithm required
 *  @param propertiesArray :: A single string containing properties in the 
 *                         form "Property1=Value1;Property2=Value2;..."
 *  @param version :: The version of the algorithm
 *  @return A pointer to the created algorithm
 *          WARNING! DO NOT DELETE THIS POINTER, because it is owned
 *          by a shared pointer in the AlgorithmManager.
 * 
 *  @throw NotFoundError Thrown if algorithm requested is not registered
 *  @throw std::invalid_argument Thrown if properties string is ill-formed
 */ 
IAlgorithm* FrameworkManagerImpl::createAlgorithm(const std::string& algName,const std::string& propertiesArray, const int& version)
{
  // Use the previous method to create the algorithm
  IAlgorithm *alg = AlgorithmManager::Instance().create(algName,version).get();//createAlgorithm(algName);
  alg->setProperties(propertiesArray);
  return alg;
}

/** Creates an instance of an algorithm, sets the properties provided and
 *       then executes it.
 * 
 *  @param algName :: The name of the algorithm required
 *  @param propertiesArray :: A single string containing properties in the 
 *                         form "Property1=Value1;Property2=Value2;..."
 *  @param version :: The version of the algorithm
 *  @return A pointer to the executed algorithm
 *          WARNING! DO NOT DELETE THIS POINTER, because it is owned
 *          by a shared pointer in the AlgorithmManager.
 * 
 *  @throw NotFoundError Thrown if algorithm requested is not registered
 *  @throw std::invalid_argument Thrown if properties string is ill-formed
 *  @throw runtime_error Thrown if algorithm cannot be executed
 */ 
IAlgorithm* FrameworkManagerImpl::exec(const std::string& algName, const std::string& propertiesArray, const int& version)
{
  // Make use of the previous method for algorithm creation and property setting
  IAlgorithm *alg = createAlgorithm(algName, propertiesArray,version);
  
  // Now execute the algorithm
  alg->execute();
  
  return alg;
}


/** Run any algorithm with a variable number of parameters
 *
 * @param algorithmName
 * @param count :: number of arguments given.
 * @return the algorithm created
 */
IAlgorithm_sptr FrameworkManagerImpl::exec(const std::string& algorithmName, int count, ...)
{
  // Create the algorithm
  Mantid::API::IAlgorithm_sptr alg;
  alg = Mantid::API::AlgorithmManager::Instance().createUnmanaged(algorithmName, -1);
  alg->initialize();
  if (!alg->isInitialized())
    throw std::runtime_error(algorithmName + " was not initialized.");

  if (count % 2 == 1)
  {
    throw std::runtime_error("Must have an even number of parameter/value string arguments");
  }

  va_list Params;
  va_start(Params, count);
  for(int i = 0; i < count; i += 2 )
  {
    std::string paramName = va_arg(Params, const char *);
    std::string paramValue = va_arg(Params, const char *);
    alg->setPropertyValue(paramName, paramValue);
  }
  va_end(Params);

  alg->execute();
  return alg;
}


/** Returns a shared pointer to the workspace requested
 * 
 *  @param wsName :: The name of the workspace
 *  @return A pointer to the workspace
 * 
 *  @throw NotFoundError If workspace is not registered with analysis data service
 */
Workspace* FrameworkManagerImpl::getWorkspace(const std::string& wsName)
{
  Workspace *space;
  try
  {
    space = AnalysisDataService::Instance().retrieve(wsName).get();
  }
  catch (Kernel::Exception::NotFoundError&)
  {
    throw Kernel::Exception::NotFoundError("Unable to retrieve workspace",wsName);
  }
  return space;
}

/** Removes and deletes a workspace from the data service store.
 * 
 *  @param wsName :: The user-given name for the workspace 
 *  @return true if the workspace was found and deleted
 * 
 *  @throw NotFoundError Thrown if workspace cannot be found
 */
bool FrameworkManagerImpl::deleteWorkspace(const std::string& wsName)
{
  bool retVal = false;
  boost::shared_ptr<Workspace> ws_sptr;
  try
  {
    ws_sptr = AnalysisDataService::Instance().retrieve(wsName);
  }
  catch(Kernel::Exception::NotFoundError&ex)
  {
    g_log.error() << ex.what() << std::endl;
    return false;
  }

  boost::shared_ptr<WorkspaceGroup> ws_grpsptr=boost::dynamic_pointer_cast<WorkspaceGroup>(ws_sptr);
  if(ws_grpsptr)
  {
    //  selected workspace is a group workspace
    ws_grpsptr->deepRemoveAll();
  }
  // Make sure we drop the references so the memory will get freed when we expect it to
  ws_sptr.reset();
  ws_grpsptr.reset();
  try
  {
    AnalysisDataService::Instance().remove(wsName);
    retVal = true;
  }
  catch (Kernel::Exception::NotFoundError&)
  {
    //workspace was not found
    g_log.error()<<"Workspace "<< wsName << " could not be found."<<std::endl;
    retVal = false;
  }
  Mantid::API::MemoryManager::Instance().releaseFreeMemory();
  return retVal;
}

} // namespace API
} // Namespace Mantid

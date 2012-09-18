#include "MantidAPI/FuncMinimizerFactory.h"
#include "MantidAPI/IFuncMinimizer.h"
#include "MantidAPI/Expression.h"
#include "MantidKernel/Logger.h"
#include "MantidKernel/LibraryManager.h"

#include <stdexcept>

namespace Mantid
{
namespace API
{

FuncMinimizerFactoryImpl::FuncMinimizerFactoryImpl() : Kernel::DynamicFactory<IFuncMinimizer>(), g_log(Kernel::Logger::get("FuncMinimizerFactory"))
{
  // we need to make sure the library manager has been loaded before we 
  // are constructed so that it is destroyed after us and thus does
  // not close any loaded DLLs with loaded algorithms in them
  Mantid::Kernel::LibraryManager::Instance();
  g_log.debug() << "FuncMinimizerFactory created." << std::endl;
}

/**
* Creates an instance of a minimizer
* @param str :: The minimizer initialization string which includes its type
*   and optionally properties: "type,prop1=value1,prop2=value2"
* @return A pointer to the created minimizer
*/
boost::shared_ptr<IFuncMinimizer> FuncMinimizerFactoryImpl::createMinimizer(const std::string& str) const
{
  Expression parser;
  parser.parse( str );
  parser.toList(); // make it a list even if it has 1 item
  const size_t n = parser.size();
  if ( n == 0 )
  {
    std::string mess = "Found empty initialization string";
    g_log.error(mess);
    throw std::invalid_argument(mess);
  }

  // create the minimizer from the type which is 
  const std::string type = parser[0].str();
  auto minimizer = create( type );

  // set the properties if there are any
  for(size_t i = 1; i < n; ++i)
  {
    auto& param = parser[i];
    if ( param.size() == 2 && param.name() == "=" )
    {
      const std::string parName = param[0].str();
      if ( minimizer->existsProperty( parName ) )
      {
        minimizer->setPropertyValue( parName, param[1].str() );
      }
    }
  }

  return minimizer;
}


} // namespace API
} // namespace Mantid
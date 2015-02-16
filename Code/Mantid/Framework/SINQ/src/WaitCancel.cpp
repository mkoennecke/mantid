/*WIKI*
This is a mini algorithm which runs until it is cancelled. Other code, for example a LiveListener,
can run this as an asynchronous child algorithm and test for cancellation by checking if this
thing still runs.
*WIKI*/

#include "MantidSINQ/WaitCancel.h"
#include <unistd.h>

namespace Mantid
{
namespace SINQ
{

  // Register the algorithm into the AlgorithmFactory
  DECLARE_ALGORITHM(WaitCancel)
  


  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  WaitCancel::WaitCancel()
  {
  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  WaitCancel::~WaitCancel()
  {
  }
  

  //----------------------------------------------------------------------------------------------
  /// Algorithm's name for identification. @see Algorithm::name
  const std::string WaitCancel::name() const { return "WaitCancel";};
  
  /// Algorithm's version for identification. @see Algorithm::version
  int WaitCancel::version() const { return 1;};
  
  /// Algorithm's category for identification. @see Algorithm::category
  const std::string WaitCancel::category() const { return "Utility";}

  //----------------------------------------------------------------------------------------------
  /// Sets documentation strings for this algorithm
  const std::string WaitCancel::summary() const
  {
    return std::string("Run until cancelled");
  }

  //----------------------------------------------------------------------------------------------
  /** Initialize the algorithm's properties.
   */
  void WaitCancel::init()
  {
  }

  //----------------------------------------------------------------------------------------------
  /** Execute the algorithm.
   */
  void WaitCancel::exec()
  {
	  while(!m_cancel){
		  usleep(500);
	  }
  }



} // namespace SINQ
} // namespace Mantid

#ifndef MANTID_API_ILIVELISTENER_H_
#define MANTID_API_ILIVELISTENER_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include <string>
#include <boost/shared_ptr.hpp>
#include <Poco/Net/SocketAddress.h>
#include "MantidKernel/DateAndTime.h"
#include "MantidAPI/DllConfig.h"

namespace Mantid
{
  namespace API
  {
    //----------------------------------------------------------------------
    // Forward declaration
    //----------------------------------------------------------------------
    class MatrixWorkspace;

    /** ILiveListener is the interface implemented by classes which connect directly to
        instrument data acquisition systems (DAS) for retrieval of 'live' data into Mantid.

        Copyright &copy; 2012 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

        This file is part of Mantid.

        Mantid is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 3 of the License, or
        (at your option) any later version.

        Mantid is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program.  If not, see <http://www.gnu.org/licenses/>.
     */
    class MANTID_API_DLL ILiveListener
    {
    public:
      //----------------------------------------------------------------------
      // Static properties
      //----------------------------------------------------------------------

      /// The name of this listener
      virtual std::string name() const = 0;
      /// Does this listener support requests for (recent) past data
      virtual bool supportsHistory() const = 0;
      /// Does this listener buffer events (true) or histogram data (false)
      virtual bool buffersEvents() const = 0;

      //----------------------------------------------------------------------
      // Actions
      //----------------------------------------------------------------------

      /** Connect to the specified address and start listening/buffering
       *  @param address   The IP address and port to contact
       *  @return True if the connection was successfully established
       */
      virtual bool connect(const Poco::Net::SocketAddress& address) = 0;

      /** Commence the collection of data from the DAS. Must be called before extractData().
       *  This method facilitates requesting an historical startpoint. Implementations
       *  that don't support this may simply start collecting data when the connect() method
       *  is called (indeed this may be required by some protocols).
       *  @param startTime The timestamp of the earliest data requested (default: now).
       *                   Ignored if not supported by an implementation.
       *                   The value of 'now' is zero for compatibility with the SNS live stream.
       */
      virtual void start(Kernel::DateAndTime startTime = Kernel::DateAndTime()) = 0;

      /** Get the data that's been buffered since the last call to this method
       *  (or since start() was called).
       *  The implementation should reset its internal buffer when this method is called
       *    - the returned workspace is for the caller to do with as they wish.
       *  IF THIS METHOD IS CALLED BEFORE start() THEN THE RESULTS ARE UNDEFINED!!!
       *  @return A pointer to the workspace containing the buffered data.
       *  TODO: Consider whether there should be a default implementation of this method
       */
      virtual boost::shared_ptr<MatrixWorkspace> extractData() = 0;

      //----------------------------------------------------------------------
      // State flags
      //----------------------------------------------------------------------

      /** Has the connection to the DAS been established?
       *  Could also be used to check for a continued connection.
       */
      virtual bool isConnected() = 0;

      /** Indicates that a reset (or period change?) signal has been received from the DAS.
       *  An example is the SNS SMS (!) statistics reset packet.
       *  A concrete listener should discard any buffered events on receipt of such a signal.
       *  It is the client's responsibility to call this method, if necessary, prior to
       *  extracting the data. Calling this method resets the flag.
       */
      virtual bool dataReset();

      // TODO: Probably some flag(s) relating to run start/stop/change

      /// Constructor.
      ILiveListener();
      /// Destructor. Should handle termination of any socket connections.
      virtual ~ILiveListener();

    protected:
      bool m_dataReset; ///< Indicates the receipt of a reset signal from the DAS.
    };

    /// Shared pointer to an ILiveListener
    typedef boost::shared_ptr<ILiveListener> ILiveListener_sptr;

  } // namespace API
} // namespace Mantid

#endif /*MANTID_API_ILIVELISTENER_H_*/

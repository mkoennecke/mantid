#ifndef MANTID_KERNEL_InternetHelper_H_
#define MANTID_KERNEL_InternetHelper_H_

#include "MantidKernel/System.h"
#include "MantidKernel/DllConfig.h"
#include "MantidKernel/ProxyInfo.h"

#include <map>

namespace Poco {
// forward declaration
class URI;

namespace Net {
// forward declaration
class HTTPClientSession;
// forward declaration
class HTTPResponse;
// forward declaration
class HTTPRequest;
}
}

namespace Mantid {
namespace Kernel {
// forward declaration
class Logger;

/** InternetHelper : A helper class for supporting access to resources through
  HTTP and HTTPS

  Copyright &copy; 2014 ISIS Rutherford Appleton Laboratory, NScD Oak Ridge
  National Laboratory & European Spallation Source

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

  File change history is stored at: <https://github.com/mantidproject/mantid>
  Code Documentation is available at: <http://doxygen.mantidproject.org>
*/
class MANTID_KERNEL_DLL InternetHelper {
public:
  InternetHelper();
  InternetHelper(const Kernel::ProxyInfo &proxy);
  virtual ~InternetHelper();

  // Convenience typedef
  typedef std::map<std::string, std::string> StringToStringMap;

  virtual int
  downloadFile(const std::string &urlFile,
               const std::string &localFilePath = "",
               const StringToStringMap &headers = StringToStringMap());
  virtual int
  sendRequest(const std::string &url, std::ostream &responseStream,
              const StringToStringMap &headers = StringToStringMap(),
              const std::string &method = std::string(),
              const std::string &body = std::string());

  Kernel::ProxyInfo &getProxy(const std::string &url);
  void clearProxy();
  void setProxy(const Kernel::ProxyInfo &proxy);

  void setTimeout(int seconds);

protected:
  virtual int sendHTTPSRequest(const std::string &url,
                               std::ostream &responseStream);
  virtual int sendHTTPRequest(const std::string &url,
                              std::ostream &responseStream);
  virtual int processErrorStates(const Poco::Net::HTTPResponse &res,
                                 std::istream &rs, const std::string &url);

private:
  void setupProxyOnSession(Poco::Net::HTTPClientSession &session,
                           const std::string &proxyUrl);
  void createRequest(Poco::URI &uri);
  int sendRequestAndProcess(Poco::Net::HTTPClientSession &session,
                            Poco::URI &uri, std::ostream &responseStream);
  int processRelocation(const Poco::Net::HTTPResponse &response,
                        std::ostream &responseStream);
  Kernel::ProxyInfo m_proxyInfo;
  bool m_isProxySet;
  int m_timeout;
  std::string m_method;
  std::string m_contentType;
  std::string m_body;
  StringToStringMap m_headers;
  Poco::Net::HTTPRequest *m_request;
};

} // namespace Kernel
} // namespace Mantid

#endif /* MANTID_KERNEL_InternetHelper_H_ */

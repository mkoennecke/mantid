#ifndef ABORTREMOTEJOB_H_
#define ABORTREMOTEJOB_H_

#include "MantidAPI/Algorithm.h"

namespace Mantid {
namespace RemoteAlgorithms {

class AbortRemoteJob : public Mantid::API::Algorithm
{
public:
  /// (Empty) Constructor
  AbortRemoteJob() : Mantid::API::Algorithm() {}
  /// Virtual destructor
  virtual ~AbortRemoteJob() {}
  /// Algorithm's name
  virtual const std::string name() const { return "AbortRemoteJob"; }
  /// Algorithm's version
  virtual int version() const { return (1); }
  /// Algorithm's category for identification
  virtual const std::string category() const { return "Remote"; }

private:
  /// Initialisation code
  void init();
  ///Execution code
  void exec();

};

} // end namespace RemoteAlgorithms
} // end namespace Mantid
#endif /*ABORTREMOTEJOB_H_*/

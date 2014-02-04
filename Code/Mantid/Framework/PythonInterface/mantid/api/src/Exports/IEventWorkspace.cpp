#include "MantidAPI/IEventWorkspace.h"
#include "MantidAPI/IEventList.h"
#include "MantidAPI/WorkspaceProperty.h"
#include "MantidPythonInterface/kernel/SharedPtrToPythonMacro.h"
#include "MantidPythonInterface/kernel/Registry/RegisterSingleValueHandler.h"
#include "MantidPythonInterface/kernel/PropertyWithValue.h"

#include <boost/python/class.hpp>

using Mantid::API::IEventWorkspace;
using Mantid::API::IEventWorkspace_sptr;
using Mantid::API::IEventList;
using Mantid::API::WorkspaceProperty;
using Mantid::API::IWorkspaceProperty;
using Mantid::Kernel::PropertyWithValue;
using namespace boost::python;

/**
 * Python exports of the Mantid::API::IEventWorkspace class.
 */
void export_IEventWorkspace()
{
  REGISTER_SHARED_PTR_TO_PYTHON(IEventWorkspace);

  class_<IEventWorkspace, bases<Mantid::API::MatrixWorkspace>, boost::noncopyable>("IEventWorkspace", no_init)
      .def("getNumberEvents", &IEventWorkspace::getNumberEvents, args("self"),
         "Returns the number of events in the workspace")
    .def("getTofMin", &IEventWorkspace::getTofMin, args("self"),
         "Returns the minimum TOF value (in microseconds) held by the workspace")
    .def("getTofMax", &IEventWorkspace::getTofMax, args("self"),
         "Returns the maximum TOF value (in microseconds) held by the workspace")
    .def("getEventList", (IEventList*(IEventWorkspace::*)(const int) ) &IEventWorkspace::getEventListPtr,
         return_internal_reference<>(), args("self", "workspace_index"), "Return the event list managing the events at the given workspace index")
    .def("clearMRU", &IEventWorkspace::clearMRU, args("self"), "Clear the most-recently-used lists")
    ;

  REGISTER_SINGLEVALUE_HANDLER(IEventWorkspace_sptr);
}


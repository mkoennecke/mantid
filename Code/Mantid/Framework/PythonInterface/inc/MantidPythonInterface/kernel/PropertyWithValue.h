#ifndef MANTID_PYTHONINTERFACE_PROPERTY_HPP_
#define MANTID_PYTHONINTERFACE_PROPERTY_HPP_

/*
    Copyright &copy; 2011 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

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

#include "MantidKernel/PropertyWithValue.h"
#include "MantidPythonInterface/kernel/Policies/DowncastReturnedValue.h"

#include <boost/python/class.hpp>
#include <boost/python/bases.hpp>
#include <boost/python/return_value_policy.hpp>

/**
 * Define a macro to export PropertyWithValue template types
 */
#define EXPORT_PROP_W_VALUE(type,suffix)   \
  boost::python::class_<Mantid::Kernel::PropertyWithValue< type >, \
     boost::python::bases<Mantid::Kernel::Property>, boost::noncopyable>("PropertyWithValue_"#suffix, boost::python::no_init) \
     .add_property("value", \
                    make_function(&Mantid::Kernel::PropertyWithValue<type>::operator(),\
                                  boost::python::return_value_policy<Mantid::PythonInterface::Policies::DowncastReturnedValue>())) \
   ;

#endif /* MANTID_PYTHONINTERFACE_PROPERTY_HPP_ */

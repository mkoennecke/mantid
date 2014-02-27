//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "MantidPythonInterface/kernel/Registry/PropertyWithValueFactory.h"
#include "MantidPythonInterface/kernel/Registry/TypedPropertyValueHandler.h"
#include "MantidKernel/PropertyWithValue.h"

#include <boost/make_shared.hpp>

#include <cassert>


namespace Mantid
{
  namespace PythonInterface
  {
    namespace Registry
    {
      namespace
      {
        /// Lookup map type
        typedef std::map<PyTypeObject const *,
                         boost::shared_ptr<PropertyValueHandler>> PyTypeIndex;

        /**
         * Initialize lookup map
         */
        void initTypeLookup(PyTypeIndex & index)
        {
          assert(index.empty());

          // Map the Python types to the best match in C++
          typedef TypedPropertyValueHandler<double> FloatHandler;
          index.insert(std::make_pair(&PyFloat_Type, boost::make_shared<FloatHandler>()));

          typedef TypedPropertyValueHandler<long> IntHandler;
          index.insert(std::make_pair(&PyInt_Type, boost::make_shared<IntHandler>()));

          typedef TypedPropertyValueHandler<bool> BoolHandler;
          index.insert(std::make_pair(&PyBool_Type, boost::make_shared<BoolHandler>()));

          typedef TypedPropertyValueHandler<std::string> StrHandler;
          index.insert(std::make_pair(&PyString_Type, boost::make_shared<StrHandler>()));

        }

        /**
         * Returns a reference to the static lookup map
         */
        const PyTypeIndex & getTypeIndex()
        {
          static PyTypeIndex index;
          if( index.empty() ) initTypeLookup(index);
          return index;
        }
      }

      /**
       * Creates a PropertyWithValue<Type> instance from the given information.
       * The python type is mapped to a C type using the mapping defined by initPythonTypeMap()
       * @param name :: The name of the property
       * @param defaultValue :: A default value for this property.
       * @param validator :: A validator object
       * @param direction :: Specifies whether the property is Input, InOut or Output
       * @returns A pointer to a new Property object
       */
      Kernel::Property *
      PropertyWithValueFactory::create(const std::string & name , const boost::python::object & defaultValue,
          const boost::python::object & validator, const unsigned int direction)
      {
        const auto &propHandle = lookup(defaultValue.ptr()->ob_type);
        return propHandle.create(name, defaultValue, validator, direction);
      }

      /**
       * Creates a PropertyWithValue<Type> instance from the given information.
       * The python type is mapped to a C type using the mapping defined by initPythonTypeMap()
       * @param name :: The name of the property
       * @param defaultValue :: A default value for this property.
       * @param direction :: Specifies whether the property is Input, InOut or Output
       * @returns A pointer to a new Property object
       */
      Kernel::Property *
      PropertyWithValueFactory::create(const std::string & name , const boost::python::object & defaultValue,
          const unsigned int direction)
      {
        boost::python::object validator; // Default construction gives None object
        return create(name, defaultValue, validator, direction);
      }


      //-------------------------------------------------------------------------
      // Private methods
      //-------------------------------------------------------------------------
      /**
       * Return a handler that maps the python type to a C++ type
       * @param pythonType :: A pointer to a PyTypeObject that represents the type
       * @returns A pointer to handler that can be used to instantiate a property
       */
      const PropertyValueHandler & PropertyWithValueFactory::lookup(PyTypeObject * const pythonType)
      {
        const PyTypeIndex & typeIndex = getTypeIndex();
        auto cit = typeIndex.find(pythonType);
        if( cit == typeIndex.end() )
          {
            std::ostringstream os;
            os << "Cannot create PropertyWithValue from Python type " << pythonType->tp_name << ". No converter registered in PropertyWithValueFactory.";
            throw std::invalid_argument(os.str());
          }
        return *(cit->second);
      }
    }
  }
}

#include "MantidDataObjects/SplittersWorkspace.h"
#include "MantidKernel/System.h"
#include "MantidAPI/Column.h"
#include "MantidAPI/TableRow.h"
#include "MantidKernel/IPropertyManager.h"

using namespace Mantid::Kernel;
using namespace Mantid::API;

namespace Mantid
{
namespace DataObjects
{

  Kernel::Logger& mg_log = Kernel::Logger::get("ITableWorkspace");

  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  SplittersWorkspace::SplittersWorkspace()
  {
    this->addColumn("long64", "start");
    this->addColumn("long64", "stop");
    this->addColumn("int", "workspacegroup");
    this->addColumn("double", "duration");
  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  SplittersWorkspace::~SplittersWorkspace()
  {
  }
  
  /*
   * Add a Splitter to
   */
  void SplittersWorkspace::addSplitter(Mantid::Kernel::SplittingInterval splitter)
  {
    int64_t start_ns = splitter.start().totalNanoseconds();
    int64_t stop_ns = splitter.stop().totalNanoseconds();
    double duration_s = static_cast<double>(stop_ns - start_ns)*1.0E-9;

    Mantid::API::TableRow row = this->appendRow();
    row << start_ns << stop_ns << splitter.index() << duration_s;

    return;
  }

  Kernel::SplittingInterval SplittersWorkspace::getSplitter(size_t index)
  {
    API::Column_const_sptr column = this->getColumn("start");
    API::TableRow row = this->getRow(index);
    int64_t start, stop;
    int wsgroup;
    row >> start;
    row >> stop;
    row >> wsgroup;

    Kernel::SplittingInterval splitter(Kernel::DateAndTime(start), Kernel::DateAndTime(stop), wsgroup);

    return splitter;
  }

  size_t SplittersWorkspace::getNumberSplitters() const
  {
    return this->rowCount();
  }


  bool SplittersWorkspace::removeSplitter(size_t index)
  {
    bool removed;
    if (index >= this->rowCount())
    {
      mg_log.error() << "Try to delete a non-existing splitter " << index << std::endl;
      removed = false;
    }
    else
    {
      this->removeRow(index);
      removed = true;
    }

    return removed;
  }



} // namespace Mantid
} // namespace DataObjects

///\cond TEMPLATE

namespace Mantid
{
  namespace Kernel
  {

    template<> DLLExport
    Mantid::DataObjects::SplittersWorkspace_sptr IPropertyManager::getValue<Mantid::DataObjects::SplittersWorkspace_sptr>(const std::string &name) const
    {
      PropertyWithValue<Mantid::DataObjects::SplittersWorkspace_sptr>* prop =
        dynamic_cast<PropertyWithValue<Mantid::DataObjects::SplittersWorkspace_sptr>*>(getPointerToProperty(name));
      if (prop)
      {
        return *prop;
      }
      else
      {
        std::string message = "Attempt to assign property "+ name +" to incorrect type. Expected SplittersWorkspace.";
        throw std::runtime_error(message);
      }
    }

    template<> DLLExport
    Mantid::DataObjects::SplittersWorkspace_const_sptr IPropertyManager::getValue<Mantid::DataObjects::SplittersWorkspace_const_sptr>(const std::string &name) const
    {
      PropertyWithValue<Mantid::DataObjects::SplittersWorkspace_sptr>* prop =
        dynamic_cast<PropertyWithValue<Mantid::DataObjects::SplittersWorkspace_sptr>*>(getPointerToProperty(name));
      if (prop)
      {
        return prop->operator()();
      }
      else
      {
        std::string message = "Attempt to assign property "+ name +" to incorrect type. Expected const SplittersWorkspace.";
        throw std::runtime_error(message);
      }
    }

  } // namespace Kernel
} // namespace Mantid
///\endcond

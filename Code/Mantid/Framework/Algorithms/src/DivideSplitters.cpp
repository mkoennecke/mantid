#include "MantidAlgorithms/DivideSplitters.h"
#include "MantidAPI/WorkspaceProperty.h"
#include "MantidAPI/TableRow.h"
#include "MantidKernel/BoundedValidator.h"

using namespace Mantid;
using namespace Mantid::API;
using namespace Mantid::DataObjects;
using namespace Mantid::Kernel;

using namespace std;

namespace Mantid
{
namespace Algorithms
{

  DECLARE_ALGORITHM(DivideSplitters)

  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  DivideSplitters::DivideSplitters()
  {
  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  DivideSplitters::~DivideSplitters()
  {
  }
  
  //----------------------------------------------------------------------------------------------
  /** Initialize documentation
    */
  void DivideSplitters::initDocs()
  {
    setWikiSummary("Divide each splitter in a splitters workspace.");
    setOptionalMessage("Divide the splitters in a SplittersWorkspace by user specification");
  }

  //----------------------------------------------------------------------------------------------
  /** Declare properties
    */
  void DivideSplitters::init()
  {
    auto inwsprop = new WorkspaceProperty<SplittersWorkspace>(
          "InputWorkspace", "Anonymous", Direction::Input);
    declareProperty(inwsprop, "Name of input SplittersWorkspace.");

    auto infowsprop = new WorkspaceProperty<TableWorkspace>(
          "InfoTableWorkspace", "", Direction::Input, PropertyMode::Optional);
    declareProperty(infowsprop, "Name of optional input information table workspace.");

    auto outwsprop = new WorkspaceProperty<SplittersWorkspace>(
          "OutputWorkspace", "", Direction::Output);
    declareProperty(outwsprop, "Name of output SplittersWorkspace.");

    auto outinfoprop = new WorkspaceProperty<TableWorkspace>("OutputInfoWorkspace", "", Direction::Output);
    declareProperty(outinfoprop, "Name of output TableWorkspace for split-to-be workspaces title.");

    boost::shared_ptr<BoundedValidator<int> > wsbc(new BoundedValidator<int>());
    wsbc->setLower(0);
    declareProperty("WorkspaceIndex", EMPTY_INT(),
                    "Index of the target workspace in input splitters workspace for furthere divide operation.");

    boost::shared_ptr<BoundedValidator<int> > segbc(new BoundedValidator<int>());
    segbc->setLower(2);
    declareProperty("NumberOfSegments", 2, "Number of segments to devidie for each splitters.");

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Main executor
    */
  void DivideSplitters::exec()
  {
    // Process user specified properties
    processAlgorithmProperties();

    // Divide workspace
    divideSplitters(m_wsIndex, m_numSegments);

    // Set output properties
    setProperty("OutputWorkspace", m_outWS);
    setProperty("OutputInfoWorkspace", m_outInfoWS);

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Divide splitters
    * @param  wsindex :: workspace index for the splitters to get divided;
    * @param  numsegments :: number of segments of one splitter to divide to.
    */
  void DivideSplitters::divideSplitters(int wsindex, int numsegments)
  {
    double invertnumsegs = 1.0/static_cast<double>(numsegments);

    size_t numrows = m_inpWS->rowCount();
    for (size_t i = 0; i < numrows; ++i)
    {
      // Determine whether to split
      int tmpindex = m_inpWS->cell<int>(i, 2);
      if (tmpindex != wsindex)
        continue;

      // Get row
      TableRow trow = m_inpWS->getRow(i);
      int64_t t_start_ns, t_stop_ns;
      trow >> t_start_ns >> t_stop_ns;

      // Divide
      double dt_ns = static_cast<double>(t_stop_ns - t_start_ns)*invertnumsegs;
      for (int j = 0; j < numsegments; ++j)
      {
        int64_t t0_ns = static_cast<int64_t>(dt_ns*static_cast<double>(j)) + t_start_ns;
        int64_t tf_ns = t0_ns + static_cast<int64_t>(dt_ns);

        TableRow newsplitrow = m_outWS->appendRow();
        newsplitrow << t0_ns << tf_ns << j << dt_ns * 1.0E-9;
      }
    }

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Process input properties and create for the output
    */
  void DivideSplitters::processAlgorithmProperties()
  {
    // Inputs
    m_inpWS = getProperty("InputWorkspace");
    m_infoWS = getProperty("InfoTableWorkspace");

    m_wsIndex = getProperty("WorkspaceIndex");
    m_numSegments = getProperty("NumberOfSegments");

    // Output
    SplittersWorkspace_sptr tempoutws(new SplittersWorkspace());
    m_outWS = tempoutws;

    TableWorkspace_sptr tablews(new TableWorkspace());
    m_outInfoWS = tablews;
    m_outInfoWS->addColumn("int", "WorkspaceGroup");
    m_outInfoWS->addColumn("str", "Description");

    // Output information workspace
    size_t numrows = m_infoWS->rowCount();
    string wsinfo("");

    //   try to get the original description
    int wsindex;
    string info;
    for (size_t i = 0; i < numrows; ++i)
    {
      TableRow tmprow = m_infoWS->getRow(i);
      tmprow >> wsindex >> info;
      if (wsindex == m_wsIndex)
      {
        wsinfo = info;
        break;
      }
    }

    for (int i = 0; i < m_numSegments; ++i)
    {
      TableRow newrow = m_outInfoWS->appendRow();
      stringstream desciptss;
      desciptss << wsinfo << ".  " << i << "-th segment of " << m_numSegments << ".";
      newrow << i << wsinfo;
    }

    return;
  }


} // namespace Algorithms
} // namespace Mantid

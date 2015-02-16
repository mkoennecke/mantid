/*WIKI*
== CombineScanGroup ==
This algorithm takes as input a WorkspaceGroup containg 2D MDHistoWorkspaces as
collected from a scan on a 2D detector. This algorithm then creates a new 3D
MDHistoWorkspace containing alla those images combined into a 3D dataset.

*WIKI*/

#include "MantidAPI/WorkspaceProperty.h"
#include "MantidAPI/IMDHistoWorkspace.h"
#include "MantidKernel/Property.h"
#include "MantidSINQ/CombineScanGroup.h"
#include "MantidGeometry/MDGeometry/IMDDimension.h"
#include "MantidGeometry/MDGeometry/MDHistoDimension.h"
#include "MantidMDEvents/MDHistoWorkspace.h"

using namespace Mantid::API;
using namespace Mantid::Kernel;
using namespace Mantid::Geometry;
using namespace Mantid::MDEvents;

namespace Mantid
{
namespace SINQ
{

  // Register the algorithm into the AlgorithmFactory
  DECLARE_ALGORITHM(CombineScanGroup)
  


  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  CombineScanGroup::CombineScanGroup()
  {
  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  CombineScanGroup::~CombineScanGroup()
  {
  }
  

  //----------------------------------------------------------------------------------------------
  /// Algorithm's name for identification. @see Algorithm::name
  const std::string CombineScanGroup::name() const { return "CombineScanGroup";};
  
  /// Algorithm's version for identification. @see Algorithm::version
  int CombineScanGroup::version() const { return 1;};
  
  /// Algorithm's category for identification. @see Algorithm::category
  const std::string CombineScanGroup::category() const { return std::string("SINQ;MDAlgorithms");}

  //----------------------------------------------------------------------------------------------
  /// Sets documentation strings for this algorithm
  const std::string CombineScanGroup::summary() const
  {
    return std::string("Combines a group of scan images into a 3D dataset");
  }

  //----------------------------------------------------------------------------------------------
  /** Initialize the algorithm's properties.
   */
  void CombineScanGroup::init()
  {
    declareProperty(new WorkspaceProperty<WorkspaceGroup>("InputWorkspaceGroup","",Direction::Input), "An input workspace group");
    declareProperty(new WorkspaceProperty<IMDHistoWorkspace>("OutputWorkspace","",Direction::Output), "An output workspace.");
  }

  //----------------------------------------------------------------------------------------------
  /** Execute the algorithm.
   */
  void CombineScanGroup::exec()
  {

		WorkspaceGroup_sptr inWS = WorkspaceGroup_sptr(getProperty("InputWorkspaceGroup"));

		checkWorkspaceGroup(inWS);

		size_t nEntries = inWS->size();
		IMDHistoWorkspace_sptr mdData = boost::dynamic_pointer_cast<IMDHistoWorkspace>(inWS->getItem(0));
		std::vector<MDHistoDimension_sptr> dimensions;
		dimensions.push_back(MDHistoDimension_sptr(new MDHistoDimension(mdData->getDimension(0).get())));
		dimensions.push_back(MDHistoDimension_sptr(new MDHistoDimension(mdData->getDimension(1).get())));
		dimensions.push_back((MDHistoDimension_sptr(new
					MDHistoDimension("Image-NO","Image-NO","",
							(coord_t)0,(coord_t)(nEntries-1),(coord_t)nEntries))));

		MDHistoWorkspace_sptr out (new MDHistoWorkspace(dimensions));

		IMDDimension_const_sptr dim = mdData->getDimension(0);
		size_t length = dim->getNBins();
		dim = mdData->getDimension(1);
		length *= dim->getNBins();

		signal_t *target = out->getSignalArray();
		signal_t *targetErrorSQ = out->getErrorSquaredArray();
		for(unsigned int i = 0; i < nEntries; i++){
			  IMDHistoWorkspace_sptr mdData = boost::dynamic_pointer_cast<IMDHistoWorkspace>(inWS->getItem(i));
			  signal_t *data = mdData->getSignalArray();
			  memcpy(target+(i*length),data, length*sizeof(signal_t));
			  memcpy(targetErrorSQ+(i*length),data, length*sizeof(signal_t));
		}

		setProperty("OutputWorkspace",out);
  }

  void CombineScanGroup::checkWorkspaceGroup(WorkspaceGroup_sptr in)
  {
	  size_t nEntries = in->size(), x = 0, y = 0;
	  unsigned int i;
	  char error[132];
	  unsigned  char initDim = 0;
	  IMDDimension_const_sptr dim;

	  if(nEntries == 0){
		  throw std::runtime_error("WorkspaceGroup is empty!");
	  }

	  for(i = 0; i < nEntries; i++){
		  IMDHistoWorkspace_sptr mdData = boost::dynamic_pointer_cast<IMDHistoWorkspace>(in->getItem(i));
		  if(mdData == NULL){
			  snprintf(error,sizeof(error),"Item %d of WorkspaceGroup is no MDHistoWorkspace",i);
			  throw std::runtime_error(error);
		  }
		  if(mdData->getNumDims() != 2){
			  snprintf(error,sizeof(error),"Item %d of WorkspaceGroup is rank %d, only 2 supported",
					  i, mdData->getNumDims());
			  throw std::runtime_error(error);
		  }
		  if(initDim == 0){
			  initDim = 1;
			  dim = mdData->getDimension(0);
			  x = dim->getNBins();
			  dim = mdData->getDimension(1);
			  y = dim->getNBins();
		  } else {
			  dim = mdData->getDimension(0);
			  if(dim->getNBins() != x){
				  snprintf(error,sizeof(error),"Item %d of WorkspaceGroup: dimension 0 mismatch: %d versus %d",
						  i,x, dim->getNBins());
				  throw std::runtime_error(error);
			  }
			  dim = mdData->getDimension(1);
			  if(dim->getNBins() != y){
				  snprintf(error,sizeof(error),"Item %d of WorkspaceGroup: dimension 0 mismatch: %d versus %d",
						  i,y, dim->getNBins());
				  throw std::runtime_error(error);
			  }
		  }
	  }
  }

} // namespace SINQ
} // namespace Mantid

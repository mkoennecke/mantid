#ifndef MANTID_ALGORITHM_AlignAndFocusPowder_H_
#define MANTID_ALGORITHM_AlignAndFocusPowder_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAPI/Algorithm.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidDataObjects/EventWorkspace.h"

namespace Mantid
{
  namespace WorkflowAlgorithms
  {
    /** 
    This is a parent algorithm that uses several different child algorithms to perform it's task.
    Takes a workspace as input and the filename of a grouping file of a suitable format.
    
    The input workspace is 
    1) Converted to d-spacing units
    2) Rebinned to a common set of bins
    3) The spectra are grouped according to the grouping file.
    
	Required Properties:
    <UL>
    <LI> InputWorkspace - The name of the 2D Workspace to take as input </LI>
    <LI> OutputWorkspace - The name of the 2D workspace in which to store the result </LI>
    </UL>

   
    @author Vickie Lynch, SNS
    @date 07/16/2012

    Copyright &copy; 2008 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

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

    File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>    
    Code Documentation is available at: <http://doxygen.mantidproject.org>
    */
    class DLLExport AlignAndFocusPowder : public API::Algorithm
    {
    public:
      /// Empty Constructor
      AlignAndFocusPowder() : API::Algorithm() {}
      /// Destructor
      virtual ~AlignAndFocusPowder() {}
      /// Algorithm's name for identification overriding a virtual method
      virtual const std::string name() const { return "AlignAndFocusPowder";}
      /// Algorithm's version for identification overriding a virtual method
      virtual int version() const { return 1;}
      /// Algorithm's category for identification overriding a virtual method
      virtual const std::string category() const { return "Workflow\\Diffraction";}
    
    private:
      /// Sets documentation strings for this algorithm
      virtual void initDocs();
      // Overridden Algorithm methods
      void init();
      void exec();
      void execEvent();
      API::MatrixWorkspace_sptr m_inputW;
      API::MatrixWorkspace_sptr m_outputW;
      DataObjects::EventWorkspace_sptr m_eventW;
      void doSortEvents(Mantid::API::Workspace_sptr ws);

    };

  } // namespace WorkflowAlgorithm
} // namespace Mantid

#endif /*MANTID_ALGORITHM_AlignAndFocusPowder_H_*/

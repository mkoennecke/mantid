/*
 * PeakHKLErrors.h
 *
 *  Created on: Jan 26, 2013
 *      Author: ruth
 */

#ifndef PEAKHKLERRORS_H_
#define PEAKHKLERRORS_H_
#include "MantidKernel/System.h"

#include "MantidAPI/IFunction.h"
#include "MantidGeometry/Instrument.h"
#include "MantidAPI/ParamFunction.h"
#include "MantidAPI/IFunction1D.h"
#include "MantidDataObjects/PeaksWorkspace.h"
#include "MantidAPI/IFunction.h"
#include "MantidGeometry/Crystal/OrientedLattice.h"
#include "MantidKernel/Matrix.h"
using Mantid::API::IFunction;
using Mantid::Geometry::Instrument;
using Mantid::DataObjects::PeaksWorkspace_sptr;

namespace Mantid
{
  namespace Crystal
  {

    /** IndexOptimizePeaks

     Description:
     This algorithm basically indexes peaks with the sample orientation matrix stored in the peaks workspace.
     The optimization is on the goniometer settings for the runs in the peaks workspace and also the sample
     orientation is optimized.

     Attributes:
        OptRuns : a list of run numbers whose sample orientations are to be optimized.  The list is separated by /.
        PeakWorkspaceName : The name of the PeaksWorkspace in the AnalysisDataService

      Parameters:
        SampleXOffset
        SampleYOffset
        SampleZOffset
        chixxx - xxx is a run number from OptRuns. This is the chi angle in degrees
        phixxx - xxx is a run number from OptRuns. This is the phi angle in degrees
        omegaxxx - xxx is a run number from OptRuns. This is the omega angle in degrees


      Workspace:
        For each peak used, there are 3 pieces of Data, one for the h int offset, one for the k int offset and one for
        the l int offset.  The x Values represent the peak number in the peaks workspace.


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

     File change history is stored at: <https://github.com/mantidproject/mantid>
     Code Documentation is available at: <http://doxygen.mantidproject.org>
     */
    class DLLExport PeakHKLErrors: public API::ParamFunction, public API::IFunction1D
    {
    public:
      PeakHKLErrors();
      virtual ~PeakHKLErrors();

      std::string name() const
      {
        return std::string("PeakHKLErrors");
      }
      ;

      virtual int version() const
      {
        return 1;
      }
      ;

      const std::string category() const
      {
        return "Calibration";
      }
      ;

      void function1D(double *out, const double *xValues, const size_t nData) const;

      void functionDeriv1D(Mantid::API::Jacobian* out, const double *xValues, const size_t nData);

      void init();

      static void  cLone( boost::shared_ptr< Geometry::ParameterMap> &pmap,
                         boost::shared_ptr< const Geometry::IComponent> component ,
                         boost::shared_ptr<const Geometry::ParameterMap> &pmapSv );

      void getRun2MatMap( PeaksWorkspace_sptr & Peaks,
                   const  std::string &OptRuns,std::map<int, Mantid::Kernel::Matrix<double> >&Res) const;
      size_t nAttributes() const
      {
        return (size_t) 2;
      }

     static Kernel::Matrix<double> DerivRotationMatrixAboutRegAxis( double theta, char axis);

     static Kernel::Matrix<double> RotationMatrixAboutRegAxis( double theta, char axis);

      boost::shared_ptr<Geometry::Instrument>
          getNewInstrument(DataObjects::PeaksWorkspace_sptr Peaks) const;

      std::vector<std::string> getAttributeNames() const
      {
        std::vector<std::string> Res;
        Res.push_back("OptRuns");
        Res.push_back("PeakWorkspaceName");
        return Res;
      }

      IFunction::Attribute getAttribute(const std::string &attName) const
      {
        if (attName == "OptRuns")
          return IFunction::Attribute(OptRuns);

        if (attName == "PeakWorkspaceName")
          return IFunction::Attribute(PeakWorkspaceName);

        throw std::invalid_argument("Not a valid attribute name");
      }

      void setAttribute(const std::string &attName, const IFunction::Attribute & value)
      {
        if (attName == "OptRuns")
        {
          OptRuns = value.asString();

          if (OptRuns.size() < 1)
            return;

          if (OptRuns.at(0) != '/')
            OptRuns = "/" + OptRuns;

          if (OptRuns.at(OptRuns.size() - 1) != '/')
            OptRuns += "/";

          if( initMode == 1 )
          {
            setUpOptRuns();
            initMode=2;
          }else if( initMode == 2)//Woops cannot do twice
            throw std::invalid_argument( "OptRuns can only be set once");

          return;

        }

        if (attName == "PeakWorkspaceName")
        {
          PeakWorkspaceName = value.asString();
          return;
        }

        throw std::invalid_argument("Not a valid attribute name");
      }

      bool hasAttribute(const std::string &attName) const
      {
        if (attName == "OptRuns")
          return true;

        if (attName == "PeakWorkspaceName")
          return true;

        return false;
      }

    private:

      std::string OptRuns;

      std::string PeakWorkspaceName;

      int initMode;//0 not invoked, 1 invoked but no OptRuns , 2 invoked and OptRuns setup

      void setUpOptRuns();

      static Kernel::Logger& g_log ;

    };
  }
}

#endif /* PEAKHKLERRORS_H_ */

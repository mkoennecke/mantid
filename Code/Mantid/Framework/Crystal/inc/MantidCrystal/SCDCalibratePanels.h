
#ifndef SCDCALIBRATEPANELS_H_
#define SCDCALIBRATEPANELS_H_

#include "MantidAPI/Algorithm.h"
#include "MantidKernel/Quat.h"
#include "MantidDataObjects/PeaksWorkspace.h"
#include "MantidGeometry/Instrument/ParameterMap.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidGeometry/IComponent.h"
#include "MantidGeometry/Instrument.h"
#include "MantidGeometry/Instrument/RectangularDetector.h"
#include <boost/lexical_cast.hpp>
#include "MantidDataObjects/Workspace2D.h"

using namespace Mantid::Kernel;
using namespace  Mantid::Geometry;
using Mantid::DataObjects::Workspace2D_sptr;

namespace Mantid
{
namespace Crystal
{
  /** SCDCalibratePanels calibrates instrument parameters for Rectangular Detectors

   *  @author Ruth Mikkelson(adapted from Isaw's Calibration code)
   *  @date   Mar 12, 2012
   *
   *  Copyright &copy; 2011 ISIS Rutherford Appleton Laboratory &
   *                   NScD Oak Ridge National Laboratory
   *
   *  This file is part of Mantid.
   *
   *  Mantid is free software; you can redistribute it and/or modify
   *  it under the terms of the GNU General Public License as published by
   *  the Free Software Foundation; either version 3 of the License, or
   *  (at your option) any later version.
   *
   *  Mantid is distributed in the hope that it will be useful,
   *  but WITHOUT ANY WARRANTY; without even the implied warranty of
   *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   *  GNU General Public License for more details.
   *
   *  You should have received a copy of the GNU General Public License
   *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
   *
   *  File change history is stored at:
   *   <https://github.com/mantidproject/mantid>
   *   Code Documentation is available at: <http://doxygen.mantidproject.org>
   */

  class SCDCalibratePanels: public Mantid::API::Algorithm
  {
  public:

    SCDCalibratePanels();

    virtual ~SCDCalibratePanels();

    virtual const std::string name() const
    {
       return "SCDCalibratePanels";
    }

     /// Algorithm's version for identification overriding a virtual method
     virtual int version() const
    {
        return 1;
    }

     /// Algorithm's category for identification overriding a virtual method
     virtual const std::string category() const
     {
       return "Crystal";
     }


  static void Quat2RotxRotyRotz(const Quat Q, double &Rotx,double &Roty,double &Rotz);


  static void FixUpBankParameterMap(  std::vector<std::string>const bankNames,
                                      boost::shared_ptr<const Instrument> NewInstrument,
                                     V3D const pos,Quat const rot,
                                     double const DetWScale, double const DetHtScale,
                                     boost::shared_ptr<const ParameterMap> const pmapOld,
                                     bool RotateCenters);


  static void FixUpSourceParameterMap( boost::shared_ptr<const Instrument> NewInstrument,
        double const L0,V3D const newSampPos, boost::shared_ptr<const ParameterMap>const  pmapOld) ;


  void CalculateGroups(std::set<std::string> &AllBankNames,
                       std::string Grouping,
                       std::string bankPrefix,
                       std::string bankingCode,
                       std::vector<std::vector<std::string> > &Groups);


  DataObjects::Workspace2D_sptr calcWorkspace( DataObjects::PeaksWorkspace_sptr & pwks,
                                               std::vector< std::string>& bankNames,
                                               double tolerance,
                                               std::vector<int>&bounds);




 static void updateBankParams(
             boost::shared_ptr<const Geometry::IComponent>  bank_const,
                boost::shared_ptr<Geometry::ParameterMap> pmap,
                boost::shared_ptr<const Geometry::ParameterMap>pmapSv) ;



  static void updateSourceParams(
        boost::shared_ptr<const Geometry::IObjComponent> bank_const,
       boost::shared_ptr<Geometry::ParameterMap> pmap,
       boost::shared_ptr<const Geometry::ParameterMap> pmapSv);

  void SaveIsawDetCal(  boost::shared_ptr<const Instrument> &NewInstrument,
                        std::set<std::string> &AllBankName,
                        double T0,std::string FileName);

  void LoadISawDetCal(
           boost::shared_ptr<const Instrument> &instrument,
           std::set<std::string> &AllBankName,double &T0,
           double &L0, std::string filename,
           std::string bankPrefixName);
  private:
    void exec ();

    void  init ();

    void initDocs ();

    static Kernel::Logger & g_log;



    boost::shared_ptr<const Instrument> GetNewCalibInstrument(
                              boost::shared_ptr<const Instrument>   instrument,
                              std::string preprocessCommand,
                              std::string preprocessFilename,
                              double &timeOffset, double &L0,
                              std::vector<std::string>  & AllBankNames);


    void CalcInitParams(  RectangularDetector_const_sptr bank_rect,
                            Instrument_const_sptr instrument,
                            Instrument_const_sptr  PreCalibinstrument,
                            double & detWidthScale0,double &detHeightScale0,
                            double &Xoffset0,double &Yoffset0,double &Zoffset0,
                            double &Xrot0,double &Yrot0,double &Zrot0);


    void  CreateFxnGetValues(Workspace2D_sptr const ws,
                         int const NGroups, std::vector<std::string> const names,
                         std::vector<double> const params,
                         std::string const BankNameString, double *out,
                         const double *xVals,const size_t nData) const;


    void SaveXmlFile(std::string const FileName, std::vector<std::vector< std::string > >const Groups,
           Instrument_const_sptr const instrument) const;

  };

}//namespace Crystal
}//namespace Mantid

#endif /* SCDCALIBRATEPANELS_H_ */

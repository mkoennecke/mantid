#ifndef MANTID_MDEVENTS_MDWS_TRANSFORMATION_H
#define MANTID_MDEVENTS_MDWS_TRANSFORMATION_H

#include "MantidMDEvents/MDTransfAxisNames.h"
#include "MantidMDEvents/MDWSDescription.h"



namespace Mantid
{
namespace MDEvents
{
 /***  The class responsible for building Momentums Transformation Matrix for CnvrtToMD algorithm
    *  from the input parameters of the algorithm and parameters, retrieved from input and 
    *  (if availible) output MD workspace
    *
    *   
      
    @date 2012-03-20

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

    File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>
    Code Documentation is available at: <http://doxygen.mantidproject.org>
*/
namespace CnvrtToMD
{
    /// enum descrines availble momentum scalings, interpreted by this class
    enum CoordScaling
    { 
        NoScaling, //< momentums in A^-1
        SingleScale, //< momentuns divided by  2*Pi/Lattice -- equivalend to d-spacing in some sence
        OrthogonalHKLScale,  //< each momentum component divided by appropriate lattice parameter; equivalent to hkl for reclenear lattice
        HKLScale,            //< non-orthogonal system for non-reclenear lattice
        NCoordScalings
    }; 
}

class DLLExport MDWSTransform
{
public:
    MDWSTransform();
 
  /** helper function which verifies if projection vectors are specified and if their values are correct when present.
      * sets default values u and v to [1,0,0] and [0,1,0] if not present or any error. */
    void setUVvectors(const std::vector<double> &ut,const std::vector<double> &vt,const std::vector<double> &wt);

   /** function provides the linear representation for the transformation matrix, which translate momentums from laboratory to crystal cartezian 
       (C)- Busing, Levi 1967 coordinate system */
   std::vector<double> getTransfMatrix(MDEvents::MDWSDescription &TargWSDescription,const std::string &QScaleRequested)const;
   std::vector<double> getTransfMatrix(MDEvents::MDWSDescription &TargWSDescription,CnvrtToMD::CoordScaling scaling)const;
  
   /// construct meaningful dimension names for Q3D case and different transformation types defined by the class
   void setQ3DDimensionsNames(MDEvents::MDWSDescription &TargWSDescription,const std::string &QScaleRequested)const;
   void setQ3DDimensionsNames(MDEvents::MDWSDescription &TargWSDescription,CnvrtToMD::CoordScaling scaling)const;
   /// construct meaningful dimension names for ModQ case and different transformation types defined by the class;
   void setModQDimensionsNames(MDEvents::MDWSDescription &TargWSDescription,const std::string &QScaleRequested)const;
  /// return the list of possible scalings for momentums
   std::vector<std::string> getQScalings()const{return m_QScalingID;}
   CnvrtToMD::CoordScaling getQScaling(const std::string &ScID)const;
private:
    bool m_isUVdefault;
    /** vectors, which describe the projection plain the target ws is based on (notional or cryst cartezian coordinate system). The transformation matrix below 
      * should bring the momentums from lab coordinate system into orthogonal, related to u,v vectors, coordinate system */
    mutable Kernel::V3D m_UProj,m_VProj,m_WProj;

  /// logger -> to provide logging, for MD dataset file operations
   static Mantid::Kernel::Logger& g_Log;

   /// string representation of QScaling ID, which would be known to user
   std::vector<std::string> m_QScalingID;
   //
   bool v3DIsDefault(const std::vector<double> &vect,const std::string &message)const;
protected: // for testing
  /// function generates "Kind of" W transformation matrix for different Q-conversion modes;
   Kernel::DblMatrix buildQTrahsf(MDEvents::MDWSDescription &TargWSDescription,CnvrtToMD::CoordScaling scaling)const;
   /// build orthogonal coordinate around two input vecotors u and v expressed in rlu;
   //std::vector<Kernel::V3D> buildOrtho3D(const Kernel::DblMatrix &BM,const Kernel::V3D &u, const Kernel::V3D &v)const;

};

}
}

#endif

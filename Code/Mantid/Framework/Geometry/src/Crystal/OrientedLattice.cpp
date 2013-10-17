#include "MantidGeometry/Crystal/OrientedLattice.h"

namespace Mantid
{
namespace Geometry
{
  using Mantid::Kernel::DblMatrix;
  using Mantid::Kernel::V3D;

  /** Default constructor
  @param Umatrix :: orientation matrix U. By default this will be identity matrix
  */
  OrientedLattice::OrientedLattice(const DblMatrix & Umatrix) : UnitCell()
  {
    if (Umatrix.isRotation()==true)
    { 
      U=Umatrix;
      UB=U*getB();
    }
    else throw std::invalid_argument("U is not a proper rotation");
  }
  OrientedLattice::OrientedLattice(const DblMatrix Umatrix) : UnitCell()
  {
    if (Umatrix.isRotation()==true)
    { 
      U=Umatrix;
      UB=U*getB();
    }
    else throw std::invalid_argument("U is not a proper rotation");
  }
 OrientedLattice::OrientedLattice() : UnitCell()
  {
    const Kernel::DblMatrix &Umatrix = Kernel::DblMatrix(3,3,true);
    if (Umatrix.isRotation()==true)
    { 
      U=Umatrix;
      UB=U*getB();
    }
    else throw std::invalid_argument("U is not a proper rotation");
  }

  /** Copy constructor
  @param other :: The OrientedLattice from which to copy information
  */    
  OrientedLattice::OrientedLattice(const OrientedLattice& other) : UnitCell(other),U(other.U),UB(other.UB)
  {
  }

  /** Constructor
  @param _a :: lattice parameter \f$ a \f$ with \f$\alpha = \beta = \gamma = 90^\circ \f$
  @param _b :: lattice parameter \f$ b \f$ with \f$\alpha = \beta = \gamma = 90^\circ \f$
  @param _c :: lattice parameter \f$ c \f$ with \f$\alpha = \beta = \gamma = 90^\circ \f$
  @param Umatrix :: orientation matrix U
  */
  OrientedLattice::OrientedLattice(const double _a,const double _b,const double _c,
                                   const DblMatrix &Umatrix) : UnitCell(_a,_b,_c)
  {
    if (Umatrix.isRotation()==true)
    { 
      U=Umatrix;
      UB=U*getB();
    }
    else throw std::invalid_argument("U is not a proper rotation");
  }
  OrientedLattice::OrientedLattice(const double _a,const double _b,const double _c,
                                   const DblMatrix Umatrix) : UnitCell(_a,_b,_c)
  {
    if (Umatrix.isRotation()==true)
    { 
      U=Umatrix;
      UB=U*getB();
    }
    else throw std::invalid_argument("U is not a proper rotation");
  }

  /** Constructor
  @param _a :: lattice parameter \f$ a \f$
  @param _b :: lattice parameter \f$ b \f$
  @param _c :: lattice parameter \f$ c \f$
  @param _alpha :: lattice parameter \f$ \alpha \f$
  @param _beta :: lattice parameter \f$ \beta \f$
  @param _gamma :: lattice parameter \f$ \gamma \f$
  @param angleunit :: units for angle, of type #AngleUnits. Default is degrees.
  @param Umatrix :: orientation matrix U
  */
   OrientedLattice::OrientedLattice(const double _a,const double _b,const double _c,const double _alpha,
                                   const double _beta,const double _gamma, DblMatrix Umatrix,
                                   const int angleunit)
  :UnitCell(_a,_b,_c,_alpha,_beta,_gamma,angleunit)
  {
    if (Umatrix.isRotation()==true)
    { 
      U=Umatrix;
      UB=U*getB();
    }
    else throw std::invalid_argument("U is not a proper rotation");
  }
  OrientedLattice::OrientedLattice(const double _a,const double _b,const double _c,const double _alpha,
                                   const double _beta,const double _gamma, 
                                   const int angleunit)
  :UnitCell(_a,_b,_c,_alpha,_beta,_gamma,angleunit)
  {
    const Kernel::DblMatrix &Umatrix = Kernel::DblMatrix(3,3,true);
    if (Umatrix.isRotation()==true)
    { 
      U=Umatrix;
      UB=U*getB();
    }
    else throw std::invalid_argument("U is not a proper rotation");
 }

  /** UnitCell constructor
  @param uc :: UnitCell
  @param Umatrix :: orientation matrix U. By default this will be identity matrix
  */
  OrientedLattice::OrientedLattice(const UnitCell & uc, const DblMatrix & Umatrix)
    : UnitCell(uc),U(Umatrix)
  {
    if (Umatrix.isRotation()==true)
    { 
      U=Umatrix;
      UB=U*getB();
    }
    else throw std::invalid_argument("U is not a proper rotation");
  }
  OrientedLattice::OrientedLattice(const UnitCell uc, const DblMatrix Umatrix)
    : UnitCell(uc),U(Umatrix)
  {
    if (Umatrix.isRotation()==true)
    { 
      U=Umatrix;
      UB=U*getB();
    }
    else throw std::invalid_argument("U is not a proper rotation");
  }

  /// Destructor
  OrientedLattice::~OrientedLattice()
  {
  } 


  /// Get the U matrix
  /// @return U :: U orientation matrix
  const DblMatrix& OrientedLattice::getU() const
  {
    return U;
  }
  
  /** Get the UB matrix.
   The UB Matrix uses the inelastic convention:
     q = UB . (hkl)
   where q is the wavevector transfer of the LATTICE (not the neutron).
     and |q| = 1.0/d_spacing

   @return UB :: UB orientation matrix
   */
  const DblMatrix& OrientedLattice::getUB() const
  {
    return UB;
  }

  /** Sets the U matrix
    @param newU :: the new U matrix
    @param force :: If true, do not check that U matrix is valid
    */
  void OrientedLattice::setU(const DblMatrix& newU, const bool force)
  {
    if (force || newU.isRotation())
    {
      U=newU;
      UB=U*getB();
    }
    else throw std::invalid_argument("U is not a proper rotation");
  }

  /** Sets the UB matrix and recalculates lattice parameters
    @param newUB :: the new UB matrix*/
  void OrientedLattice::setUB(const DblMatrix& newUB)
  {
    if (UB.determinant()>0)
    {
      UB=newUB;
      DblMatrix newGstar,B;
      newGstar=newUB.Tprime()*newUB;
      this->recalculateFromGstar(newGstar);
      B=this->getB();
      B.Invert();
      U=newUB*B;
    }
    else throw std::invalid_argument("determinant of UB is not greater than 0");
  }


  /** Calculate the hkl corresponding to a given Q-vector
   * @param Q :: Q-vector in $AA^-1 in the sample frame
   * @return a V3D with H,K,L
   */
  V3D OrientedLattice::hklFromQ(const V3D & Q) const
  {
    DblMatrix UBinv = this->getUB();
    UBinv.Invert();
    V3D out = UBinv*Q; //transform back to HKL
    return out;
  }


  /** gets a vector along beam direction when goniometers are at 0. Note, this vector is not unique, but
    all vectors can be obtaineb by multiplying with a scalar
    @return u :: V3D vector along beam direction*/
    Kernel::V3D OrientedLattice::getuVector()
    {
      V3D z(0,0,1);
      DblMatrix UBinv=UB;
      UBinv.Invert();
      return UBinv*z;
    }

  /** gets a vector in the horizontal plane, perpendicular to the beam direction when
    goniometers are at 0. Note, this vector is not unique, but all vectors can be obtaineb by multiplying with a scalar
    @return v :: V3D vector perpendicular to the beam direction, in the horizontal plane*/
    Kernel::V3D OrientedLattice::getvVector()
    {
      V3D x(1,0,0);
      DblMatrix UBinv=UB;
      UBinv.Invert();
      return UBinv*x;
    }

 /**  Set the U rotation matrix, to provide the transformation, which translate an 
   *  arbitrary vector V expressed in RLU (hkl) 
   *  into another coordinate system defined by vectors u and v, expressed in RLU (hkl) 
   *  Author: Alex Buts
   *  @param u :: first vector of new coordinate system (in hkl units)
   *  @param v :: second vector of the new coordinate system 
   *  @return the U matrix calculated
   *  The transformation from old coordinate system to new coordinate system is performed by 
   *  the whole UB matrix
   **/
  const DblMatrix & OrientedLattice::setUFromVectors(const V3D &u, const V3D &v)
  {
    const DblMatrix & BMatrix=this->getB();
    V3D buVec = BMatrix*u;
    V3D bvVec = BMatrix*v;
    //try to make an orthonormal system
    if (buVec.norm2()<1e-10) throw std::invalid_argument("|B.u|~0");
    if (bvVec.norm2()<1e-10) throw std::invalid_argument("|B.v|~0");
    buVec.normalize(); // 1st unit vector, along Bu
    V3D bwVec = buVec.cross_prod(bvVec);
    if (bwVec.normalize()<1e-5) throw std::invalid_argument("u and v are parallel"); // 3rd unit vector, perpendicular to Bu,Bv
    bvVec = bwVec.cross_prod(buVec); // 2nd unit vector, perpendicular to Bu, in the Bu,Bv plane
    DblMatrix tau(3,3),lab(3,3),U(3,3);
    /*lab      = U tau
    / 0 1 0 \     /bu[0] bv[0] bw[0]\
    | 0 0 1 | = U |bu[1] bv[1] bw[1]|
    \ 1 0 0 /     \bu[2] bv[2] bw[2]/
     */
    lab[0][1]=1.;
    lab[1][2]=1.;
    lab[2][0]=1.;
    tau[0][0]=buVec[0];
    tau[0][1]=bvVec[0];
    tau[0][2]=bwVec[0];
    tau[1][0]=buVec[1];
    tau[1][1]=bvVec[1];
    tau[1][2]=bwVec[1];
    tau[2][0]=buVec[2];
    tau[2][1]=bvVec[2];
    tau[2][2]=bwVec[2];
    tau.Invert();
    U=lab*tau;
    this->setU(U);
    return getU();
  }


  /** Save the object to an open NeXus file.
   * @param file :: open NeXus file
   * @param group :: name of the group to create
   */
  void OrientedLattice::saveNexus(::NeXus::File * file, const std::string & group) const
  {
    file->makeGroup(group, "NXcrystal", 1);
    file->writeData("unit_cell_a", this->a());
    file->writeData("unit_cell_b", this->b());
    file->writeData("unit_cell_c", this->c());
    file->writeData("unit_cell_alpha", this->alpha());
    file->writeData("unit_cell_beta", this->beta());
    file->writeData("unit_cell_gamma", this->gamma());
    // Save the UB matrix
    std::vector<double> ub = this->UB.getVector();
    std::vector<int> dims(2,3); // 3x3 matrix
    file->writeData("orientation_matrix", ub, dims);

    file->closeGroup();
  }

  /** Load the object from an open NeXus file.
   * @param file :: open NeXus file
   * @param group :: name of the group to open
   */
  void OrientedLattice::loadNexus(::NeXus::File * file, const std::string & group)
  {
    file->openGroup(group, "NXcrystal");
    std::vector<double> ub;
    file->readData("orientation_matrix", ub);
    // Turn into a matrix
    DblMatrix ubMat(ub);
    // This will set the lattice parameters and the U matrix:
    this->setUB(ubMat);
    file->closeGroup();
  }

}//Namespace Geometry
}//Namespace Mantid

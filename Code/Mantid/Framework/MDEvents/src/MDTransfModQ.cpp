#include "MantidMDEvents/MDTransfModQ.h"
#include "MantidKernel/RegistrationHelper.h"

namespace Mantid
{
  namespace MDEvents
  {
    // register the class, whith conversion factory under ModQ name
    DECLARE_MD_TRANSFID(MDTransfModQ,|Q|);

    /** method calculates the unigs, the transformation expects input ws to be in. If input ws is in different units, 
    the WS data will be converted into the units requested on-fly. 
    */
    const std::string MDTransfModQ::inputUnitID(CnvrtToMD::EModes dEmode, API::MatrixWorkspace_const_sptr inWS)const
    {
      UNUSED_ARG(inWS);
      switch(dEmode)
      {
      case(CnvrtToMD::Elastic):  return "Momentum";
      case(CnvrtToMD::Direct):  return "DeltaE";
      case(CnvrtToMD::Indir):  return "DeltaE";
      default:
        throw(std::invalid_argument(" MDTransfModQ::inputUnitID: this class supports only conversion in Elastic and Inelastic energy transfer modes"));
      }
    }
    /** method returns number of matrix dimensions calculated by this class
    * as function of energy analysis mode   */
    unsigned int MDTransfModQ::getNMatrixDimensions(CnvrtToMD::EModes mode,API::MatrixWorkspace_const_sptr inWS)const
    {
      UNUSED_ARG(inWS);
      switch(mode)
      {
      case(CnvrtToMD::Direct):  return 2;
      case(CnvrtToMD::Indir):   return 2;
      case(CnvrtToMD::Elastic): return 1;
      default: throw(std::invalid_argument("Unknow or unsupported energy conversion mode"));
      }
    }



    bool MDTransfModQ::calcMatrixCoord(const double& x,std::vector<coord_t> &Coord)const
    {
      if(m_Emode == CnvrtToMD::Elastic)
      {
        return calcMatrixCoordElastic(x,Coord);
      }else{
        return calcMatrixCoordInelastic(x,Coord);
      }
    }

    /** Method fills-in all additional properties requested by user and not defined by matrix workspace itselt. 
    *  it fills in [nd - (1 or 2 -- depending on emode)] values into Coord vector;
    *
    *@param Coord -- input-output vector of MD-coordinates
    *@param nd    -- number of current dimensions
    *
    *@returns     -- Coord vector with nd-(1 or 2, depending on emode) values of MD coordinates
    */
    bool MDTransfModQ::calcGenericVariables(std::vector<coord_t> &Coord, size_t nd)
    {      
      // sanity check. If fails, something went fundamentally wrong
      if(m_NMatrixDim+m_AddDimCoordinates.size()!=nd)
      {
        std::string ERR="Number of matrix dimensions: "+boost::lexical_cast<std::string>(m_NMatrixDim)+
          " plus number of additional dimensions: "+boost::lexical_cast<std::string>(m_AddDimCoordinates.size())+
          " not equal to number of workspace dimensions: "+boost::lexical_cast<std::string>(nd);
        throw(std::invalid_argument(ERR));
      }

      // in Elastic case, 1  coordinate (|Q|) came from workspace
      // in inelastic 2 coordinates (|Q| dE) came from workspace. All other are defined by properties. 
      // m_NMatrixDim is either 1 in elastic case or 2 in inelastic
      size_t ic(0);
      for(size_t i=m_NMatrixDim;i<nd;i++)
      {
        if(m_AddDimCoordinates[ic]<m_DimMin[i] || m_AddDimCoordinates[ic]>=m_DimMax[i])return false;
        Coord[i]= m_AddDimCoordinates[ic];
        ic++;
      }
      return true;
    }
    /** Method updates the value of preprocessed detector coordinates in Q-space, used by other functions 
    *@param i -- index of the detector, which corresponds to the spectra to process. 
    * 
    */
    bool MDTransfModQ::calcYDepCoordinates(std::vector<coord_t> &Coord,size_t i)
    {
      UNUSED_ARG(Coord); 
      m_ex = (m_Det+i)->X();
      m_ey = (m_Det+i)->Y();
      m_ez = (m_Det+i)->Z();
      return true;
    }
    /** function calculates workspace-dependent coordinates in inelastic case. 
    * Namely, it calculates module of Momentum transfer and the Energy 
    * transfer and put them into initial positions (0 and 1) in the Coord vector
    *
    *@Param     E_tr   input energy transfer
    *@returns   &Coord  vector of MD coordinates with filled in momentum and energy transfer 

    *@returns   true if all momentum and energy are within the limits requested by the algorithm and false otherwise. 
    *
    * it also uses preprocessed detectors positions, which are calculated by PreprocessDetectors algorithm and set up by 
    * calcYDepCoordinates(std::vector<coord_t> &Coord,size_t i) method.    */    
    bool MDTransfModQ::calcMatrixCoordInelastic(const double& E_tr,std::vector<coord_t> &Coord)const
    {
      if(E_tr<m_DimMin[1]||E_tr>=m_DimMax[1])return false;
      Coord[1]    =(coord_t)E_tr;
      double k_tr;
      // get module of the wavevector for scattered neutrons
      if(this->m_Emode==CnvrtToMD::Direct)
      {
        k_tr=sqrt((m_Ei-E_tr)/PhysicalConstants::E_mev_toNeutronWavenumberSq);
      }else{
        k_tr=sqrt((m_Ei+E_tr)/PhysicalConstants::E_mev_toNeutronWavenumberSq);
      }

      double  qx  =  -m_ex*k_tr;                
      double  qy  =  -m_ey*k_tr;       
      double  qz  = m_Ki - m_ez*k_tr;
      // transformation matrix has to be here for "Crystal AS Powder conversion mode, further specialization possible if "powder" mode defined"
      double Qx  = (m_RotMat[0]*qx+m_RotMat[1]*qy+m_RotMat[2]*qz);
      double Qy  = (m_RotMat[3]*qx+m_RotMat[4]*qy+m_RotMat[5]*qz); 
      double Qz  = (m_RotMat[6]*qx+m_RotMat[7]*qy+m_RotMat[8]*qz);

      double Qsq = Qx*Qx+Qy*Qy+Qz*Qz;
      if(Qsq < m_DimMin[0]||Qsq>=m_DimMax[0])return false;
      Coord[0]   = (coord_t)sqrt(Qsq);

      return true;

    }
    /** function calculates workspace-dependent coordinates in elastic case. 
    * Namely, it calculates module of Momentum transfer
    * put it into specified (0) position in the Coord vector
    *
    *@Param     k0   module of input momentum
    *@returns   &Coord  vector of MD coordinates with filled in momentum and energy transfer 

    *@returns   true if momentum is within the limits requested by the algorithm and false otherwise. 
    *
    * it uses preprocessed detectors positions, which are calculated by PreprocessDetectors algorithm and set up by 
    * calcYDepCoordinates(std::vector<coord_t> &Coord,size_t i) method. */    
    bool MDTransfModQ::calcMatrixCoordElastic(const double& k0,std::vector<coord_t> &Coord)const
    {

      double  qx  =  -m_ex*k0;                
      double  qy  =  -m_ey*k0;       
      double  qz  = (1 - m_ez)*k0;
      // transformation matrix has to be here for "Crystal AS Powder mode, further specialization possible if powder mode is defined "
      double Qx  = (m_RotMat[0]*qx+m_RotMat[1]*qy+m_RotMat[2]*qz);
      double Qy  = (m_RotMat[3]*qx+m_RotMat[4]*qy+m_RotMat[5]*qz); 
      double Qz  = (m_RotMat[6]*qx+m_RotMat[7]*qy+m_RotMat[8]*qz);

      double Qsq = Qx*Qx+Qy*Qy+Qz*Qz;
      if(Qsq < m_DimMin[0]||Qsq>=m_DimMax[0])return false;
      Coord[0]   = (coord_t)sqrt(Qsq);
      return true;

    }

    /** function initalizes all variables necessary for converting workspace variables into MD variables in ModQ (elastic/inelastic) cases  */
    void MDTransfModQ::initialize(const MDWSDescription &ConvParams)
    { 
      //********** Generic part of initialization, common for elastic and inelastic modes:
      //   pHost      = &Conv;
      // get transformation matrix (needed for CrystalAsPoder mode)
      m_RotMat = ConvParams.getTransfMatrix();

      // get pointer to the positions of the detectors
      std::vector<Kernel::V3D> const & DetDir = ConvParams.getDetectors()->getDetDir();
      m_Det = &DetDir[0];     //

      // get min and max values defined by the algorithm. 
      ConvParams.getMinMax(m_DimMin,m_DimMax);
      // m_DimMin/max here are momentums and they are verified on momentum squared base         
      if(m_DimMin[0]<0)m_DimMin[0]=0;
      if(m_DimMax[0]<0)m_DimMax[0]=0;

      // m_DimMin here is a momentum and it is verified on momentum squared base
      m_DimMin[0]*=m_DimMin[0];
      m_DimMax[0]*=m_DimMax[0];
      if(std::fabs(m_DimMin[0]-m_DimMax[0])<FLT_EPSILON||m_DimMax[0]<m_DimMin[0])
      {
        std::string ERR = "ModQ coordinate transformation: Min Q^2 value: "+boost::lexical_cast<std::string>(m_DimMin[0])+
          " is more or equal then Max Q^2 value: "+boost::lexical_cast<std::string>(m_DimMax[0]);
        throw(std::invalid_argument(ERR));
      }
      this->m_AddDimCoordinates = ConvParams.getAddCoord();

      //************   specific part of the initialization, dependent on emode:
      m_Emode      = ConvParams.getEMode();
      m_NMatrixDim = getNMatrixDimensions(m_Emode);
      if(m_Emode == CnvrtToMD::Direct||m_Emode == CnvrtToMD::Indir)
      {
        // energy needed in inelastic case
        m_Ei  =  ConvParams.getEi();
        // the wave vector of incident neutrons;
        m_Ki=sqrt(m_Ei/PhysicalConstants::E_mev_toNeutronWavenumberSq); 
      }else{
        if (m_Emode != CnvrtToMD::Elastic)throw(std::invalid_argument("MDTransfModQ::initialize::Unknown energy conversion mode"));
      }

    }
    /**method returns default ID-s for ModQ elastic and inelastic modes. The ID-s are related to the units, 
    * this class produces its ouptut in. 
    *@param Emode   -- energy conversion mode

    *@returns       -- vector of default dimension ID-s for correspondent energy conversion mode. 
    The position of each dimID in the vector corresponds to the position of each MD coordinate in the Coord vector
    */
    std::vector<std::string> MDTransfModQ::getDefaultDimID(CnvrtToMD::EModes dEmode, API::MatrixWorkspace_const_sptr inWS)const
    {
      UNUSED_ARG(inWS);
      std::vector<std::string> default_dim_ID;
      switch(dEmode)
      {
      case(CnvrtToMD::Elastic):
        {
          default_dim_ID.resize(1);
          break;
        }
      case(CnvrtToMD::Direct):
      case(CnvrtToMD::Indir):
        {
          default_dim_ID.resize(2);
          default_dim_ID[1]= "DeltaE";
          break;
        }
      default:
        throw(std::invalid_argument("MDTransfModQ::getDefaultDimID::Unknown energy conversion mode"));
      }
      default_dim_ID[0]="|Q|";

      return default_dim_ID;
    }


    /**function returns units ID-s which this transformation prodiuces its ouptut. 
    * @param Emode   -- energy conversion mode
    *
    * It is Momentum and DelteE in inelastic modes   */
    std::vector<std::string> MDTransfModQ::outputUnitID(CnvrtToMD::EModes dEmode, API::MatrixWorkspace_const_sptr inWS)const
    {
      UNUSED_ARG(inWS);
      std::vector<std::string> UnitID = this->getDefaultDimID(dEmode,inWS);
      //TODO: is it really momentum transfer, as MomentumTransfer units are seems bound to elastic mode only (at least accorting to Units description on Wiki)?
      if(dEmode==CnvrtToMD::Elastic)
      {
        UnitID[0] = "Momentum";
      }else{
        UnitID[0] = "MomentumTransfer";
      }
      return UnitID;
    }


    /// constructor;
    MDTransfModQ::MDTransfModQ():
    m_Det(NULL),m_NMatrixDim(-1)
    {}    

  } // End MDAlgorighms namespace
} // End Mantid namespace

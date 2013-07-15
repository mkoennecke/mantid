/*WIKI*
Converts a 2D workspace from units of spectrum number, energy transfer to the
intensity as a function of momentum transfer and energy. The rebinning is done
as a weighted sum of overlapping polygons with fractional area tracking. The
result is stored in a new workspace type: '''RebinnedOutput'''. The new workspace
presents the data as the fractional counts divided by the fractional area.
The biggest consequence of this method is that in places where there are no
counts and no acceptance (no fractional areas), '''nan'''s will result.

The algorithm operates in non-PSD mode by default. This means that all azimuthal
angles and widths are forced to zero. PSD mode will determine the azimuthal
angles and widths from the instrument geometry. This mode is activated by
placing the following named parameter in a Parameter file: ''detector-neighbour-offset''.
The integer value of this parameter should be the number of pixels that separates
two pixels at the same vertical position in adjacent tubes.
*WIKI*/
//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAlgorithms/SofQW3.h"
#include "MantidAlgorithms/SofQW.h"
#include "MantidAPI/NumericAxis.h"
#include "MantidAPI/SpectraAxis.h"
#include "MantidAPI/WorkspaceFactory.h"
#include "MantidGeometry/Instrument/DetectorGroup.h"
#include "MantidGeometry/Math/LaszloIntersection.h"
#include "MantidGeometry/Math/Quadrilateral.h"
#include "MantidGeometry/Math/Vertex2D.h"
#include "MantidKernel/UnitFactory.h"
#include "MantidKernel/VectorHelper.h"

namespace Mantid
{
namespace Algorithms
{
  // Setup typedef for later use
  typedef std::map<specid_t, Mantid::Kernel::V3D>  SpectraDistanceMap;
  typedef Geometry::IDetector_const_sptr DetConstPtr;

  // Register the algorithm into the AlgorithmFactory
  DECLARE_ALGORITHM(SofQW3)

  using namespace Mantid::Kernel;
  using namespace Mantid::API;
  using namespace Mantid::DataObjects;
  using Geometry::IDetector_const_sptr;
  using Geometry::DetectorGroup;
  using Geometry::DetectorGroup_const_sptr;
  using Geometry::ConvexPolygon;
  using Geometry::Quadrilateral;
  using Geometry::Vertex2D;

  /// Default constructor
  SofQW3::SofQW3()
    : Rebin2D(),
      m_Qout(), m_thetaWidth(0.0), m_detNeighbourOffset(-1)
  {
  }

  //----------------------------------------------------------------------------------------------
  /// Sets documentation strings for this algorithm
  void SofQW3::initDocs()
  {
    this->setWikiSummary("Calculate the intensity as a function of momentum transfer and energy");
    this->setOptionalMessage("Calculate the intensity as a function of momentum transfer and energy.");
  }

  /**
   * @return the name of the Algorithm
   */
  const std::string SofQW3::name() const
  {
    return "SofQW3";
  }

  /**
   * @return the version number of the Algorithm
   */
  int SofQW3::version() const
  {
    return 1;
  }

  /**
   * @return the category list for the Algorithm
   */
  const std::string SofQW3::category() const
  {
    return "Inelastic";
  }

  /**
   * Initialize the algorithm
   */
  void SofQW3::init()
  {
    SofQW::createInputProperties(*this);
  }

  /**
   * Execute the algorithm.
   */
  void SofQW3::exec()
  {
    MatrixWorkspace_sptr inputWS = getProperty("InputWorkspace");
    // Do the full check for common binning
    if( !WorkspaceHelpers::commonBoundaries(inputWS) )
    {
      throw std::invalid_argument("The input workspace must have common binning across all spectra");
    }

    RebinnedOutput_sptr outputWS = this->setUpOutputWorkspace(inputWS,
                                                 getProperty("QAxisBinning"),
                                                              m_Qout);
    g_log.debug() << "Workspace type: " << outputWS->id() << std::endl;
    setProperty("OutputWorkspace", outputWS);
    const size_t nEnergyBins = inputWS->blocksize();
    const size_t nHistos = inputWS->getNumberHistograms();

    // Progress reports & cancellation
    const size_t nreports(nHistos * nEnergyBins);
    m_progress = boost::shared_ptr<API::Progress>(new API::Progress(this, 0.0,
                                                                    1.0,
                                                                    nreports));

    // Compute input caches
    m_EmodeProperties.initCachedValues(inputWS,this);

    std::vector<double> par = inputWS->getInstrument()->getNumberParameter("detector-neighbour-offset");
    if (par.empty())
    {
      // Index theta cache
      this->initThetaCache(inputWS);
    }
    else
    {
      g_log.debug() << "Offset: " << par[0] << std::endl;
      this->m_detNeighbourOffset = static_cast<int>(par[0]);
      this->getValuesAndWidths(inputWS);
    }

    const MantidVec & X = inputWS->readX(0);

    int emode = m_EmodeProperties.m_emode;
    PARALLEL_FOR2(inputWS, outputWS)
    for (int64_t i = 0; i < static_cast<int64_t>(nHistos); ++i) // signed for openmp
    {
      PARALLEL_START_INTERUPT_REGION

      DetConstPtr detector = inputWS->getDetector(i);
      if (detector->isMasked() || detector->isMonitor())
      {
        continue;
      }

      double theta = this->m_theta[i];
      double phi = 0.0;
      double thetaWidth = 0.0;
      double phiWidth = 0.0;
      // Non-PSD mode
      if (par.empty())
      {
        thetaWidth = this->m_thetaWidth;
      }
      // PSD mode
      else
      {
        phi = this->m_phi[i];
        thetaWidth = this->m_thetaWidths[i];
        phiWidth = this->m_phiWidths[i];
      }

      double thetaHalfWidth = 0.5 * thetaWidth;
      double phiHalfWidth = 0.5 * phiWidth;

      const double thetaLower = theta - thetaHalfWidth;
      const double thetaUpper = theta + thetaHalfWidth;

      const double phiLower = phi - phiHalfWidth;
      const double phiUpper = phi + phiHalfWidth;

      const double efixed = m_EmodeProperties.getEFixed(detector);

      for(size_t j = 0; j < nEnergyBins; ++j)
      {
        m_progress->report("Computing polygon intersections");
        // For each input polygon test where it intersects with
        // the output grid and assign the appropriate weights of Y/E
        const double dE_j = X[j];
        const double dE_jp1 = X[j+1];

        const V2D ll(dE_j, this->calculateQ(efixed, emode,dE_j, thetaLower, phiLower));
        const V2D lr(dE_jp1, this->calculateQ(efixed,emode, dE_jp1, thetaLower, phiLower));
        const V2D ur(dE_jp1, this->calculateQ(efixed,emode, dE_jp1, thetaUpper, phiUpper));
        const V2D ul(dE_j, this->calculateQ(efixed,emode, dE_j, thetaUpper, phiUpper));
        Quadrilateral inputQ = Quadrilateral(ll, lr, ur, ul);

        this->rebinToFractionalOutput(inputQ, inputWS, i, j, outputWS, m_Qout);
      }

      PARALLEL_END_INTERUPT_REGION
    }
    PARALLEL_CHECK_INTERUPT_REGION

    outputWS->finalize();
    this->normaliseOutput(outputWS, inputWS);
  }


  /**
   * Calculate the Q value for a given set of energy transfer, scattering
   * and azimuthal angle.
   * @param efixed :: An fixed energy value
   * @param emode  :: the energy evaluation mode
   * @param deltaE :: The energy change
   * @param twoTheta :: The value of the scattering angle
   * @param azimuthal :: The value of the azimuthual angle
   * @return The value of Q
   */
  double SofQW3::calculateQ(const double efixed,int emode, const double deltaE,
                            const double twoTheta, const double azimuthal) const
  {
    double ki = 0.0;
    double kf = 0.0;
    if (emode == 1)
      {
        ki = std::sqrt(efixed * SofQW::energyToK());
        kf = std::sqrt((efixed - deltaE) * SofQW::energyToK());
      }
    else if(emode == 2)
      {
        ki = std::sqrt((deltaE + efixed) * SofQW::energyToK());
        kf = std::sqrt(efixed * SofQW::energyToK());
      }
    const double Qx = ki - kf * std::cos(twoTheta);
    const double Qy = -kf * std::sin(twoTheta) * std::cos(azimuthal);
    const double Qz = -kf * std::sin(twoTheta) * std::sin(azimuthal);
    return std::sqrt(Qx * Qx + Qy * Qy + Qz * Qz);
  }
  /**
   * A map detector ID and Q ranges
   * This method looks unnecessary as it could be calculated on the fly but
   * the parallelization means that lazy instantation slows it down due to the
   * necessary CRITICAL sections required to update the cache. The Q range
   * values are required very frequently so the total time is more than
   * offset by this precaching step
   */
  void SofQW3::initThetaCache(API::MatrixWorkspace_const_sptr workspace)
  {
    const size_t nhist = workspace->getNumberHistograms();
    this->m_theta = std::vector<double>(nhist);
    size_t ndets(0);
    double minTheta(DBL_MAX), maxTheta(-DBL_MAX);

    for(int64_t i = 0 ; i < (int64_t)nhist; ++i) //signed for OpenMP
    {

      m_progress->report("Calculating detector angles");
      IDetector_const_sptr det;
      try
      {
        det = workspace->getDetector(i);
        // Check to see if there is an EFixed, if not skip it
        try
        {
            m_EmodeProperties.getEFixed(det);
        }
        catch(std::runtime_error&)
        {
          det.reset();
        }
      }
      catch(Kernel::Exception::NotFoundError&)
      {
        // Catch if no detector. Next line tests whether this happened - test placed
        // outside here because Mac Intel compiler doesn't like 'continue' in a catch
        // in an openmp block.
      }
      // If no detector found, skip onto the next spectrum
      if( !det || det->isMonitor() )
      {
        this->m_theta[i] = -1.0; // Indicates a detector to skip
      }
      else
      {
        ++ndets;
        const double theta = workspace->detectorTwoTheta(det);
        this->m_theta[i] = theta;
        if( theta < minTheta )
        {
          minTheta = theta;
        }
        else if( theta > maxTheta )
        {
          maxTheta = theta;
        }
      }
    }

    this->m_thetaWidth = (maxTheta - minTheta)/static_cast<double>(ndets);
    g_log.information() << "Calculated detector width in theta=" << (m_thetaWidth*180.0/M_PI) << " degrees.\n";
  }

  /**
   * Function that retrieves the two-theta and azimuthal angles from a given
   * detector. It then looks up the nearest neighbours. Using those detectors,
   * it calculates the two-theta and azimuthal angle widths.
   * @param workspace : the workspace containing the needed detector information
   */
  void SofQW3::getValuesAndWidths(API::MatrixWorkspace_const_sptr workspace)
  {
    // Trigger a build of the nearst neighbors outside the OpenMP loop
    const int numNeighbours = 4;
    const size_t nHistos = workspace->getNumberHistograms();
    g_log.debug() << "Number of Histograms: " << nHistos << std::endl;

    this->m_theta = std::vector<double>(nHistos);
    this->m_thetaWidths = std::vector<double>(nHistos);
    this->m_phi = std::vector<double>(nHistos);
    this->m_phiWidths = std::vector<double>(nHistos);

    for (size_t i = 0; i < nHistos; ++i)
    {
      m_progress->report("Calculating detector angular widths");
      DetConstPtr detector = workspace->getDetector(i);
      g_log.debug() << "Current histogram: " << i << std::endl;
      specid_t inSpec = workspace->getSpectrum(i)->getSpectrumNo();
      SpectraDistanceMap neighbours = workspace->getNeighboursExact(inSpec,
                                                                    numNeighbours,
                                                                    true);

      g_log.debug() << "Current ID: " << inSpec << std::endl;
      // Convert from spectrum numbers to workspace indices
      double thetaWidth = -DBL_MAX;
      double phiWidth = -DBL_MAX;

      // Find theta and phi widths
      double theta = workspace->detectorTwoTheta(detector);
      double phi = detector->getPhi();

      specid_t deltaPlus1 = inSpec + 1;
      specid_t deltaMinus1 = inSpec - 1;
      specid_t deltaPlusT = inSpec + this->m_detNeighbourOffset;
      specid_t deltaMinusT = inSpec - this->m_detNeighbourOffset;

      for (SpectraDistanceMap::iterator it = neighbours.begin();
           it != neighbours.end(); ++it)
      {
        specid_t spec = it->first;
        g_log.debug() << "Neighbor ID: " << spec << std::endl;
        if (spec == deltaPlus1 || spec == deltaMinus1 ||
            spec == deltaPlusT || spec == deltaMinusT)
        {
          DetConstPtr detector_n = workspace->getDetector(spec - 1);
          double theta_n = workspace->detectorTwoTheta(detector_n);
          double phi_n = detector_n->getPhi();

          double dTheta = std::fabs(theta - theta_n);
          double dPhi = std::fabs(phi - phi_n);
          if (dTheta > thetaWidth)
          {
            thetaWidth = dTheta;
            //g_log.debug() << "Current ThetaWidth: " << thetaWidth << std::endl;
          }
          if (dPhi > phiWidth)
          {
            phiWidth = dPhi;
            //g_log.debug() << "Current PhiWidth: " << phiWidth << std::endl;
          }
        }
      }
      this->m_theta[i] = theta;
      this->m_phi[i] = phi;
      this->m_thetaWidths[i] = thetaWidth;
      this->m_phiWidths[i] = phiWidth;
    }
  }

  /** Creates the output workspace, setting the axes according to the input binning parameters
   *  @param[in]  inputWorkspace The input workspace
   *  @param[in]  binParams The bin parameters from the user
   *  @param[out] newAxis        The 'vertical' axis defined by the given parameters
   *  @return A pointer to the newly-created workspace
   */
  RebinnedOutput_sptr SofQW3::setUpOutputWorkspace(API::MatrixWorkspace_const_sptr inputWorkspace,
      const std::vector<double> & binParams, std::vector<double>& newAxis)
  {
    // Create vector to hold the new X axis values
    MantidVecPtr xAxis;
    xAxis.access() = inputWorkspace->readX(0);
    const int xLength = static_cast<int>(xAxis->size());
    // Create a vector to temporarily hold the vertical ('y') axis and populate that
    const int yLength = static_cast<int>(VectorHelper::createAxisFromRebinParams(binParams,newAxis));

    // Create the output workspace
    MatrixWorkspace_sptr temp = WorkspaceFactory::Instance().create("RebinnedOutput", yLength - 1, xLength, xLength - 1);
    RebinnedOutput_sptr outputWorkspace = boost::static_pointer_cast<RebinnedOutput>(temp);
    WorkspaceFactory::Instance().initializeFromParent(inputWorkspace,
                                                      outputWorkspace, true);

    // Create a numeric axis to replace the default vertical one
    Axis* const verticalAxis = new NumericAxis(yLength);
    outputWorkspace->replaceAxis(1, verticalAxis);

    // Now set the axis values
    for (int i = 0; i < yLength - 1; ++i)
    {
      outputWorkspace->setX(i, xAxis);
      verticalAxis->setValue(i, newAxis[i]);
    }
    // One more to set on the 'y' axis
    verticalAxis->setValue(yLength - 1, newAxis[yLength - 1]);

    // Set the axis units
    verticalAxis->unit() = UnitFactory::Instance().create("MomentumTransfer");
    verticalAxis->title() = "|Q|";

    // Set the X axis title (for conversion to MD)
    outputWorkspace->getAxis(0)->title() = "Energy transfer";

    return outputWorkspace;
  }

} // namespace Mantid
} // namespace Algorithms


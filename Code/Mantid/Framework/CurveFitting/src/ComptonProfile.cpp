//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "MantidCurveFitting/ComptonProfile.h"
#include "MantidCurveFitting/ConvertToYSpace.h"
#include "MantidAPI/FunctionFactory.h"
#include <gsl/gsl_poly.h>

namespace Mantid
{
namespace CurveFitting
{

  namespace
  {
    ///@cond
    const char * WSINDEX_NAME = "WorkspaceIndex";
    const char * MASS_NAME = "Mass";

    const double STDDEV_TO_HWHM = std::sqrt(std::log(4.0));
    ///@endcond
  }

  /**
   */
  ComptonProfile::ComptonProfile() : API::ParamFunction(), API::IFunction1D(),
      m_log(Kernel::Logger::get("ComptonProfile")),
      m_wsIndex(0), m_mass(0.0), m_voigt(),
      m_yspace(), m_modQ(), m_e0(),m_resolutionSigma(0.0), m_lorentzFWHM(0.0)
  {}

  //-------------------------------------- Function evaluation -----------------------------------------


  /**
   * Calculates the value of the function for each x value and stores in the given output array
   * @param out An array of size nData to store the results
   * @param xValues The input X data array of size nData. It is assumed to be times in microseconds
   * @param nData The length of the out & xValues arrays
   */
  void ComptonProfile::function1D(double* out, const double* xValues, const size_t nData) const
  {
    UNUSED_ARG(xValues); // Y-space values have already been pre-cached

    this->massProfile(out, nData);

    m_log.setEnabled(false);
  }

  /*
   * Creates the internal caches
   */
  void ComptonProfile::setUpForFit()
  {
    // Voigt
    using namespace Mantid::API;
    m_voigt = boost::dynamic_pointer_cast<IPeakFunction>(FunctionFactory::Instance().createFunction("Voigt"));
  }

  /**
   * Also caches parameters from the instrument
   * Throws if it is not a MatrixWorkspace
   * @param ws The workspace set as input
   */
  void ComptonProfile::setWorkspace(boost::shared_ptr<const API::Workspace> ws)
  {
    auto workspace = boost::dynamic_pointer_cast<const API::MatrixWorkspace>(ws);
    if(!workspace)
    {
      throw std::invalid_argument("ComptonProfile expected an object of type MatrixWorkspace, type=" + ws->id());
    }
    auto inst = workspace->getInstrument();
    auto sample = inst->getSample();
    auto source = inst->getSource();
    if(!sample || !source)
    {
      throw std::invalid_argument("ComptonProfile - Workspace has no source/sample.");
    }
    Geometry::IDetector_const_sptr det;
    try
    {
     det = workspace->getDetector(m_wsIndex);
    }
    catch (Kernel::Exception::NotFoundError &)
    {
      throw std::invalid_argument("ComptonProfile - Workspace has no detector attached to histogram at index " + boost::lexical_cast<std::string>(m_wsIndex));
    }

    DetectorParams detpar = ConvertToYSpace::getDetectorParameters(workspace, m_wsIndex);
    const auto & pmap = workspace->constInstrumentParameters();

    ResolutionParams respar;
    respar.dl1 = ConvertToYSpace::getComponentParameter(det, pmap, "sigma_l1");
    respar.dl2 = ConvertToYSpace::getComponentParameter(det, pmap, "sigma_l2");
    respar.dthe = ConvertToYSpace::getComponentParameter(det, pmap, "sigma_theta"); //radians
    respar.dEnLorentz = ConvertToYSpace::getComponentParameter(det, pmap, "hwhm_lorentz");
    respar.dEnGauss = ConvertToYSpace::getComponentParameter(det, pmap, "sigma_gauss");

    this->cacheYSpaceValues(workspace->readX(m_wsIndex), workspace->isHistogramData(), detpar, respar);
  }

  /**
   * @param tseconds A vector containing the time-of-flight values in seconds
   * @param isHistogram True if histogram tof values have been passed in
   * @param detpar Structure containing detector parameters
   * @param respar Structure containing resolution parameters
   */
  void ComptonProfile::cacheYSpaceValues(const std::vector<double> & tseconds, const bool isHistogram,
                                         const DetectorParams & detpar, const ResolutionParams & respar)
  {
    // geometry
    double theta = detpar.theta; //cache for frequent access
    double hwhmLorentzE = respar.dEnLorentz;
    double hwhmGaussE = STDDEV_TO_HWHM*respar.dEnGauss;

    // ------ Fixed coefficients related to resolution & Y-space transforms ------------------
    const double mn = PhysicalConstants::NeutronMassAMU;
    const double mevToK = PhysicalConstants::E_mev_toNeutronWavenumberSq;
    const double massToMeV = 0.5*PhysicalConstants::NeutronMass/PhysicalConstants::meV; // Includes factor of 1/2

    const double v1 = std::sqrt(detpar.efixed/massToMeV);
    const double k1 = std::sqrt(detpar.efixed/mevToK);
    const double l2l1 = detpar.l2/detpar.l1;

    // Resolution dependence

    // Find K0/K1 at y=0 by taking the largest root of (M-1)s^2 + 2cos(theta)s - (M+1) = 0
    // Quadratic if M != 1 but simple linear if it does
    double k0k1(0.0);
    if((m_mass-1.0) > DBL_EPSILON)
    {
      double x0(0.0),x1(0.0);
      gsl_poly_solve_quadratic(m_mass-1.0, 2.0*std::cos(theta), -(m_mass+1.0), &x0, &x1);
      k0k1 = std::max(x0,x1); // K0/K1 at y=0
    }
    else
    {
      // solution is simply s = 1/cos(theta)
      k0k1 = 1.0/std::cos(theta);
    }
    double qy0(0.0), wgauss(0.0);

    if(m_mass > 1.0)
    {
      qy0 = std::sqrt(k1*k1*m_mass*(k0k1*k0k1 - 1));
      double k0k1p3 = std::pow(k0k1,3);
      double r1 = -(1.0 + l2l1*k0k1p3);
      double r2 = 1.0 - l2l1*k0k1p3 + l2l1*std::pow(k0k1,2)*std::cos(theta) - k0k1*std::cos(theta);

      double factor = (0.2413/qy0)*((m_mass/mn)*r1 - r2);
      m_lorentzFWHM = std::abs(factor*hwhmLorentzE*2);
      wgauss = std::abs(factor*hwhmGaussE*2);
    }
    else
    {
      qy0 = k1*std::tan(theta);
      double factor = (0.2413*2.0/k1)*std::abs((std::cos(theta) + l2l1)/std::sin(theta));
      m_lorentzFWHM = hwhmLorentzE*factor;
      wgauss = hwhmGaussE*factor;
    }

    double k0y0 = k1*k0k1;                     // k0_y0 =  k0 value at y=0
    double wtheta = 2.0*STDDEV_TO_HWHM*std::abs(k0y0*k1*std::sin(theta)/qy0)*respar.dthe;
    double common = (m_mass/mn) - 1 + k1*std::cos(theta)/k0y0;
    double wl1 = 2.0*STDDEV_TO_HWHM*std::abs((std::pow(k0y0,2)/(qy0*detpar.l1))*common)*respar.dl1;
    double wl2 = 2.0*STDDEV_TO_HWHM*std::abs((std::pow(k0y0,3)/(k1*qy0*detpar.l1))*common)*respar.dl2;

    m_resolutionSigma = std::sqrt(std::pow(wgauss,2) + std::pow(wtheta,2) + std::pow(wl1,2) + std::pow(wl2,2));

    m_log.notice() << "--------------------- Mass=" << m_mass << " -----------------------" << std::endl;
    m_log.notice() << "w_l1 (FWHM)=" << wl2 << std::endl;
    m_log.notice() << "w_l0 (FWHM)=" << wl1 << std::endl;
    m_log.notice() << "w_theta (FWHM)=" << wtheta << std::endl;
    m_log.notice() << "w_foil_lorentz (FWHM)=" << m_lorentzFWHM << std::endl;
    m_log.notice() << "w_foil_gauss (FWHM)=" << wgauss << std::endl;

    // Calculate energy dependent factors and transform q to Y-space
    const size_t nData = (isHistogram) ? tseconds.size() - 1 : tseconds.size();

    m_e0.resize(nData);
    m_modQ.resize(nData);
    m_yspace.resize(nData);
    for(size_t i = 0; i < nData; ++i)
    {
      const double tsec = (isHistogram) ? 0.5*(tseconds[i] + tseconds[i+1]) : tseconds[i];
      ConvertToYSpace::calculateY(m_yspace[i], m_modQ[i],m_e0[i],
                                  m_mass,tsec,k1,v1,detpar);
    }
  }

  /**
   */
  void ComptonProfile::declareAttributes()
  {
    declareAttribute(WSINDEX_NAME, IFunction::Attribute(static_cast<int>(m_wsIndex)));
    declareAttribute(MASS_NAME, IFunction::Attribute(m_mass));
  }

  /**
   * @param name The name of the attribute
   * @param value The attribute's value
   */
  void ComptonProfile::setAttribute(const std::string& name,const Attribute& value)
  {
    IFunction::setAttribute(name,value); // Make sure the base-class stores it
    if(name == WSINDEX_NAME)  m_wsIndex = static_cast<size_t>(value.asInt());
    else if(name == MASS_NAME) m_mass = value.asDouble();
  }

  /**
   * Transforms the input y coordinates using a difference if Voigt functions across the whole range
   * @param voigtDiff [Out] Output values (vector is expected to be of the correct size)
   * @param yspace Input y coordinates
   * @param lorentzPos LorentzPos parameter
   * @param lorentzAmp LorentzAmp parameter
   * @param lorentzWidth LorentzFWHM parameter
   * @param gaussWidth GaussianFWHM parameter
   */
  void ComptonProfile::voigtApproxDiff(std::vector<double> & voigtDiff, const std::vector<double> & yspace, const double lorentzPos, const double lorentzAmp,
                                       const double lorentzWidth, const double gaussWidth) const
  {
    double miny(DBL_MAX), maxy(-DBL_MAX);
    auto iend = yspace.end();
    for(auto itr = yspace.begin(); itr != iend; ++itr)
    {
      const double absy = std::abs(*itr);
      if(absy < miny) miny = absy;
      else if(absy > maxy) maxy = absy;
    }
    const double epsilon = (maxy - miny)/1000.0;

    // Compute: V = (voigt(y+2eps,...) - voigt(y-2eps,...) - 2*voigt(y+eps,...) + 2*(voigt(y-eps,...))/(2eps^3)

    std::vector<double> ypmEps(yspace.size());
    // y+2eps
    std::transform(yspace.begin(), yspace.end(), ypmEps.begin(), std::bind2nd(std::plus<double>(), 2.0*epsilon)); // Add 2 epsilon
    voigtApprox(voigtDiff, ypmEps, lorentzPos, lorentzAmp, lorentzWidth, gaussWidth);
    // y-2eps
    std::transform(yspace.begin(), yspace.end(), ypmEps.begin(), std::bind2nd(std::minus<double>(), 2.0*epsilon)); // Subtract 2 epsilon
    std::vector<double> tmpResult(yspace.size());
    voigtApprox(tmpResult, ypmEps, lorentzPos, lorentzAmp, lorentzWidth, gaussWidth);
    // Difference of first two terms - result is put back in voigtDiff
    std::transform(voigtDiff.begin(), voigtDiff.end(), tmpResult.begin(), voigtDiff.begin(), std::minus<double>());

    // y+eps
    std::transform(yspace.begin(), yspace.end(), ypmEps.begin(), std::bind2nd(std::plus<double>(), epsilon)); // Add epsilon
    voigtApprox(tmpResult, ypmEps, lorentzPos, lorentzAmp, lorentzWidth, gaussWidth);
    std::transform(tmpResult.begin(), tmpResult.end(), tmpResult.begin(), std::bind2nd(std::multiplies<double>(), 2.0)); // times 2
    // Difference with 3rd term - result is put back in voigtDiff
    std::transform(voigtDiff.begin(), voigtDiff.end(), tmpResult.begin(), voigtDiff.begin(), std::minus<double>());

    //y-eps
    std::transform(yspace.begin(), yspace.end(), ypmEps.begin(), std::bind2nd(std::minus<double>(), epsilon)); // Subtract epsilon
    voigtApprox(tmpResult, ypmEps, lorentzPos, lorentzAmp, lorentzWidth, gaussWidth);
    std::transform(tmpResult.begin(), tmpResult.end(), tmpResult.begin(), std::bind2nd(std::multiplies<double>(), 2.0)); // times 2
    // Sum final term
    std::transform(voigtDiff.begin(), voigtDiff.end(), tmpResult.begin(), voigtDiff.begin(), std::plus<double>());

    // Finally multiply by 2*eps^3
    std::transform(voigtDiff.begin(), voigtDiff.end(), voigtDiff.begin(), std::bind2nd(std::divides<double>(), 2.0*std::pow(epsilon,3))); // divided by (2eps^3)
  }

  /**
   * Transforms the input y coordinates using the Voigt function approximation. The area is normalized to lorentzAmp
   * @param voigt [Out] Output values (vector is expected to be of the correct size
   * @param yspace Input y coordinates
   * @param lorentzPos LorentzPos parameter
   * @param lorentzAmp LorentzAmp parameter
   * @param lorentzWidth LorentzFWHM parameter
   * @param gaussWidth GaussianFWHM parameter
   */
  void ComptonProfile::voigtApprox(std::vector<double> & voigt, const std::vector<double> & yspace, const double lorentzPos,
                                 const double lorentzAmp, const double lorentzWidth, const double gaussWidth) const
  {
    m_voigt->setParameter(0,lorentzAmp);
    m_voigt->setParameter(1,lorentzPos);
    m_voigt->setParameter(2,lorentzWidth);
    m_voigt->setParameter(3,gaussWidth);
    assert(voigt.size() == yspace.size());
    m_voigt->functionLocal(voigt.data(), yspace.data(), yspace.size());

    // Normalize so that integral of V=lorentzAmp
    const double norm = 1.0/(0.5*M_PI*lorentzWidth);
    std::transform(voigt.begin(), voigt.end(), voigt.begin(), std::bind2nd(std::multiplies<double>(), norm));
  }

} // namespace CurveFitting
} // namespace Mantid

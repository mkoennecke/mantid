#include "MantidCurveFitting/Bk2BkExpConvPV.h"
#include "MantidKernel/System.h"
#include "MantidAPI/FunctionFactory.h"
#include <gsl/gsl_sf_erf.h>

#define PI 3.14159265358979323846264338327950288419716939937510582

using namespace Mantid::Kernel;
using namespace Mantid::API;

namespace Mantid
{
namespace CurveFitting
{

  DECLARE_FUNCTION(Bk2BkExpConvPV)

  // Get a reference to the logger
  Mantid::Kernel::Logger& Bk2BkExpConvPV::g_log = Kernel::Logger::get("Bk2BkExpConvPV");

  // ----------------------------
  /*
   * Constructor and Desctructor
   */
  Bk2BkExpConvPV::Bk2BkExpConvPV() : mFWHM(0.0)
  {

  }

  Bk2BkExpConvPV::~Bk2BkExpConvPV()
  {

  }

  /*
   * Initialize:  declare paraemters
   */
  void Bk2BkExpConvPV::init()
  {
    declareParameter("I", 1.0);
    declareParameter("TOF_h", -0.0);
    declareParameter("height", 1.0);
    declareParameter("Alpha",1.6);
    declareParameter("Beta",1.6);
    declareParameter("Sigma2", 1.0);
    declareParameter("Gamma", 0.0);

    return;
  }

  double Bk2BkExpConvPV::centre()const
  {
    double tofh = getParameter("TOF_h");


    return tofh;
  }


  void Bk2BkExpConvPV::setHeight(const double h)
  {
    setParameter("height", h);

    return;
  };

  double Bk2BkExpConvPV::height() const
  {
    double height = this->getParameter("height");
    return height;
  };

  double Bk2BkExpConvPV::fwhm() const
  {
    if (fabs(mFWHM) < 1.0E-8)
    {
      double sigma2 = this->getParameter("Sigma2");
      double gamma = this->getParameter("Gamma");
      double H, eta;
      calHandEta(sigma2, gamma, H, eta);
    }

    return mFWHM;
  };

  void Bk2BkExpConvPV::setFwhm(const double w)
  {
    UNUSED_ARG(w);
    throw std::invalid_argument("Unable to set FWHM");
  };

  void Bk2BkExpConvPV::setCentre(const double c)
  {
    setParameter("TOF_h",c);
  };

  /*
   * Implement the peak calculating formula
   */
  void Bk2BkExpConvPV::functionLocal(double* out, const double* xValues, const size_t nData) const
  {
    // 1. Prepare constants
    const double alpha = this->getParameter("Alpha");
    const double beta = this->getParameter("Beta");
    const double sigma2 = this->getParameter("Sigma2");
    const double gamma = this->getParameter("Gamma");
    const double height = this->getParameter("height");
    const double tof_h = this->getParameter("TOF_h");

    double invert_sqrt2sigma = 1.0/sqrt(2.0*sigma2);
    double N = alpha*beta*0.5/(alpha+beta);

    double H, eta;
    calHandEta(sigma2, gamma, H, eta);

    // g_log.debug() << "DB1140: TOF_h = " << tof_h << " h = " << height << ", I = " << this->getParameter("I") << " alpha = "
    // << alpha << " beta = " << beta << " H = " << H << " eta = " << eta << std::endl;

    // 2. Do calculation
    std::cout << "DB1143:  nData = " << nData << "  From " << xValues[0] << " To " << xValues[nData-1] << std::endl;
    for (size_t id = 0; id < nData; ++id)
    {
      double dT = xValues[id]-tof_h;
      double omega = calOmega(dT, eta, N, alpha, beta, H, sigma2, invert_sqrt2sigma);
      out[id] = height*omega;
      // std::cout << "DB1143  " << xValues[id] << "   " << out[id] << "   " << omega << std::endl;
    }

    return;
  }

  void Bk2BkExpConvPV::functionDerivLocal(API::Jacobian* , const double* , const size_t )
  {
    throw Mantid::Kernel::Exception::NotImplementedError("functionDerivLocal is not implemented for IkedaCarpenterPV.");
  }

  void Bk2BkExpConvPV::functionDeriv(const API::FunctionDomain& domain, API::Jacobian& jacobian)
  {
    calNumericalDeriv(domain, jacobian);
  }

  /*
   * Calculate Omega(x) = ... ...
   */
  double Bk2BkExpConvPV::calOmega(double x, double eta, double N, double alpha, double beta, double H,
      double sigma2, double invert_sqrt2sigma) const
  {
    // 1. Prepare
    std::complex<double> p(alpha*x, alpha*H*0.5);
    std::complex<double> q(-beta*x, beta*H*0.5);

    double u = 0.5*alpha*(alpha*sigma2+2*x);
    double y = (alpha*sigma2 + x)*invert_sqrt2sigma;

    double v = 0.5*beta*(beta*sigma2 - 2*x);
    double z = (beta*sigma2 - x)*invert_sqrt2sigma;

    // 2. Calculate
    double omega1 = (1-eta)*N*(exp(u)*gsl_sf_erfc(y) + std::exp(v)*gsl_sf_erfc(z));
    double omega2;
    if (eta < 1.0E-8)
    {
      omega2 = 0.0;
    }
    else
    {
      omega2 = 2*N*eta/PI*(imag(exp(p)*E1(p)) + imag(exp(q)*E1(q)));
    }
    double omega = omega1+omega2;

    return omega;
  }

  /*
   * Implementation of complex integral E_1
   */
  std::complex<double> Bk2BkExpConvPV::E1(std::complex<double> z) const
  {
    return z;
  }

  void Bk2BkExpConvPV::geneatePeak(double* out, const double* xValues, const size_t nData)
  {
    this->functionLocal(out, xValues, nData);

    return;
  }

  void Bk2BkExpConvPV::calHandEta(double sigma2, double gamma, double& H, double& eta) const
  {
    // 1. Calculate H
    double H_G = sqrt(8.0 * sigma2 * log(2.0));
    double H_L = gamma;

    double temp1 = std::pow(H_L, 5) + 0.07842*H_G*std::pow(H_L, 4) + 4.47163*std::pow(H_G, 2)*std::pow(H_L, 3) +
        2.42843*std::pow(H_G, 3)*std::pow(H_L, 2) + 2.69269*std::pow(H_G, 4)*H_L + std::pow(H_G, 5);

    H = std::pow(temp1, 0.2);

    mFWHM = H;

    // 2. Calculate eta
    double gam_pv = H_L/H;
    eta = 1.36603 * gam_pv - 0.47719 * std::pow(gam_pv, 2) + 0.11116 * std::pow(gam_pv, 3);

    if (eta > 1 || eta < 0)
    {
      g_log.error() << "Calculated eta = " << eta << " is out of range [0, 1]." << std::endl;
    }

    return;
  }

} // namespace Mantid
} // namespace CurveFitting

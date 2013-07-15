/*WIKI*

This function interpolates between a set of data points.

First and second derivatives can be calculated by using the derivative function.

CubicSpline function has three attributes (non-fitting parameters). First is 'n' which has
integer type and sets the number of interpolation points.
The parameter names have the form 'yi' where 'A' is letter 'A' and 'i' is the
parameter's index starting from 0.

 *WIKI*/

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidCurveFitting/CubicSpline.h"
#include "MantidAPI/FunctionFactory.h"

#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace Mantid
{
  namespace CurveFitting
  {

    using namespace Kernel;
    using namespace API;

    DECLARE_FUNCTION(CubicSpline)

    const int CubicSpline::M_MIN_POINTS = 3;

    CubicSpline::CubicSpline() :
        m_acc(gsl_interp_accel_alloc()),
        m_spline(gsl_spline_alloc(gsl_interp_cspline, M_MIN_POINTS)),
        m_interp(gsl_interp_alloc(gsl_interp_cspline, M_MIN_POINTS)),
        m_recalculateSpline(true)
    {
      //setup class with a default set of attributes
      declareAttribute("n", Attribute(M_MIN_POINTS));

      declareAttribute("x0", Attribute(0.0));
      declareAttribute("x1", Attribute(1.0));
      declareAttribute("x2", Attribute(2.0));

      declareParameter("y0", 0);
      declareParameter("y1", 0);
      declareParameter("y2", 0);

    };

    void CubicSpline::function1D(double* out, const double* xValues, const size_t nData) const
    {
      //check if spline needs recalculating
      if (m_recalculateSpline)
      {
        int n = getAttribute("n").asInt();


        boost::scoped_array<double> x(new double[n]);
        boost::scoped_array<double> y(new double[n]);

        //setup the reference points and calculate
        setupInput(x, y, n);
        calculateSpline(out, xValues, nData);

        m_recalculateSpline = false;
      }
    }

    void CubicSpline::setupInput(boost::scoped_array<double>& x,
        boost::scoped_array<double>& y, int n) const
    {
      //Populate data points from the input attributes and parameters
      bool xSortFlag = false;

      for (int i = 0; i < n; ++i)
      {
        std::string num = boost::lexical_cast<std::string>(i);

        std::string xName = "x" + num;
        std::string yName = "y" + num;

        x[i] = getAttribute(xName).asDouble();

        //if x[i] is out of order with its neighbours
        if(i>1 && i < n &&
            (x[i-1] < x[i-2] || x[i-1] > x[i]))
        {
          xSortFlag = true;
        }

        y[i] = getParameter(yName);
      }

      //sort the data points if necessary
      if(xSortFlag)
      {
        g_log.warning() << "Spline x parameters are not in ascending order. Values will be sorted." << std::endl;
        std::sort(x.get(), x.get()+n);
      }

      //pass values to GSL objects
      initGSLObjects(x,y,n);
    }

    void CubicSpline::derivative1D(double* out, const double* xValues, size_t nData, const size_t order) const
    {
        int n = getAttribute("n").asInt();

        boost::scoped_array<double> x(new double[n]);
        boost::scoped_array<double> y(new double[n]);

        //setup the reference points and calculate
        setupInput(x,y,n);
        calculateDerivative(x,y,out,xValues,nData, order);
    }

    void CubicSpline::calculateSpline(double* out, const double* xValues, const size_t nData) const
    {
      //calculate spline for given input set
      double y = 0;
      for (size_t i = 0; i < nData; ++i)
      {
        y = gsl_spline_eval(m_spline, xValues[i], m_acc);
        int errorCode = gsl_spline_eval_e(m_spline, xValues[i], m_acc, &y);

        //check if GSL function returned an error
        checkGSLError(errorCode, GSL_EDOM);

        out[i] = y;
      }
    }

    void CubicSpline::calculateDerivative(const boost::scoped_array<double>& x, const boost::scoped_array<double>& y,
        double* out, const double* xValues, const size_t nData, const size_t order) const
    {
      double x_deriv = 0;
      int errorCode = 0;

      for(size_t i = 0; i < nData; ++i)
      {
        //choose the order of the derivative
        if(order == 1)
        {
          x_deriv = gsl_interp_eval_deriv(m_interp,x.get(),y.get(),xValues[i],m_acc);
          errorCode = gsl_interp_eval_deriv_e (m_interp,x.get(),y.get(),xValues[i],m_acc,&x_deriv);
        }
        else if (order == 2)
        {
          x_deriv = gsl_interp_eval_deriv2(m_interp,x.get(),y.get(),xValues[i],m_acc);
          errorCode = gsl_interp_eval_deriv2_e (m_interp,x.get(),y.get(),xValues[i],m_acc,&x_deriv);
        }
        else
        {
          //throw error if the order is not the 1st or 2nd derivative
          throw std::invalid_argument("CubicSpline: order of derivative must be either 1 or 2");
        }

        //check GSL functions didn't return an error
        checkGSLError(errorCode, GSL_EDOM);

        //record the value
        out[i] = x_deriv;
      }
    }

    void CubicSpline::setParameter(size_t i, const double& value, bool explicitlySet)
    {
      //Call parent setParameter implementation
      ParamFunction::setParameter(i, value, explicitlySet);

      //recalculate if necessary
      m_recalculateSpline = true;
    }

    void CubicSpline::setAttribute(const std::string& attName, const API::IFunction::Attribute& att)
    {
      if (attName == "n")
      {
        //get the new and old number of data points
        int n = att.asInt();
        int oldN = getAttribute("n").asInt();

        //check that the number of data points is in a valid range
        if (n > oldN)
        {
          //get the name of the last x data point
          std::string oldXName = "x" + boost::lexical_cast<std::string>(oldN - 1);
          double oldX = getAttribute(oldXName).asDouble();

          //reallocate gsl object to new size
          reallocGSLObjects(n);

          //create blank a number of new blank parameters and attributes
          for (int i = oldN; i < n; ++i)
          {
            std::string num = boost::lexical_cast<std::string>(i);

            std::string newXName = "x" + num;
            std::string newYName = "y" + num;

            declareAttribute(newXName, Attribute(oldX + static_cast<double>(i - oldN + 1)));
            declareParameter(newYName, 0);
          }

          //flag that the spline+derivatives will now need to be recalculated
          m_recalculateSpline = true;
        }
        else if (n < oldN)
        {
          throw std::invalid_argument("Cubic Spline: Can't decrease the number of attributes");
        }
      }

      storeAttributeValue(attName, att);
    }

    void CubicSpline::setXAttribute(const size_t index, double x)
    {
      size_t n = static_cast<size_t>(getAttribute("n").asInt());

      //check that setting the x attribute is within our range
      if (index < n)
      {
        std::string xName = "x" + boost::lexical_cast<std::string>(index);
        setAttributeValue(xName, x);

        //attributes updated, flag for recalculation
        m_recalculateSpline = true;
      }
      else
      {
        throw std::range_error("Cubic Spline: x index out of range.");
      }
    }

    void CubicSpline::checkGSLError(const int status, const int errorType) const
    {
      //check GSL functions didn't return an error
      if(status == errorType)
      {
        m_recalculateSpline = true;

        std::string message("CubicSpline: ");
        message.append(gsl_strerror(errorType));
        throw std::runtime_error(message);
      }
    }

    void CubicSpline::initGSLObjects(boost::scoped_array<double>& x, boost::scoped_array<double>& y, int n) const
    {
      //init the gsl structures if required
      if(m_recalculateSpline)
      {
        int status = gsl_spline_init(m_spline, x.get(), y.get(), n);
        checkGSLError(status, GSL_EINVAL);
      }

       int status = gsl_interp_init(m_interp, x.get(), y.get(), n);
       checkGSLError(status, GSL_EINVAL);
    }

    void CubicSpline::reallocGSLObjects(const int n)
    {
      gsl_interp_free(m_interp);
      gsl_spline_free(m_spline);

      m_spline = gsl_spline_alloc(gsl_interp_cspline, n);
      m_interp = gsl_interp_alloc(gsl_interp_cspline, n);

      gsl_interp_accel_reset (m_acc);
    }

    CubicSpline::~CubicSpline()
    {
      gsl_spline_free(m_spline);
      gsl_interp_free(m_interp);
      gsl_interp_accel_free(m_acc);
    }
  } // namespace CurveFitting
} // namespace Mantid

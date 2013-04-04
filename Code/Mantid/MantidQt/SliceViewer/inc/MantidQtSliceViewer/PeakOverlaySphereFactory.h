#ifndef MANTID_SLICEVIEWER_PEAKOVERLAYSPHERE_FACTORY_H_
#define MANTID_SLICEVIEWER_PEAKOVERLAYSPHERE_FACTORY_H_

#include "MantidQtSliceViewer/PeakOverlayViewFactoryBase.h"
#include "MantidAPI/IPeaksWorkspace.h"
#include <boost/shared_ptr.hpp>

namespace MantidQt
{
  namespace SliceViewer
  {

    /** Concrete view factory. For creating instances of PeakOverlaySphere widget.

    @date 2012-08-24

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

    File change history is stored at: <https://github.com/mantidproject/mantid>
    Code Documentation is available at: <http://doxygen.mantidproject.org>
    */
    class DLLExport PeakOverlaySphereFactory : public PeakOverlayViewFactoryBase
    {
    private:
      double m_peakRadius;
      double m_backgroundInnerRadius;
      double m_backgroundOuterRadius;
      Mantid::API::IPeaksWorkspace_sptr m_peaksWS;
      int m_FOM;
    public:
      PeakOverlaySphereFactory(Mantid::API::IPeaksWorkspace_sptr peaksWS, QwtPlot * plot, QWidget * parent, const size_t colourNumber=0);
      virtual ~PeakOverlaySphereFactory();
      virtual boost::shared_ptr<PeakOverlayView> createView(const int peakIndex, PeakTransform_const_sptr transform) const;
      virtual int FOM() const;
    };
  }
}

#endif /*MANTID_SLICEVIEWER_PEAKOVERLAYSPHERE_FACTORY_H_*/

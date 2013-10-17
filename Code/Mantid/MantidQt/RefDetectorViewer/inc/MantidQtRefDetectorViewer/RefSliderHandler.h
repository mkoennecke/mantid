#ifndef REF_SLIDER_HANDLER_H
#define REF_SLIDER_HANDLER_H

#include "MantidQtSpectrumViewer/ISliderHandler.h"
#include <QRect>

#include "ui_RefImageView.h"
#include "MantidQtSpectrumViewer/SpectrumDataSource.h"
#include "MantidQtRefDetectorViewer/DllOption.h"

/**
    @class SliderHandler 
  
    This manages the horizontal and vertical scroll bars for the
    SpectrumView data viewer. 
 
    @author Dennis Mikkelson 
    @date   2012-04-03 
     
    Copyright © 2012 ORNL, STFC Rutherford Appleton Laboratories
  
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
    
    Code Documentation is available at 
                 <http://doxygen.mantidproject.org>
 */

namespace MantidQt
{
namespace RefDetectorViewer
{


class EXPORT_OPT_MANTIDQT_REFDETECTORVIEWER RefSliderHandler : public SpectrumView::ISliderHandler
{
  public:

    /// Construct object to manage image scrollbars from the specified UI
  RefSliderHandler( Ui_RefImageViewer* iv_ui );

    /// Configure the image scrollbars for the specified data and drawing area
    void ConfigureSliders( QRect            draw_area, 
                           SpectrumView::SpectrumDataSource* data_source );

    /// Configure the horizontal scrollbar to cover the specified range
    void ConfigureHSlider( int         n_data_steps, 
                           int         n_pixels );

    /// Return true if the image horizontal scrollbar is enabled.
    bool HSliderOn();

    /// Return true if the image vertical scrollbar is enabled.
    bool VSliderOn();

    /// Get the range of columns to display in the image.
    void GetHSliderInterval( int &x_min, int &x_max );

    /// Get the range of rows to display in the image.
    void GetVSliderInterval( int &y_min, int &y_max );

  private:
    /// Configure the specified scrollbar to cover the specified range
    void ConfigureSlider( QScrollBar* scroll_bar, 
                          int         n_data_steps,
                          int         n_pixels,
                          int         val );

    Ui_RefImageViewer*   iv_ui;
};

} // namespace RefDetectorViewer
} // namespace MantidQt 

#endif // REF_SLIDER_HANDLER_H

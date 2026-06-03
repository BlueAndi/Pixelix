/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
 * Copyright (c) 2026        Julie Hill <queenkjuul@pm.me>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*******************************************************************************
    DESCRIPTION
*******************************************************************************/
/**
 * @file   IChicagoBusTrackerView.h
 * @brief  ChicagoBusTracker view interface
 * @author Julie Hill <queenkjuul@pm.me>
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef ICHICAGO_BUS_TRACKER_VIEW_H
#define ICHICAGO_BUS_TRACKER_VIEW_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <YAGfx.h>
#include <Fonts.h>
#include <WString.h>
#include <limits>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Interface for a view with weather informations.
 */
class IChicagoBusTrackerView
{
public:

    /**
     * Destroy the interface.
     */
    virtual ~IChicagoBusTrackerView()
    {
    }

    /**
     * Initialize view, which will prepare the widgets and the default values.
     *
     * @param[in] width     Display width in pixel.
     * @param[in] height    Display height in pixel.
     */
    virtual void init(uint16_t width, uint16_t height)      = 0;

    /**
     * Get font type.
     *
     * @return The font type the view uses.
     */
    virtual Fonts::FontType getFontType() const             = 0;

    /**
     * Set font type.
     *
     * @param[in] fontType  The font type which the view shall use.
     */
    virtual void setFontType(Fonts::FontType fontType)      = 0;

    /**
     * Update the underlying canvas.
     *
     * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
     */
    virtual void update(YAGfx& gfx)                         = 0;

    /**
     * Set text for the route information part
     * 
     * @param[in] text  Text to display
     */
    virtual void setRouteInfoText(const String& text)       = 0;
    
    /**
     * Set text for the arrivals information part
     * 
     * @param[in] text  Text to display
     */
    virtual void setArrivalsInfoText(const String& text)    = 0;

protected:

    /**
     * Construct the interface.
     */
    IChicagoBusTrackerView()
    {
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* ICHICAGO_BUS_TRACKER_VIEW_H */

/** @} */

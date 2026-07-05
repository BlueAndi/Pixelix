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
 * @file   ChicagoBusTrackerView64x64.h
 * @brief  Chicago bus tracker view with route and arrivals info 64x64 LED matrix
 * @author Julie Hill <queenkjuul@pm.me>
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef CHICAGO_BUS_TRACKER_VIEW_64X64_H
#define CHICAGO_BUS_TRACKER_VIEW_64X64_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <YAGfx.h>
#include <Fonts.h>
#include <BitmapWidget.h>
#include <TextWidget.h>
#include <Util.h>

#include "../interface/IChicagoBusTrackerView.h"
#include "ViewConfig.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * ChicagoBusTracker view for 64x64 LED matrix with icon and text.
 */
class ChicagoBusTrackerView64x64 : public IChicagoBusTrackerView
{
public:

    /**
     * Construct the view.
     */
    ChicagoBusTrackerView64x64();

    /**
     * Destroy the view.
     */
    virtual ~ChicagoBusTrackerView64x64()
    {
    }

    /**
     * Initialize view, which will prepare the widgets and the default values.
     *
     * @param[in] width     Display width in pixel.
     * @param[in] height    Display height in pixel.
     */
    void init(uint16_t width, uint16_t height) override
    {
        UTIL_NOT_USED(width);
        UTIL_NOT_USED(height);

        m_routeNumberWidget.setBrush(ViewConfig::getInstance().getBrush());
        m_infoWidget1.setBrush(ViewConfig::getInstance().getBrush());
        m_infoWidget2.setBrush(ViewConfig::getInstance().getBrush());
        m_infoWidget3.setBrush(ViewConfig::getInstance().getBrush());
        m_infoWidget4.setBrush(ViewConfig::getInstance().getBrush());
    }

    /**
     * Get font type.
     *
     * @return The font type the view uses.
     */
    Fonts::FontType getFontType() const override
    {
        return m_fontType;
    }

    /**
     * Set font type.
     *
     * @param[in] fontType  The font type which the view shall use.
     */
    void setFontType(Fonts::FontType fontType) override
    {
        m_fontType = fontType;
        m_routeNumberWidget.setFont(Fonts::getFontByType(m_fontType));
        m_infoWidget1.setFont(Fonts::getFontByType(m_fontType));
        m_infoWidget2.setFont(Fonts::getFontByType(m_fontType));
        m_infoWidget3.setFont(Fonts::getFontByType(m_fontType));
        m_infoWidget4.setFont(Fonts::getFontByType(m_fontType));
    }

    /**
     * Update the underlying canvas.
     *
     * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
     */
    void update(YAGfx& gfx) override;

    /**
     * Set the route number text
     *
     * @param[in] text  Text to display
     */
    void setRouteNumberText(const String& text) override
    {
        m_routeNumberText = text;
    }

    /**
     * Set the route info text
     *
     * @param[in] text  Text to display
     */
    void setRouteInfoText(const String& text) override
    {
        m_routeInfoText = text;
    }

    /**
     * Set the first arrival info text
     *
     * @param[in] text  Text to display
     */
    void setFirstArrivalText(const String& text) override
    {
        m_firstArrivalText = text;
    }

    /**
     * Set the second arrival info text
     *
     * @param[in] text  Text to display
     */
    void setSecondArrivalText(const String& text) override
    {
        m_secondArrivalText = text;
    }

    /**
     * Set the third arrival info text
     *
     * @param[in] text  Text to display
     */
    void setThirdArrivalText(const String& text) override
    {
        m_thirdArrivalText = text;
    }

    /**
     * 64x64 uses separate routeNumber and routeInfo widgets
     *
     */
    void updateWidgets()
    {
        m_routeNumberWidget.setFormatStr(m_routeNumberText);

        /* No route text (orig+dest disabled) so put first arrival in top row */
        if ((m_routeInfoText == "null") || (m_routeInfoText == ""))
        {
            m_infoWidget1.setFormatStr(m_firstArrivalText);
            m_infoWidget2.setFormatStr(m_secondArrivalText);
            m_infoWidget3.setFormatStr(m_thirdArrivalText);
            m_infoWidget4.setFormatStr("");
        }
        /* Route info provided: arrivals use rows 2-4 */
        else
        {
            m_infoWidget1.setFormatStr(m_routeInfoText);
            m_infoWidget2.setFormatStr(m_firstArrivalText);
            m_infoWidget3.setFormatStr(m_secondArrivalText);
            m_infoWidget4.setFormatStr(m_thirdArrivalText);
        }
    }

protected:

    Fonts::FontType m_fontType;          /**< Font type which shall be used if there is no conflict with the layout. */
    TextWidget      m_routeNumberWidget; /**< Route number widget */
    TextWidget      m_infoWidget1;       /**< Top-right widget, next to route number */
    TextWidget      m_infoWidget2;       /**< Second row, full-width below route number */
    TextWidget      m_infoWidget3;       /**< Third row, full-width */
    TextWidget      m_infoWidget4;       /**< Bottom full-width widget */
    String          m_routeNumberText;   /**< Route number text to display */
    String          m_routeInfoText;     /**< Route details text (stop name, destination) */
    String          m_firstArrivalText;  /**< First upcoming arrival text (formatted) */
    String          m_secondArrivalText; /**< Second upcoming arrival text (formatted, optional) */
    String          m_thirdArrivalText;  /**< Third upcoming arrival text (formatted, optional) */

private:

    ChicagoBusTrackerView64x64(const ChicagoBusTrackerView64x64& other);
    ChicagoBusTrackerView64x64& operator=(const ChicagoBusTrackerView64x64& other);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* CHICAGO_BUS_TRACKER_VIEW_64X64_H */

/** @} */

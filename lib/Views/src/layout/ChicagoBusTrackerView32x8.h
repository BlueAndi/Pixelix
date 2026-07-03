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
 * @file   ChicagoBusTrackerView32x8.h
 * @brief  Chicago bus tracker view tith route and arrivals info 32x8 LED matrix
 * @author Julie Hill <queenkjuul@pm.me>
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef CHICAGO_BUS_TRACKER_VIEW_32X8_H
#define CHICAGO_BUS_TRACKER_VIEW_32X8_H

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
#include <Logging.h>

#include "../interface/IChicagoBusTrackerView.h"
#include "ViewConfig.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * ChicagoBusTracker view for 32x8 LED matrix with icon and text.
 */
class ChicagoBusTrackerView32x8 : public IChicagoBusTrackerView
{
public:

    /**
     * Construct the view.
     */
    ChicagoBusTrackerView32x8();

    /**
     * Destroy the view.
     */
    virtual ~ChicagoBusTrackerView32x8()
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

        m_routeWidget.setBrush(ViewConfig::getInstance().getBrush());
        m_arrivalsWidget.setBrush(ViewConfig::getInstance().getBrush());
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
        m_arrivalsWidget.setFont(Fonts::getFontByType(m_fontType));
        m_routeWidget.setFont(Fonts::getFontByType(m_fontType));
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
     * 32x8 uses one widget for all route info, one widget for arrivals
     *
     */
    void updateWidgets()
    {
        m_routeWidget.setFormatStr(m_routeNumberText + m_routeInfoText);

        m_arrivalsInfoText  = "";
        m_arrivalsInfoText += m_firstArrivalText;

        if ((m_secondArrivalText != "null") && (m_secondArrivalText != ""))
        {
            appendArrivalInfo(m_secondArrivalText);
        }

        if ((m_thirdArrivalText != "null") && (m_thirdArrivalText != ""))
        {
            appendArrivalInfo(m_thirdArrivalText);
        }

        m_arrivalsWidget.setFormatStr(m_arrivalsInfoText);

        LOG_DEBUG("Chicago Bus Route: " + m_routeWidget.getFormatStr());
        LOG_DEBUG("Chicago Bus Arrival: " + m_arrivalsWidget.getFormatStr());
    }

protected:

    /**
     * Color to mimic CTA vehicles' amber LED screens
     */
    static constexpr const char* DISPLAY_COLOR = "{#FF5500}";

    Fonts::FontType              m_fontType;          /**< Font type which shall be used if there is no conflict with the layout. */
    TextWidget                   m_routeWidget;       /**< Route information widget */
    TextWidget                   m_arrivalsWidget;    /**< Arrivals information widget */
    String                       m_routeNumberText;   /**< Route number text (formatted) */
    String                       m_routeInfoText;     /**< Route info text (stop name, destination) */
    String                       m_arrivalsInfoText;  /**< String containing the concatenated arrivals info */
    String                       m_firstArrivalText;  /**< First arrival text info. */
    String                       m_secondArrivalText; /**< Seccond arrival text info. */
    String                       m_thirdArrivalText;  /**< Third arrival text info. */

private:

    ChicagoBusTrackerView32x8(const ChicagoBusTrackerView32x8& other);
    ChicagoBusTrackerView32x8& operator=(const ChicagoBusTrackerView32x8& other);

    /**
     * Append another arrival time to m_arrivalsInfoText
     *
     * @param[in] info  Arrival time text (e.g. '6 min')
     */
    void appendArrivalInfo(const String& info)
    {
        m_arrivalsInfoText += DISPLAY_COLOR;
        m_arrivalsInfoText += " / ";
        m_arrivalsInfoText += info;
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* CHICAGO_BUS_TRACKER_VIEW_32X8_H */

/** @} */

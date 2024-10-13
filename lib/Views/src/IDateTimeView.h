/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Date and time view interface.
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup plugin
 *
 * @{
 */

#ifndef IDATE_TIME_VIEW_H
#define IDATE_TIME_VIEW_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <YAGfx.h>
#include <Fonts.h>
#include <WString.h>
#include <time.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Interface for a view with date and time.
 */
class IDateTimeView
{
public:

    /**
     * Destroy the view.
     */
    virtual ~IDateTimeView()
    {
    }

    /**
     * The view modes, which influences how the data is
     * shown on the display.
     */
    enum ViewMode
    {
        DIGITAL_ONLY = 0U,       /**< Show date and time */
        ANALOG_ONLY,             /**< Show only the date */
        DIGITAL_AND_ANALOG,      /**< Show only the time */
        VIEW_MODE_MAX            /**< Number of configurations */
    };

    /** 
     * Options for displaying seconds in analog clock.
     */
    enum SecondsDisplayMode
    {
        SECOND_DISP_OFF = 0U,  /**< No second indicator display. */
        SECOND_DISP_HAND = 1U, /**< Draw second clock hand. */
        SECOND_DISP_RING = 2U, /**< Show passed seconds on minute tick ring. */
        SECOND_DISP_BOTH = 3U, /**< Show hand and on ring. */
        SECONDS_DISP_MAX       /**< Number of configurations. */
    };

    /** 
     * Color array indexes for the analog clock drawing.
     */
    enum AnalogClockColor
    {
        ANA_CLK_COL_HAND_HOUR = 0U, /**< Hour clock hand color. */
        ANA_CLK_COL_HAND_MIN,       /**< Minutes clock hand color. */
        ANA_CLK_COL_HAND_SEC,       /**< Seconds colock hand color */
        ANA_CLK_COL_RING_MIN5_MARK, /**< Ring five minute marks color. */
        ANA_CLK_COL_RING_MIN_DOT,   /**< Ring minut dots color. */
        ANA_CLK_COL_MAX             /**< Number of colors. */
    };

    /** Analog clock appearance configuration. */
    struct AnalogClockConfig
    {
        SecondsDisplayMode m_secondsMode;              /**< Seconds visualisation mode. */
        Color              m_colors[ANA_CLK_COL_MAX];  /**< Clock colors to use.        */
    };

    /**
     * Initialize view, which will prepare the widgets and the default values.
     */
    virtual void init(uint16_t width, uint16_t height) = 0;

    /**
     * Get font type.
     * 
     * @return The font type the view uses.
     */
    virtual Fonts::FontType getFontType() const = 0;

    /**
     * Set font type.
     * 
     * @param[in] fontType  The font type which the view shall use.
     */
    virtual void setFontType(Fonts::FontType fontType) = 0;

    /**
     * Update the underlying canvas.
     * 
     * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
     */
    virtual void update(YAGfx& gfx) = 0;

    /**
     * Get text (non-formatted).
     * 
     * @return Text
     */
    virtual String getText() const = 0;

    /**
     * Get text (formatted).
     * 
     * @return Text
     */
    virtual String getFormatText() const = 0;

    /**
     * Set text (formatted).
     * 
     * @param[in] formatText    Formatted text to show.
     */
    virtual void setFormatText(const String& formatText) = 0;

    /**
     * Get the start of week configuration (Sunday = 0).
     *
     * @return uint8_t
     */
    virtual uint8_t getStartOfWeek() const = 0;

    /**
     * Set the start of week configuration (Sunday = 0).
     *
     * @param[in] startOfWeek uint8_t offset for starting of week
     *
     * @return bool success
     */
    virtual bool setStartOfWeek(uint8_t startOfWeek) = 0;

    /**
     * Get the color to show the actual day.
     * 
     * @return Color
     */
    virtual const Color& getDayOnColor() const = 0;

    /**
     * Set the color which is used for the actual day.
     * 
     * @param[in] color Color for the actual day
     */
    virtual void setDayOnColor(const Color& color) = 0;

    /**
     * Get the color to show the other days than the actual one.
     * 
     * @return Color
     */
    virtual const Color& getDayOffColor() const = 0;

    /**
     * Set the color which is used for the other days than the actual day.
     * 
     * @param[in] color Color for the other days
     */
    virtual void setDayOffColor(const Color& color) = 0;

    /**
     * Get the view mode (analog, digital or both).
     * 
     * @return ViewMode 
     */
    virtual ViewMode getViewMode() const = 0;

    /**
     * Set the view mode (analog, digital or both).
     * 
     * @return bool success of failure 
     */
    virtual bool setViewMode(ViewMode mode) = 0;

    /**
     * Get the analog clock clonfiguration.
     * 
     * @return AnalogClockConfig or nullptr if unsupported.
     */
    virtual const AnalogClockConfig* getAnalogClockConfig() const = 0;

    /**
     * Set the analog clock configuration.
     * 
     * @param[in] cfg The new configuration to apply.
     * 
     * @return success or failure
     */
    virtual bool setAnalogClockConfig(const AnalogClockConfig& cfg) = 0;

    /**
     * @brief Update current time values in view.
     * 
     * @param[in] now current time
     */
    virtual void setCurrentTime(const tm& now) = 0;

protected:

    /**
     * Construct the view.
     */
    IDateTimeView()
    {
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* IDATE_TIME_VIEW_H */

/** @} */

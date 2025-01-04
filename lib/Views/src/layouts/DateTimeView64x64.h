/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @brief  View for 64x64 LED matrix with date and time.
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef DATE_TIME_VIEW_64X64_H
#define DATE_TIME_VIEW_64X64_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <YAGfx.h>
#include "DateTimeViewGeneric.h"

#include <Fonts.h>
#include <LampWidget.h>
#include <TextWidget.h>
#include <Util.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * View for 64x64 LED matrix with date and time.
 */
class DateTimeView64x64 : public DateTimeViewGeneric
{
public:

    /**
     * Construct the view.
     */
    DateTimeView64x64() :
        DateTimeViewGeneric(),
        m_mode(ViewMode::DIGITAL_AND_ANALOG),
        m_secondsMode(SECOND_DISP_RING),
        m_analogColors{
            ColorDef::WHITE,
            ColorDef::GRAY,
            ColorDef::YELLOW,
            ColorDef::BLUE,
            ColorDef::YELLOW
        },
        m_lastUpdateSecondVal(-1)
    {
        /* Disable fade effect in case the user required to show seconds,
         * which will continuously trigger the fading effect.
         */
        m_textWidget.disableFadeEffect();

        /*
         * Move digital clock to lower part of analog clock.
         * Analog clock is also shifted in X by one as the
         * mid point is 32.
         */
        m_textWidget.move(1, 47);
    }

    /**
     * Destroy the view.
     */
    virtual ~DateTimeView64x64()
    {
    }

    /**
     * Update the underlying canvas.
     *
     * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
     */
    void update(YAGfx& gfx) override;


    /**
     * Get the view mode (analog, digital or both).
     *
     * @return View mode
     */
    ViewMode getViewMode() const override
    {
        return m_mode;
    }

    /**
     * Set the view mode (analog, digital or both).
     *
     * @param[in] mode  View mode
     *
     * @return success or failure
     */
    bool setViewMode(ViewMode mode) override
    {
        bool ret = false;

        if (ViewMode::VIEW_MODE_MAX <= mode)
        {
            LOG_WARNING("Illegal DateTime view mode (%hhu)", mode);
        }
        else
        {
            m_mode = mode;
            ret    = true;
        }

        return true;
    }

    /**
     * Get current active configuration in JSON format.
     *
     * @param[out] jsonCfg Configuration
     */
    void getConfiguration(JsonObject& jsonCfg) const override;

    /**
     * Apply configuration from JSON.
     *
     * @param[in] jsonCfg Configuration
     *
     * @return If successful set, it will return true otherwise false.
     */
    bool setConfiguration(const JsonObjectConst& jsonCfg) override;

    /**
     * Merge JSON configuration with local settings to create a complete set.
     *
     * The received configuration may not contain all single key/value pair.
     * Therefore create a complete internal configuration and overwrite it
     * with the received one.
     *
     * @param[out] jsonMerged  The complete config set with merge content from jsonSource.
     * @param[in]  jsonSource  The recevied congi set, which may not cover all keys.
     * @return     true        Keys needed merging.
     * @return     false       Nothing needed merging.
     */
    bool mergeConfiguration(JsonObject& jsonMerged, const JsonObjectConst& jsonSource) override;

protected:

    /**
     * Options for displaying seconds in analog clock.
     */
    enum SecondsDisplayMode
    {
        SECOND_DISP_OFF  = 0U, /**< No second indicator display. */
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

    /** Color key names for the analog clock configuration. */
    static const char* ANALOG_CLOCK_COLOR_KEYS[ANA_CLK_COL_MAX];

    ViewMode           m_mode;                          /**< Used View mode analog, digital or both.  */
    SecondsDisplayMode m_secondsMode;                   /**< Seconds visualisation mode. */
    Color              m_analogColors[ANA_CLK_COL_MAX]; /**< Clock colors to use.        */

    /**
     * Seconds value of last display update. Used to avoid unecessary redrawing.
     */
    int m_lastUpdateSecondVal;

private:

    DateTimeView64x64(const DateTimeView64x64& other);
    DateTimeView64x64& operator=(const DateTimeView64x64& other);

    /**
     * Draw analog clock backround (the minute tick marks)
     *
     * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
     */
    void drawAnalogClockBackground(YAGfx& gfx);

    /**
     * Draw analog clock hands for given time.
     *
     * @param[in] gfx    Graphic functionality to draw on the underlying canvas.
     * @param[in] minute Minute to point to (0..59).
     * @param[in] radius Length of hand (radius from clock mid point)
     * @param[in] col    Color of the hand.
     */
    void drawAnalogClockHand(YAGfx& gfx, int16_t minute, int16_t radius, const Color& col);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* DATE_TIME_VIEW_64X64_H */

/** @} */

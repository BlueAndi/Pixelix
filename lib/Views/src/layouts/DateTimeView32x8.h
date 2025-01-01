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
 * @brief  View for 32x8 LED matrix with date and time.
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef DATE_TIME_VIEW_32X8_H
#define DATE_TIME_VIEW_32X8_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <YAGfx.h>
#include <IDateTimeView.h>
#include <Fonts.h>
#include <LampWidget.h>
#include <TextWidget.h>
#include <Util.h>
#include <Logging.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * View for 32x8 LED matrix with date and time.
 */
class DateTimeView32x8 : public IDateTimeView
{
public:

    /**
     * Construct the view.
     */
    DateTimeView32x8() :
        IDateTimeView(),
        m_fontType(Fonts::FONT_TYPE_DEFAULT),
        m_textWidget(TEXT_WIDTH, TEXT_HEIGHT, TEXT_X, TEXT_Y),
        m_lampWidgets{{LAMP_WIDTH, LAMP_HEIGHT, LAMP_0_X , LAMP_Y},
                      {LAMP_WIDTH, LAMP_HEIGHT, LAMP_1_X , LAMP_Y},
                      {LAMP_WIDTH, LAMP_HEIGHT, LAMP_2_X , LAMP_Y},
                      {LAMP_WIDTH, LAMP_HEIGHT, LAMP_3_X , LAMP_Y},
                      {LAMP_WIDTH, LAMP_HEIGHT, LAMP_4_X , LAMP_Y},
                      {LAMP_WIDTH, LAMP_HEIGHT, LAMP_5_X , LAMP_Y},
                      {LAMP_WIDTH, LAMP_HEIGHT, LAMP_6_X , LAMP_Y}},
        m_startOfWeek(START_OF_WEEK),
        m_dayOnColor(DAY_ON_COLOR),
        m_dayOffColor(DAY_OFF_COLOR)
    {
        /* Disable fade effect in case the user required to show seconds,
         * which will continuously trigger the fading effect.
         */
        m_textWidget.disableFadeEffect();

        /* Keep text (default font) in the middle, which means one empty
         * pixel row at the top and one between the text and the day lamps.
         * Don't use text widget alignment feature, because it will calculate
         * a 0 as optimum.
         */
        m_textWidget.move(0, 1);
    }

    /**
     * Destroy the view.
     */
    virtual ~DateTimeView32x8()
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

        m_textWidget.setFormatStr("{hc}No NTP");
        updateLampWidgetsColors();
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
        m_textWidget.setFont(Fonts::getFontByType(m_fontType));
    }

    /**
     * Update the underlying canvas.
     * 
     * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
     */
    void update(YAGfx& gfx) override
    {
        uint8_t idx = 0U;

        gfx.fillScreen(ColorDef::BLACK);
        m_textWidget.update(gfx);

        while(MAX_LAMPS > idx)
        {
            m_lampWidgets[idx].update(gfx);

            ++idx;
        }
    }

    /**
     * Get text (non-formatted).
     * 
     * @return Text
     */
    String getText() const override
    {
        return m_textWidget.getStr();
    }

    /**
     * Get text (formatted).
     * 
     * @return Text
     */
    String getFormatText() const override
    {
        return m_textWidget.getFormatStr();
    }

    /**
     * Set text (formatted).
     * 
     * @param[in] formatText    Formatted text to show.
     */
    void setFormatText(const String& formatText) override
    {
        m_textWidget.setFormatStr(formatText);
    }

    /**
     * Get the start of week configuration (Sunday = 0).
     *
     * @return uint8_t
     */
    uint8_t getStartOfWeek() const override
    {
        return m_startOfWeek;
    }

    /**
     * Set the start of week configuration (Sunday = 0).
     *
     * @param[in] startOfWeek uint8_t offset for starting of week
     *
     * @return bool success
     */
    bool setStartOfWeek(uint8_t startOfWeek) override
    {
        if (MAX_LAMPS <= startOfWeek)
        {
            LOG_WARNING("Illegal start of week value (%hhu).", startOfWeek);
            return false;
        }
        m_startOfWeek = startOfWeek;
        return true;
    }

    /**
     * Get the color to show the actual day.
     * 
     * @return Color
     */
    const Color& getDayOnColor() const override
    {
        return m_dayOnColor;
    }

    /**
     * Set the color which is used for the actual day.
     * 
     * @param[in] color Color for the actual day
     */
    void setDayOnColor(const Color& color) override
    {
        m_dayOnColor = color;
        updateLampWidgetsColors();
    }

    /**
     * Get the color to show the other days than the actual one.
     * 
     * @return Color
     */
    const Color& getDayOffColor() const override
    {
        return m_dayOffColor;
    }

    /**
     * Set the color which is used for the other days than the actual day.
     * 
     * @param[in] color Color for the other days
     */
    void setDayOffColor(const Color& color) override
    {
        m_dayOffColor = color;
        updateLampWidgetsColors();
    }

    /**
     * Get the view mode (analog, digital or both).
     * 
     * @return View mode 
     */
    ViewMode getViewMode() const override
    {
        return ViewMode::DIGITAL_ONLY;  /* 32X8 layout can only do digital. */
    }

    /**
     * Set the view mode (analog, digital or both).
     * 
     * @param[in] mode View mode
     * 
     * @return ViewMode 
     */
    bool setViewMode(ViewMode mode) override
    {
        bool isSuccessful = true;

        if (ViewMode::DIGITAL_ONLY != mode)
        {
            LOG_WARNING("Illegal DateTime view mode for 32X8: (%hhu)", mode);
            isSuccessful = false;
        }

        return isSuccessful;
    }

    /**
     * @brief Update current time values in view.
     * 
     * @param[in] now current time
     */
    virtual void setCurrentTime(const tm& now) override;

    /**
     * Get current active configuration in JSON format.
     * 
     * @param[out] jsonCfg Configuration
     */
    void getConfiguration(JsonObject& jsonCfg) const override
    {
        (void)jsonCfg;  /* No configuration for 32x8. */
    }

    /**
     * Apply configuration from JSON.
     * 
     * @param[in] jsonCfg Configuration
     * 
     * @return If successful set, it will return true otherwise false.
     */
    bool setConfiguration(const JsonObjectConst& jsonCfg) override
    {
        (void)jsonCfg;

        return true;
    }

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
    bool mergeConfiguration(JsonObject& jsonMerged, const JsonObjectConst& jsonSource) override
    {
        (void)jsonMerged;
        (void)jsonSource;

        return false; /* Nothing to merge. */
    }

    /** Max. number of lamps. One lamp per day in a week. */
    static const uint8_t    MAX_LAMPS       = 7U;

protected:

    /** Distance between two lamps in pixel. */
    static const uint8_t    LAMP_DISTANCE   = 1U;

    /** Lamp width in pixel. */
    static const uint8_t    LAMP_WIDTH      = (CONFIG_LED_MATRIX_WIDTH - ((MAX_LAMPS + 1U) * LAMP_DISTANCE)) / MAX_LAMPS;

    /** Lamp distance to the canvas border in pixel. */
    static const uint8_t    LAMP_BORDER     = (CONFIG_LED_MATRIX_WIDTH - (MAX_LAMPS * LAMP_WIDTH) - ((MAX_LAMPS - 1U) * LAMP_DISTANCE)) / 2U;

    /** Lamp height in pixel. */
    static const uint8_t    LAMP_HEIGHT     = 1U;

    /** Lamp 0 x-coordinate in pixel. */
    static const uint8_t    LAMP_0_X        = LAMP_BORDER + (0 * (LAMP_WIDTH + LAMP_DISTANCE));

    /** Lamp 1 x-coordinate in pixel. */
    static const uint8_t    LAMP_1_X        = LAMP_BORDER + (1 * (LAMP_WIDTH + LAMP_DISTANCE));

    /** Lamp 2 x-coordinate in pixel. */
    static const uint8_t    LAMP_2_X        = LAMP_BORDER + (2 * (LAMP_WIDTH + LAMP_DISTANCE));

    /** Lamp 3 x-coordinate in pixel. */
    static const uint8_t    LAMP_3_X        = LAMP_BORDER + (3 * (LAMP_WIDTH + LAMP_DISTANCE));

    /** Lamp 4 x-coordinate in pixel. */
    static const uint8_t    LAMP_4_X        = LAMP_BORDER + (4 * (LAMP_WIDTH + LAMP_DISTANCE));

    /** Lamp 5 x-coordinate in pixel. */
    static const uint8_t    LAMP_5_X        = LAMP_BORDER + (5 * (LAMP_WIDTH + LAMP_DISTANCE));

    /** Lamp 6 x-coordinate in pixel. */
    static const uint8_t    LAMP_6_X        = LAMP_BORDER + (6 * (LAMP_WIDTH + LAMP_DISTANCE));

    /** Lamp y-coordindate in pixel. */
    static const uint8_t    LAMP_Y          = CONFIG_LED_MATRIX_HEIGHT - 1U;

    /**
     * Text width in pixels.
     */
    static const uint16_t   TEXT_WIDTH      = CONFIG_LED_MATRIX_WIDTH;

    /**
     * Text height in pixels.
     */
    static const uint16_t   TEXT_HEIGHT     = CONFIG_LED_MATRIX_HEIGHT - LAMP_HEIGHT;

    /**
     * Text widget x-coordinate in pixels.
     */
    static const int16_t    TEXT_X          = 0;

    /**
     * Text widget y-coordinate in pixels.
     */
    static const int16_t    TEXT_Y          = 0;

    /** Start of week offset for the week bar (Sunday = 0). */
    static const uint8_t    START_OF_WEEK   = 1U;

    /** Color of the current day shown in the day of the week bar. */
    static const Color      DAY_ON_COLOR;

    /** Color of the other days (not the current one) shown in the day of the week bar. */
    static const Color      DAY_OFF_COLOR;

    Fonts::FontType m_fontType;                 /**< Font type which shall be used if there is no conflict with the layout. */
    TextWidget      m_textWidget;               /**< Text widget, used for showing the text. */
    LampWidget      m_lampWidgets[MAX_LAMPS];   /**< Lamp widgets, used to signal the day of week. */
    uint8_t         m_startOfWeek;              /**< Start of week offset for the week bar (Sunday = 0). */
    Color           m_dayOnColor;               /**< Color of current day in the day of the week bar. */
    Color           m_dayOffColor;              /**< Color of the other days in the day of the week bar. */

private:
    DateTimeView32x8(const DateTimeView32x8& other);
    DateTimeView32x8& operator=(const DateTimeView32x8& other);

    /**
     * Updates all colors of the lamp widgets.
     */
    void updateLampWidgetsColors();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* DATE_TIME_VIEW_32X8_H */

/** @} */

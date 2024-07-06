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
 * @brief  View for 64x64 LED matrix with date and time.
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup plugin
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
#include <IDateTimeView.h>
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
class DateTimeView64x64 : public IDateTimeView
{
public:

    /**
     * Construct the view.
     */
    DateTimeView64x64() :
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
        m_dayOnColor(DAY_ON_COLOR),
        m_dayOffColor(DAY_OFF_COLOR)
    {
    }

    /**
     * Destroy the view.
     */
    virtual ~DateTimeView64x64()
    {
    }

    /**
     * Initialize view, which will prepare the widgets and the default values.
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
     * Set weekday indicator depended on the given time info.
     *
     * @param[in] timeInfo the current time info.
     */
    void setWeekdayIndicator(tm timeInfo) override;

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

    /** Color of the current day shown in the day of the week bar. */
    static const Color      DAY_ON_COLOR;

    /** Color of the other days (not the current one) shown in the day of the week bar. */
    static const Color      DAY_OFF_COLOR;

    Fonts::FontType m_fontType;                 /**< Font type which shall be used if there is no conflict with the layout. */
    TextWidget      m_textWidget;               /**< Text widget, used for showing the text. */
    LampWidget      m_lampWidgets[MAX_LAMPS];   /**< Lamp widgets, used to signal the day of week. */
    Color           m_dayOnColor;               /**< Color of current day in the day of the week bar. */
    Color           m_dayOffColor;              /**< Color of the other days in the day of the week bar. */

    DateTimeView64x64(const DateTimeView64x64& other);
    DateTimeView64x64& operator=(const DateTimeView64x64& other);

    /**
     * Updates all colors of the lamp widgets.
     */
    void updateLampWidgetsColors();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* DATE_TIME_VIEW_64X64_H */

/** @} */

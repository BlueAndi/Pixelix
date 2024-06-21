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
 * @brief  Plugin view for 32x8 LED matrix
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup plugin
 *
 * @{
 */

#ifndef ICON_TEXT_LAMP_PLUGIN_VIEW_32X8_H
#define ICON_TEXT_LAMP_PLUGIN_VIEW_32X8_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <YAGfx.h>
#include <BitmapWidget.h>
#include <TextWidget.h>
#include <LampWidget.h>
#include <Util.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Internal plugin functionality. */
namespace _IconTextLampPlugin
{

/**
 * View for 32x8 LED matrix.
 */
class View
{
public:

    /**
     * Construct the layout.
     */
    View() :
        m_bitmapWidget(BITMAP_WIDTH, BITMAP_HEIGHT, 0, 0),
        m_textWidget(TEXT_WIDTH, TEXT_HEIGHT, TEXT_X, 0),
        m_lampWidgets{{LAMP_WIDTH, LAMP_HEIGHT, LAMP_0_X , LAMP_Y},
                      {LAMP_WIDTH, LAMP_HEIGHT, LAMP_1_X , LAMP_Y},
                      {LAMP_WIDTH, LAMP_HEIGHT, LAMP_2_X , LAMP_Y},
                      {LAMP_WIDTH, LAMP_HEIGHT, LAMP_3_X , LAMP_Y}}
    {
    }

    /**
     * Destroy the layout.
     */
    ~View()
    {
    }

    /**
     * Initialize view, which will prepare the widgets and the default values.
     */
    void init(uint16_t width, uint16_t height)
    {
        UTIL_NOT_USED(width);
        UTIL_NOT_USED(height);

        /* Move the text into the middle of the height, considering the the lamps. */
        if (TEXT_HEIGHT > m_textWidget.getFont().getHeight())
        {
            uint16_t diffY = TEXT_HEIGHT - m_textWidget.getFont().getHeight();
            uint16_t offsY = diffY / 2U;

            m_textWidget.move(BITMAP_WIDTH, offsY);
        }
    }

    /**
     * Update the underlying canvas.
     * 
     * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
     */
    void update(YAGfx& gfx)
    {
        uint8_t idx = 0U;

        gfx.fillScreen(ColorDef::BLACK);
        m_bitmapWidget.update(gfx);
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
    String getText() const
    {
        return m_textWidget.getStr();
    }

    /**
     * Get text (formatted).
     * 
     * @return Text
     */
    String getFormatText() const
    {
        return m_textWidget.getFormatStr();
    }

    /**
     * Set text (formatted).
     * 
     * @param[in] formatText    Formatted text to show.
     */
    void setFormatText(const String& formatText)
    {
        m_textWidget.setFormatStr(formatText);
    }

    /**
     * Load icon image from filesystem.
     *
     * @param[in] filename  Image filename
     *
     * @return If successul, it will return true otherwise false.
     */
    bool loadIcon(const String& filename);

    /**
     * Clear icon.
     */
    void clearIcon()
    {
        m_bitmapWidget.clear(ColorDef::BLACK);
    }

    /**
     * Get lamp state (true = on / false = off).
     * 
     * @param[in] lampId    Lamp id
     * 
     * @return Lamp state
     */
    bool getLamp(uint8_t lampId) const;

    /**
     * Set lamp state.
     *
     * @param[in] lampId    Lamp id
     * @param[in] state     Lamp state (true = on / false = off)
     */
    void setLamp(uint8_t lampId, bool state);

    /**
     * Max. number of lamps.
     */
    static const uint8_t    MAX_LAMPS       = 4U;

private:

    /**
     * Bitmap width in pixels.
     */
    static const uint16_t   BITMAP_WIDTH    = CONFIG_LED_MATRIX_HEIGHT;

    /**
     * Bitmap height in pixels.
     */
    static const uint16_t   BITMAP_HEIGHT   = CONFIG_LED_MATRIX_HEIGHT;


    /** Distance between two lamps in pixel. */
    static const uint8_t    LAMP_DISTANCE   = 1U;

    /** Lamp width in pixel. */
    static const uint8_t    LAMP_WIDTH      = (CONFIG_LED_MATRIX_WIDTH - BITMAP_WIDTH - ((MAX_LAMPS + 1U) * LAMP_DISTANCE)) / MAX_LAMPS;

    /** Lamp distance to the canvas border in pixel. */
    static const uint8_t    LAMP_BORDER     = (CONFIG_LED_MATRIX_WIDTH - BITMAP_WIDTH - (MAX_LAMPS * LAMP_WIDTH) - ((MAX_LAMPS - 1U) * LAMP_DISTANCE)) / 2U;

    /** Lamp height in pixel. */
    static const uint8_t    LAMP_HEIGHT     = 1U;

    /** Lamp 0 x-coordinate in pixel. */
    static const uint8_t    LAMP_0_X        = BITMAP_WIDTH + LAMP_BORDER + (0 * (LAMP_WIDTH + LAMP_DISTANCE));

    /** Lamp 1 x-coordinate in pixel. */
    static const uint8_t    LAMP_1_X        = BITMAP_WIDTH + LAMP_BORDER + (1 * (LAMP_WIDTH + LAMP_DISTANCE));

    /** Lamp 2 x-coordinate in pixel. */
    static const uint8_t    LAMP_2_X        = BITMAP_WIDTH + LAMP_BORDER + (2 * (LAMP_WIDTH + LAMP_DISTANCE));

    /** Lamp 3 x-coordinate in pixel. */
    static const uint8_t    LAMP_3_X        = BITMAP_WIDTH + LAMP_BORDER + (3 * (LAMP_WIDTH + LAMP_DISTANCE));

    /** Lamp y-coordindate in pixel. */
    static const uint8_t    LAMP_Y          = CONFIG_LED_MATRIX_HEIGHT - 1;

    /**
     * Text width in pixels.
     */
    static const uint16_t   TEXT_WIDTH      = CONFIG_LED_MATRIX_WIDTH - BITMAP_WIDTH;

    /**
     * Text height in pixels.
     */
    static const uint16_t   TEXT_HEIGHT     = CONFIG_LED_MATRIX_HEIGHT - LAMP_HEIGHT;

    /**
     * Text widget x-coordinate in pixels.
     */
    static const int16_t    TEXT_X          = BITMAP_WIDTH;

    BitmapWidget    m_bitmapWidget;             /**< Bitmap widget used to show a icon. */
    TextWidget      m_textWidget;               /**< Text widget used to show some text. */
    LampWidget      m_lampWidgets[MAX_LAMPS];   /**< Lamp widgets, used to signal different things. */

    View(const View& other);
    View& operator=(const View& other);
};

} /* _IconTextLampPlugin */

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* ICON_TEXT_LAMP_PLUGIN_VIEW_32X8_H */

/** @} */

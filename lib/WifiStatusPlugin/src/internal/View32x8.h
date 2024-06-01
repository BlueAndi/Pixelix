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

#ifndef WIFI_STATUS_PLUGIN_VIEW_32X8_H
#define WIFI_STATUS_PLUGIN_VIEW_32X8_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <YAGfx.h>
#include <Fonts.h>
#include <CanvasWidget.h>
#include <TextWidget.h>
#include <Util.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Internal plugin functionality. */
namespace _WifiStatusPlugin
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
        m_fontType(Fonts::FONT_TYPE_DEFAULT),
        m_canvasWidget(CANVAS_WIDTH, CANVAS_HEIGHT, 0, 0),
        m_textWidget(TEXT_WIDTH, TEXT_HEIGHT, TEXT_X, 0)
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

        m_textWidget.setFormatStr("\\calignWiFi");

        /* Move the text into the middle of the height. */
        if (height > m_textWidget.getFont().getHeight())
        {
            uint16_t diffY = height - m_textWidget.getFont().getHeight();
            uint16_t offsY = diffY / 2U;

            m_textWidget.move(CANVAS_WIDTH, offsY);
        }
    }

    /**
     * Get font type.
     * 
     * @return The font type the view uses.
     */
    Fonts::FontType getFontType() const
    {
        return m_fontType;
    }

    /**
     * Set font type.
     * 
     * @param[in] fontType  The font type which the view shall use.
     */
    void setFontType(Fonts::FontType fontType)
    {
        m_fontType = fontType;
        m_textWidget.setFont(Fonts::getFontByType(m_fontType));
    }

    /**
     * Update the underlying canvas.
     * 
     * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
     */
    void update(YAGfx& gfx)
    {
        m_canvasWidget.update(gfx);
        m_textWidget.update(gfx);
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
     * Get text color.
     *
     * @return Text color
     */
    Color getTextColor() const
    {
        return m_textWidget.getTextColor();
    }

    /**
     * Set text color.
     *
     * @param[in] color Text color
     */
    void setTextColor(const Color& color)
    {
        m_textWidget.setTextColor(color);
    }

    /**
     * Update wifi status.
     *
     * @param[in] quality   Signal quality in percent [0; 100].
     */
    void updateWifiStatus(uint8_t quality);

private:

    /**
     * Width in pixel of a single signal strength bar.
     */
    static const uint16_t   WIFI_BAR_WIDTH          = 2U;

    /**
     * Height in pixel of the whole signal strength icon.
     */
    static const uint16_t   WIFI_ICON_HEIGHT        = 8U;

    /**
     * Width in pixel of a single signal strength bar.
     */
    static const uint16_t   WIFI_BAR_SPACE_WIDTH    = 1U;

    /**
     * Height in pixel of the lowest signal strength bar.
     */
    static const uint16_t   WIFI_BAR_HEIGHT         = 2U;

    /**
     * Number of signal signal strength bars.
     */
    static const uint8_t    WIFI_BARS               = 4U;

    /**
     * Width in pixel of the whole signal strength icon.
     * Between each bar is a short space.
     */
    static const uint16_t   WIFI_ICON_WIDTH         = (WIFI_BARS * WIFI_BAR_WIDTH) + ((WIFI_BARS - 1U) * WIFI_BAR_SPACE_WIDTH);

    /**
     * Canvas width in pixels.
     */
    static const uint16_t   CANVAS_WIDTH            = WIFI_ICON_WIDTH;

    /**
     * Canvas height in pixels.
     */
    static const uint16_t   CANVAS_HEIGHT           = CONFIG_LED_MATRIX_HEIGHT;

    /**
     * Text width in pixels.
     */
    static const uint16_t   TEXT_WIDTH              = CONFIG_LED_MATRIX_WIDTH - CANVAS_WIDTH;

    /**
     * Text height in pixels.
     */
    static const uint16_t   TEXT_HEIGHT             = CONFIG_LED_MATRIX_HEIGHT;

    /**
     * Text widget x-coordinate in pixels.
     */
    static const int16_t    TEXT_X                  = CANVAS_WIDTH;

    Fonts::FontType m_fontType;     /**< Font type which shall be used if there is no conflict with the layout. */
    CanvasWidget    m_canvasWidget; /**< Canvas widget used to draw the signal strenth. */
    TextWidget      m_textWidget;   /**< Text widget used to show some text. */

    View(const View& other);
    View& operator=(const View& other);
};

} /* _WifiStatusPlugin */

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* WIFI_STATUS_PLUGIN_VIEW_32X8_H */

/** @} */

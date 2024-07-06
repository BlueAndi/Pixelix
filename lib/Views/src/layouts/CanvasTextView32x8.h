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
 * @brief  Generic view with canvas and text for LED matrix
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup plugin
 *
 * @{
 */

#ifndef CANVAS_TEXT_VIEW_32X8_H
#define CANVAS_TEXT_VIEW_32X8_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <YAGfx.h>
#include <Fonts.h>
#include <ICanvasTextView.h>
#include <CanvasWidget.h>
#include <TextWidget.h>
#include <Util.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * View for 32x8 LED matrix with canvas and text.
 */
class CanvasTextView32x8 : public ICanvasTextView
{
public:

    /**
     * Construct the view.
     */
    CanvasTextView32x8() :
        ICanvasTextView(),
        m_fontType(Fonts::FONT_TYPE_DEFAULT),
        m_canvasWidget(CANVAS_WIDTH, CANVAS_HEIGHT, CANVAS_X, CANVAS_Y),
        m_textWidget(TEXT_WIDTH, TEXT_HEIGHT, TEXT_X, TEXT_Y)
    {
    }

    /**
     * Destroy the view.
     */
    virtual ~CanvasTextView32x8()
    {
    }

    /**
     * Initialize view, which will prepare the widgets and the default values.
     */
    void init(uint16_t width, uint16_t height) override
    {
        UTIL_NOT_USED(width);
        UTIL_NOT_USED(height);
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
        gfx.fillScreen(ColorDef::BLACK);
        m_canvasWidget.update(gfx);
        m_textWidget.update(gfx);
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
     * Get canvas for drawing.
     * 
     * @return Canvas
     */
    YAGfx& getCanvasGfx() override
    {
        return m_canvasWidget;
    }

protected:

    /**
     * Canvas width in pixels.
     */
    static const uint16_t   CANVAS_WIDTH    = 12U;

    /**
     * Canvas height in pixels.
     */
    static const uint16_t   CANVAS_HEIGHT   = CONFIG_LED_MATRIX_HEIGHT;

    /**
     * Canvas widget x-coordinate in pixels.
     * Left aligned.
     */
    static const int16_t    CANVAS_X        = 0;

    /**
     * Canvas widget y-coordinate in pixels.
     * Top aligned.
     */
    static const int16_t    CANVAS_Y        = 0;

    /**
     * Text width in pixels.
     */
    static const uint16_t   TEXT_WIDTH      = CONFIG_LED_MATRIX_WIDTH - CANVAS_WIDTH;

    /**
     * Text height in pixels.
     */
    static const uint16_t   TEXT_HEIGHT     = CONFIG_LED_MATRIX_HEIGHT;

    /**
     * Text widget x-coordinate in pixels.
     */
    static const int16_t    TEXT_X          = CANVAS_WIDTH;

    /**
     * Text widget y-coordinate in pixels.
     * Top aligned, below bitmap.
     */
    static const int16_t    TEXT_Y          = 0;

    Fonts::FontType m_fontType;     /**< Font type which shall be used if there is no conflict with the layout. */
    CanvasWidget    m_canvasWidget; /**< Canvas widget used to draw. */
    TextWidget      m_textWidget;   /**< Text widget used to show some text. */

    CanvasTextView32x8(const CanvasTextView32x8& other);
    CanvasTextView32x8& operator=(const CanvasTextView32x8& other);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* CANVAS_TEXT_VIEW_32X8_H */

/** @} */

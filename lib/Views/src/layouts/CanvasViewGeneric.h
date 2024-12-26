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
 * @brief  Generic view with bitmap and canvas for LED matrix
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup plugin
 *
 * @{
 */

#ifndef CANVAS_VIEW_GENERIC_H
#define CANVAS_VIEW_GENERIC_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <YAGfx.h>
#include <ICanvasView.h>
#include <BitmapWidget.h>
#include <CanvasWidget.h>
#include <Util.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Generic view for LED matrix with bitmap and canvas.
 */
class CanvasViewGeneric : public ICanvasView
{
public:

    /**
     * Construct the view.
     */
    CanvasViewGeneric() :
        ICanvasView(),
        m_bitmapWidget(BITMAP_WIDTH, BITMAP_HEIGHT, BITMAP_X, BITMAP_Y),
        m_canvasWidget(CANVAS_WIDTH, CANVAS_HEIGHT, CANVAS_X, CANVAS_Y)
    {
    }

    /**
     * Destroy the view.
     */
    virtual ~CanvasViewGeneric()
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
     * Update the underlying canvas.
     * 
     * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
     */
    void update(YAGfx& gfx) override
    {
        gfx.fillScreen(ColorDef::BLACK);
        m_bitmapWidget.update(gfx);
        m_canvasWidget.update(gfx);
    }

    /**
     * Load icon image from filesystem.
     *
     * @param[in] filename  Image filename
     *
     * @return If successul, it will return true otherwise false.
     */
    bool loadIcon(const String& filename) override;

    /**
     * Clear icon.
     */
    void clearIcon() override
    {
        m_bitmapWidget.clear(ColorDef::BLACK);
    }

protected:

    /**
     * Bitmap width in pixels.
     */
    static const uint16_t   BITMAP_WIDTH    = CONFIG_LED_MATRIX_WIDTH;

    /**
     * Bitmap height in pixels.
     */
    static const uint16_t   BITMAP_HEIGHT   = CONFIG_LED_MATRIX_HEIGHT;

    /**
     * Bitmap widget x-coordinate in pixels.
     * Left aligned.
     */
    static const int16_t    BITMAP_X        = 0;

    /**
     * Bitmap widget y-coordinate in pixels.
     * Top aligned.
     */
    static const int16_t    BITMAP_Y        = 0;

    /**
     * Canvas width in pixels.
     */
    static const uint16_t   CANVAS_WIDTH    = CONFIG_LED_MATRIX_WIDTH;

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

    BitmapWidget    m_bitmapWidget; /**< Bitmap widget used to show a icon. */
    CanvasWidget    m_canvasWidget; /**< Canvas used for drawing. */

    CanvasViewGeneric(const CanvasViewGeneric& other);
    CanvasViewGeneric& operator=(const CanvasViewGeneric& other);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* CANVAS_VIEW_GENERIC_H */

/** @} */

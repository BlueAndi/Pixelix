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
 * @brief  Canvas widget
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef CANVAS_WIDGET_H
#define CANVAS_WIDGET_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <WString.h>
#include <Widget.hpp>
#include <YAGfxBitmap.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This class defines a canvas widget.
 */
class CanvasWidget : public Widget, public YAGfx
{
public:

    /**
     * Constructs a canvas widget.
     * 
     * @param[in] width     Widget width in pixel.
     * @param[in] height    Widget height in pixel.
     * @param[in] x         Upper left corner (x-coordinate) of the widget in a canvas.
     * @param[in] y         Upper left corner (y-coordinate) of the widget in a canvas.
     */
    CanvasWidget(uint16_t width = 0U, uint16_t height = 0U, int16_t x = 0, int16_t y = 0) :
        Widget(WIDGET_TYPE, width, height, x, y),
        YAGfx(),
        m_bitmap(width, height)
    {
    }

    /**
     * Constructs a canvas widget by copy.
     * 
     * @param[in] canvas Source canvas which to copy.
     */
    CanvasWidget(const CanvasWidget& canvas) :
        Widget(canvas),
        YAGfx(),
        m_bitmap(canvas.m_bitmap)
    {
    }

    /**
     * Destroys the canvas widget.
     */
    ~CanvasWidget()
    {
    }

    /**
     * Assigns a canvas widget.
     * 
     * @param[in] canvas Source canvas widget which to assign.
     * 
     * @return Canvas widget
     */
    CanvasWidget& operator=(const CanvasWidget& canvas)
    {
        if (&canvas != this)
        {
            Widget::operator=(canvas);

            m_bitmap = canvas.m_bitmap;
        }

        return *this;
    }

    /**
     * Get canvas width in pixel.
     *
     * @return Canvas width in pixel.
     */
    uint16_t getWidth() const final
    {
        return Widget::getWidth();
    }

    /**
     * Set widget width.
     * 
     * @param[in] width Width in pixel
     */
    void setWidth(uint16_t width) final
    {
        /* If the width changes, the bitmap size will be adjusted. */
        if (width != Widget::getWidth())
        {
            m_bitmap.release();
            Widget::setWidth(width);
            (void)m_bitmap.create(Widget::getWidth(), Widget::getHeight());
        }
    }

    /**
     * Get canvas height in pixel.
     *
     * @return Canvas height in pixel.
     */
    uint16_t getHeight() const final
    {
        return Widget::getHeight();
    }

    /**
     * Set widget height.
     * 
     * @param[in] height Height in pixel
     */
    void setHeight(uint16_t height) final
    {
        /* If the height changes, the bitmap size will be adjusted. */
        if (height != Widget::getHeight())
        {
            m_bitmap.release();
            Widget::setHeight(height);
            (void)m_bitmap.create(Widget::getWidth(), Widget::getHeight());
        }
    }

    /**
     * Get pixel color at given position.
     * Attention, it will only return a valid color in case of active painting!
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color in RGB888 format.
     */
    Color& getColor(int16_t x, int16_t y) final
    {
        return m_bitmap.getColor(x, y);
    }

    /**
     * Get pixel color at given position.
     * Attention, it will only return a valid color in case of active painting!
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color in RGB888 format.
     */
    const Color& getColor(int16_t x, int16_t y) const final
    {
        return m_bitmap.getColor(x, y);
    }

    /**
     * Draw a single pixel at given position.
     * 
     * It will NOT be checked whether the pixel is outside the canvas.
     * The other graphic functions will ensure to draw inside the canvas
     * and may call drawPixel() quite often. This shall keep the processor
     * load smaller.
     *
     * @param[in] x     x-coordinate
     * @param[in] y     y-coordinate
     * @param[in] color Color
     */
    void drawPixel(int16_t x, int16_t y, const Color& color) final
    {
        m_bitmap.drawPixel(x, y, color);
    }

    /**
     * Get the address inside the framebuffer at certain coordinates.
     * If the requested length is not available, it will return nullptr.
     * 
     * To address pixel by pixel on the x-axis, the returned offset shall be considered.
     * Otherwise its not guaranteed to address out of bounds!
     * 
     * @param[in] x         x-coordinate
     * @param[in] y         y-coordinate
     * @param[in] length    Requested number of colors on x-axis.
     * @param[out] offset   Address offset in pixel which to use to calculate address of next pixel.
     * 
     * @return Address in the framebuffer or nullptr.
     */
    Color* getFrameBufferXAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) final
    {
        return m_bitmap.getFrameBufferXAddr(x, y, length, offset);
    }

    /**
     * Get the address inside the framebuffer at certain coordinates.
     * If the requested length is not available, it will return nullptr.
     * 
     * To address pixel by pixel on the x-axis, the returned offset shall be considered.
     * Otherwise its not guaranteed to address out of bounds!
     * 
     * @param[in] x         x-coordinate
     * @param[in] y         y-coordinate
     * @param[in] length    Requested number of colors on x-axis.
     * @param[out] offset   Address offset in pixel which to use to calculate address of next pixel.
     * 
     * @return Address in the framebuffer or nullptr.
     */
    const Color* getFrameBufferXAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) const final
    {
        return m_bitmap.getFrameBufferXAddr(x, y, length, offset);
    }

    /**
     * Get the address inside the framebuffer at certain coordinates.
     * If the requested length is not available, it will return nullptr.
     * 
     * To address pixel by pixel on the y-axis, the returned offset shall be considered.
     * Otherwise its not guaranteed to address out of bounds!
     * 
     * @param[in] x         x-coordinate
     * @param[in] y         y-coordinate
     * @param[in] length    Requested number of colors on y-axis.
     * @param[out] offset   Address offset in pixel which to use to calculate address of next pixel.
     * 
     * @return Address in the framebuffer or nullptr.
     */
    Color* getFrameBufferYAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) final
    {
        return m_bitmap.getFrameBufferYAddr(x, y, length, offset);
    }

    /**
     * Get the address inside the framebuffer at certain coordinates.
     * If the requested length is not available, it will return nullptr.
     * 
     * To address pixel by pixel on the y-axis, the returned offset shall be considered.
     * Otherwise its not guaranteed to address out of bounds!
     * 
     * @param[in] x         x-coordinate
     * @param[in] y         y-coordinate
     * @param[in] length    Requested number of colors on y-axis.
     * @param[out] offset   Address offset in pixel which to use to calculate address of next pixel.
     * 
     * @return Address in the framebuffer or nullptr.
     */
    const Color* getFrameBufferYAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) const final
    {
        return m_bitmap.getFrameBufferYAddr(x, y, length, offset);
    }

    /** Widget type string */
    static const char*      WIDGET_TYPE;

private:

    YAGfxDynamicBitmap m_bitmap; /**< Bitmap which is used to draw inside. */

    /**
     * Paint the widget with the given graphics interface.
     * 
     * @param[in] gfx   Graphics interface
     */
    void paint(YAGfx& gfx) override
    {
        gfx.drawBitmap(0, 0, m_bitmap);
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* CANVAS_WIDGET_H */

/** @} */
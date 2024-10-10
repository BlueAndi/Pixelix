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
 * @brief  Basic graphics bitmap canvas
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef BASE_GFX_CANVAS_HPP
#define BASE_GFX_CANVAS_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <BaseGfx.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The canvas ensures the graphic operations take place inside a dedicated
 * window. The window itself can be moved to a position inside the parent
 * canvas.
 * 
 * @tparam TColor The color representation.
 */
template < typename TColor >
class BaseGfxCanvas : public BaseGfx<TColor>
{
public:

    /**
     * Constructs a canvas.
     * 
     * @param[in] parentGfx The graphic operations of the parent canvas.
     * @param[in] offsX     The x offset in the parent canvas.
     * @param[in] offsY     The y offset in the parent canvas.
     * @param[in] width     The canvas width in pixels.
     * @param[in] height    The canvas height in pixels.
     */
    BaseGfxCanvas(BaseGfx<TColor>* parentGfx = nullptr, int16_t offsX = 0, int16_t offsY = 0, uint16_t width = 0U, uint16_t height = 0U) :
        BaseGfx<TColor>(),
        m_parentGfx(parentGfx),
        m_offsX(offsX),
        m_offsY(offsY),
        m_width(width),
        m_height(height)
    {
    }

    /**
     * Constructs a canvas by copy.
     * 
     * @param[in] canvas    Source canvas which to copy.
     */
    BaseGfxCanvas(const BaseGfxCanvas& canvas) :
        BaseGfx<TColor>(canvas),
        m_parentGfx(canvas.m_parentGfx),
        m_offsX(canvas.m_offsX),
        m_offsY(canvas.m_offsY),
        m_width(canvas.m_width),
        m_height(canvas.m_height)
    {
    }

    /**
     * Destroys the canvas.
     */
    virtual ~BaseGfxCanvas()
    {
    }

    /**
     * Assigns a canvas.
     * 
     * @param[in] canvas    Source canvas which to assign.
     * 
     * @return Canvas
     */
    BaseGfxCanvas& operator=(const BaseGfxCanvas& canvas)
    {
        if (&canvas != this)
        {
            m_parentGfx = canvas.m_parentGfx;
            m_offsX     = canvas.m_offsX;
            m_offsY     = canvas.m_offsY;
            m_width     = canvas.m_width;
            m_height    = canvas.m_height;
        }

        return *this;
    }

    /**
     * Get the parent canvas graphic operations.
     * 
     * @return Parent graphic functions
     */
    BaseGfx<TColor>* getParentGfx() const
    {
        return m_parentGfx;
    }

    /**
     * Set parent canvas graphic operations.
     * 
     * @param[in] gfx   Parent graphic functions
     */
    void setParentGfx(BaseGfx<TColor>& gfx)
    {
        m_parentGfx = &gfx;
    }

    /**
     * Get x offset in the parent canvas.
     * 
     * @return x offset in the parent canvas
     */
    int16_t getOffsetX() const
    {
        return m_offsX;
    }

    /**
     * Set x offset in the parent canvas.
     * 
     * @param[in] offsX x offset in the parent canvas
     */
    void setOffsetX(int16_t offsX)
    {
        m_offsX = offsX;
    }

    /**
     * Get y offset in the parent canvas.
     * 
     * @return y offset in the parent canvas
     */
    int16_t getOffsetY() const
    {
        return m_offsY;
    }

    /**
     * Set y offset in the parent canvas.
     * 
     * @param[in] offsY y offset in the parent canvas
     */
    void setOffsetY(int16_t offsY)
    {
        m_offsY = offsY;
    }

    /**
     * Set canvas width in pixels.
     * 
     * @param[in] width Canvas width in pixels
     */
    void setWidth(uint16_t width)
    {
        m_width = width;
    }

    /**
     * Set canvas height in pixels.
     * 
     * @param[in] height Canvas height in pixels
     */
    void setHeight(uint16_t height)
    {
        m_height = height;
    }

    /**
     * Get canvas width in pixel.
     *
     * @return Map canvas width in pixel.
     */
    uint16_t getWidth() const final
    {
        return m_width;
    }

    /**
     * Get canvas height in pixel.
     *
     * @return Canvas height in pixel.
     */
    uint16_t getHeight() const final
    {
        return m_height;
    }

    /**
     * Get pixel color at given position.
     * This is used for color manipulation in higher layers.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color
     */
    TColor& getColor(int16_t x, int16_t y) final
    {
        static TColor   trash;
        TColor*         pixel   = &trash;

        if ((nullptr != m_parentGfx) &&
            (0 <= x) &&
            (0 <= y) &&
            (m_width > x) &&
            (m_height > y))
        {
            int16_t absX = x + m_offsX;
            int16_t absY = y + m_offsY;

            pixel = &m_parentGfx->getColor(absX, absY);
        }

        return *pixel;
    }

    /**
     * Get pixel color at given position.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color
     */
    const TColor& getColor(int16_t x, int16_t y) const final
    {
        static TColor   trash;
        const TColor*   pixel   = &trash;

        if ((nullptr != m_parentGfx) &&
            (0 <= x) &&
            (0 <= y) &&
            (m_width > x) &&
            (m_height > y))
        {
            int16_t absX = x + m_offsX;
            int16_t absY = y + m_offsY;

            pixel = &m_parentGfx->getColor(absX, absY);
        }

        return *pixel;
    }

    /**
     * Draw a single pixel at given position.
     *
     * @param[in] x     x-coordinate
     * @param[in] y     y-coordinate
     * @param[in] color Color
     */
    void drawPixel(int16_t x, int16_t y, const TColor& color) final
    {
        if ((nullptr != m_parentGfx) &&
            (0 <= x) &&
            (0 <= y) &&
            (m_width > x) &&
            (m_height > y))
        {
            int16_t absX = x + m_offsX;
            int16_t absY = y + m_offsY;

            m_parentGfx->drawPixel(absX, absY, color);
        }
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
    TColor* getFrameBufferXAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) final
    {
        TColor* addr = nullptr;

        if ((nullptr != m_parentGfx) &&
            (m_width >= (x + length)))
        {
            int16_t absX = x + m_offsX;
            int16_t absY = y + m_offsY;

            addr = m_parentGfx->getFrameBufferXAddr(absX, absY, length, offset);
        }

        return addr;
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
    const TColor* getFrameBufferXAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) const final
    {
        const TColor* addr = nullptr;

        if ((nullptr != m_parentGfx) &&
            (m_width >= (x + length)))
        {
            int16_t absX = x + m_offsX;
            int16_t absY = y + m_offsY;

            addr = m_parentGfx->getFrameBufferXAddr(absX, absY, length, offset);
        }

        return addr;
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
    TColor* getFrameBufferYAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) final
    {
        TColor* addr = nullptr;

        if ((nullptr != m_parentGfx) &&
            (m_height >= (y + length)))
        {
            int16_t absX = x + m_offsX;
            int16_t absY = y + m_offsY;

            addr = m_parentGfx->getFrameBufferXAddr(absX, absY, length, offset);
        }

        return addr;
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
    const TColor* getFrameBufferYAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) const final
    {
        const TColor* addr = nullptr;

        if ((nullptr != m_parentGfx) &&
            (m_height >= (y + length)))
        {
            int16_t absX = x + m_offsX;
            int16_t absY = y + m_offsY;

            addr = m_parentGfx->getFrameBufferXAddr(absX, absY, length, offset);
        }

        return addr;
    }

private:

    BaseGfx<TColor>*    m_parentGfx;    /**< The parent graphic operations. */
    int16_t             m_offsX;        /**< The x offset in the parent canvas. */
    int16_t             m_offsY;        /**< The y offset in the parent canvas. */
    uint16_t            m_width;        /**< Canvas width in pixels. */
    uint16_t            m_height;       /**< Canvas height in pixels. */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* BASE_GFX_CANVAS_HPP */

/** @} */
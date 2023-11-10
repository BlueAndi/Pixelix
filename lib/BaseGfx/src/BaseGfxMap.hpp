/* MIT License
 *
 * Copyright (c) 2019 - 2023 Andreas Merkle <web@blue-andi.de>
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

#ifndef BASE_GFX_MAP_HPP
#define BASE_GFX_MAP_HPP

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
 * A map is like a window over graphic operations. It defines the window size
 * and the offset. It ensures that drawing is kept inside its defined borders
 * (width, height).
 * 
 * @tparam TColor The color representation.
 */
template < typename TColor >
class BaseGfxMap : public BaseGfx<TColor>
{
public:

    /**
     * Constructs a map canvas.
     */
    BaseGfxMap() :
        BaseGfx<TColor>(),
        m_gfx(nullptr),
        m_offsX(0),
        m_offsY(0),
        m_width(0U),
        m_height(0U)
    {
    }

    /**
     * Constructs a map canvas.
     * 
     * @param[in] gfx       The graphic operations of the underlying canvas.
     * @param[in] offsX     The x offset in the underlying canvas.
     * @param[in] offsY     The y offset in the underlying canvas.
     * @param[in] width     The map canvas width in pixels.
     * @param[in] height    The map canvas height in pixels.
     */
    BaseGfxMap(BaseGfx<TColor>& gfx, int16_t offsX = 0, int16_t offsY = 0, uint16_t width = 0U, uint16_t height = 0U) :
        BaseGfx<TColor>(),
        m_gfx(&gfx),
        m_offsX(offsX),
        m_offsY(offsY),
        m_width(width),
        m_height(height)
    {
    }

    /**
     * Constructs a map canvas by copy.
     * 
     * @param[in] map   Source map which to copy.
     */
    BaseGfxMap(const BaseGfxMap& map) :
        BaseGfx<TColor>(map),
        m_gfx(map.m_gfx),
        m_offsX(map.m_offsX),
        m_offsY(map.m_offsY),
        m_width(map.m_width),
        m_height(map.m_height)
    {
    }

    /**
     * Destroys the map canvas.
     */
    virtual ~BaseGfxMap()
    {
    }

    /**
     * Assigns a map.
     * 
     * @param[in] map   Source map which to assign.
     * 
     * @return Canvas
     */
    BaseGfxMap& operator=(const BaseGfxMap& map)
    {
        if (&map != this)
        {
            m_gfx       = map.m_gfx;
            m_offsX     = map.m_offsX;
            m_offsY     = map.m_offsY;
            m_width     = map.m_width;
            m_height    = map.m_height;
        }

        return *this;
    }

    /**
     * Set canvas graphic operations.
     * 
     * @param[in] gfx   Graphic functions
     */
    void setGfx(BaseGfx<TColor>& gfx)
    {
        m_gfx = &gfx;
    }

    /**
     * Get x offset in the underlying canvas.
     * 
     * @return x offset in the underlying canvas
     */
    int16_t getOffsetX() const
    {
        return m_offsX;
    }

    /**
     * Set x offset in the underlying canvas.
     * 
     * @param[in] offsX x offset in the underlying canvas
     */
    void setOffsetX(int16_t offsX)
    {
        m_offsX = offsX;
    }

    /**
     * Get y offset in the underlying canvas.
     * 
     * @return y offset in the underlying canvas
     */
    int16_t getOffsetY() const
    {
        return m_offsY;
    }

    /**
     * Set y offset in the underlying canvas.
     * 
     * @param[in] offsY y offset in the underlying canvas
     */
    void setOffsetY(int16_t offsY)
    {
        m_offsY = offsY;
    }

    /**
     * Set map canvas width in pixels.
     * 
     * @param[in] width Map canvas width in pixels
     */
    void setWidth(uint16_t width)
    {
        m_width = width;
    }

    /**
     * Set map canvas height in pixels.
     * 
     * @param[in] height Map canvas height in pixels
     */
    void setHeight(uint16_t height)
    {
        m_height = height;
    }

    /**
     * Get map canvas width in pixel.
     *
     * @return Map canvas width in pixel
     */
    uint16_t getWidth() const final
    {
        return m_width;
    }

    /**
     * Get map canvas height in pixel.
     *
     * @return Map canvas height in pixel
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

        if ((nullptr != m_gfx) &&
            (0 <= x) &&
            (0 <= y) &&
            (m_width > x) &&
            (m_height > y))
        {
            pixel = &m_gfx->getColor(x + m_offsX, y + m_offsY);
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

        if ((nullptr != m_gfx) &&
            (0 <= x) &&
            (0 <= y) &&
            (m_width > x) &&
            (m_height > y))
        {
            pixel = &m_gfx->getColor(x + m_offsX, y + m_offsY);
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
        if ((nullptr != m_gfx) &&
            (0 <= x) &&
            (0 <= y) &&
            (m_width > x) &&
            (m_height > y))
        {
            m_gfx->drawPixel(x + m_offsX, y + m_offsY, color);
        }
    }

private:

    BaseGfx<TColor>*    m_gfx;      /**< The underlying graphic operations. */
    int16_t             m_offsX;    /**< The x offset in the underlying canvas. */
    int16_t             m_offsY;    /**< The y offset in the underlying canvas. */
    uint16_t            m_width;    /**< Map canvas width in pixels. */
    uint16_t            m_height;   /**< Map canvas height in pixels. */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* BASE_GFX_MAP_HPP */

/** @} */
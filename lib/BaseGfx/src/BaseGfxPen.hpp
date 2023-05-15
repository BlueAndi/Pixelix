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
 * @brief  Base graphics pen
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef BASE_GFX_PEN_HPP
#define BASE_GFX_PEN_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "BaseGfx.hpp"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The graphics draw pen can be used for continously drawing by only using
 * always the destination coordinates. It is color agnostic like the base
 * graphic functions.
 *
 * Note, the type for coordinates must be a signed type!
 *
 */
template < typename TColor >
class BaseGfxPen
{
public:

    /**
     * Destroys a graphics pen.
     */
    ~BaseGfxPen()
    {
    }

    /**
     * Get pen color.
     *
     * @return Pen color
     */
    TColor getColor() const
    {
        return m_pen;
    }

    /**
     * Set pen color.
     *
     * @param[in] color Pen color which to set
     */
    void setColor(const TColor& color)
    {
        m_pen = color;
    }

    /**
     * Plot a pixel at given position with pen.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     */
    void plot(int16_t x, int16_t y)
    {
        m_gfx.drawPixel(x, y, m_color);

        m_x = x;
        m_y = y;
    }

    /**
     * Get pen position.
     *
     * @param[out] x x-coordinate
     * @param[out] y y-coordinate
     */
    void getPos(int16_t& x, int16_t& y) const
    {
        x = m_x;
        y = m_y;
    }

    /**
     * Move pen to given coordinates.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     */
    void moveTo(int16_t x, int16_t y)
    {
        m_x = x;
        m_y = y;
    }

    /**
     * Draw line from current pen location to the given
     * coordinates.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     */
    void lineTo(int16_t x, int16_t y)
    {
        m_gfx.line(m_x, m_y, x, y, m_color);

        m_x = x;
        m_y = y;
    }

protected:

    BaseGfx<TColor>&    m_gfx;      /**< Base gfx */
    TColor              m_color;    /**< Pen color */
    int16_t             m_x;        /**< Pen x-coordinate */
    int16_t             m_y;        /**< Pen y-coordinate */

    /**
     * Constructs a base graphics pen.
     */
    BaseGfxPen(BaseGfx& gfx) :
        m_gfx(gfx),
        m_color(),
        m_x(0),
        m_y(0)
    {
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* BASE_GFX_PEN_HPP */

/** @} */
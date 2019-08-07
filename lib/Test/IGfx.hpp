/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
@brief  Abstract graphics interface for test
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
This module provides the general graphics interface for testing purposes.

*******************************************************************************/
/** @defgroup igfx Abstract graphics interface for test
 * This module provides the abstract graphics interface for testing purposes.
 *
 * @{
 */

#ifndef __IGFX_HPP__
#define __IGFX_HPP__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Abstract graphics interface for testing purposes.
 */
class IGfx
{
public:

    /**
     * Constructs the abstract graphics interface, which can be used to
     * draw inside a defined area.
     * 
     * @param[in] width     Area width in pixel
     * @param[in] height    Area height in pixel
     */
    IGfx(int16_t width, int16_t height) :
        m_width(width),
        m_height(height)
    {
    }

    /**
     * Destroys the abstract graphics interface.
     */
    virtual ~IGfx()
    {
    }

    /**
     * Get width in pixel.
     * 
     * @return Width in pixel
     */
    int16_t getWidth(void) const
    {
        return m_width;
    }

    /**
     * Get height in pixel.
     * 
     * @return Height in pixel
     */
    int16_t getHeight(void) const
    {
        return m_height;
    }

    /**
     * Draw a single pixel in the matrix and ensure that the drawing borders
     * are not violated.
     * Note, method declaration is from Adafruit_GFX.
     * 
     * @param[in] x     x-coordinate
     * @param[in] y     y-coordinate
     * @param[in] color Pixel color
     */
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;

    /**
     * Fill a rectangle.
     * Note, method declaration is from Adafruit_GFX.
     * 
     * @param[in] x         Upper left corner x-coordinate
     * @param[in] y         Upper left corner y-coordinate
     * @param[in] width     Rectangle width in pixel
     * @param[in] height    Rectangle height in pixel
     * @param[in] color     Color
     */
    void fillRect(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t color)
    {
        int16_t relX = 0;
        int16_t relY = 0;

        for(relY = 0; relY < height; ++relY)
        {
            for(relX = 0; relX < width; ++relX)
            {
                drawPixel(x + relX, y + relY, color);
            }
        }

        return;
    }

private:

    int16_t m_width;    /**< Width in pixel */
    int16_t m_height;   /**< Height in pixel */

    IGfx(const IGfx& gfx);
    IGfx& operator=(const IGfx& gfx);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __IGFX_HPP__ */

/** @} */
/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Graphics interface
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
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
#include <Adafruit_GFX.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Graphics interface, based on the Adafruit GFX with extensions.
 */
class IGfx : public Adafruit_GFX
{
public:

    /**
     * Constructs the graphics interface.
     *
     * @param[in] width     Display width in pixel
     * @param[in] height    Display height in pixel
     */
    IGfx(int16_t width, int16_t height) :
        Adafruit_GFX(width, height)
    {
    }

    /**
     * Destroys the graphics interface.
     */
    virtual ~IGfx()
    {
    }

    /**
     * Get pixel color at given position.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color in RGB565 format.
     */
    virtual uint16_t getColor(int16_t x, int16_t y) = 0;

private:

    /* Default constructor not allowed. */
    IGfx();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __IGFX_HPP__ */

/** @} */
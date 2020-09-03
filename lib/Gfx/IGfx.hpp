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
#include <BaseGfx.hpp>
#include <Color.h>
#include <Print.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Pixelix graphics operations with RGB888 color format.
 */
typedef BaseGfx<Color> PixelixGfx;

/**
 * Graphics interface, based on the Adafruit GFX with extensions.
 */
class IGfx : public PixelixGfx, public Print
{
public:

    /**
     * Constructs the graphics interface.
     *
     * @param[in] width     Display width in pixel
     * @param[in] height    Display height in pixel
     */
    IGfx(uint16_t width, uint16_t height) :
        PixelixGfx(width, height)
    {
    }

    /**
     * Destroys the graphics interface.
     */
    virtual ~IGfx()
    {
    }

    /**
     * Write a single character on the display.
     * This method is necessary for using print() methods.
     *
     * @param[in] singleChar    Single character
     *
     * @return Number of written characters.
     */
    size_t write(uint8_t singleChar) override
    {
        drawChar(static_cast<char>(singleChar));

        return 1U;
    }

private:

    /* Default constructor not allowed. */
    IGfx();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __IGFX_HPP__ */

/** @} */
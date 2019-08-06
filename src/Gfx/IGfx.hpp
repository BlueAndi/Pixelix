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
@brief  Abstract graphics interface
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
This module provides the abstract graphics interface.

*******************************************************************************/
/** @defgroup igfx Abstract graphics interface
 * This module provides the abstract graphics interface, mainly based on the
 * Adafruit GFX.
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
 * Abstract graphics interface.
 */
class IGfx : public Adafruit_GFX
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
        Adafruit_GFX(width, height)
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
        return _width;
    }

    /**
     * Get height in pixel.
     * 
     * @return Height in pixel
     */
    int16_t getHeight(void) const
    {
        return _height;
    }

private:

    IGfx(const IGfx& gfx);
    IGfx& operator=(const IGfx& gfx);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __IGFX_HPP__ */

/** @} */
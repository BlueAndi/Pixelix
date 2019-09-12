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
 * @brief  LED matrix
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup gfx
 *
 * @{
 */

#ifndef __LEDMATRIX_H__
#define __LEDMATRIX_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

/** FastLED RMT driver shall use only one channel to avoid wasting time and memory. */
#define FASTLED_RMT_MAX_CHANNELS    1
#include <FastLED.h>
#include <FastLED_NeoMatrix.h>

#include "Board.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Specific LED matrix.
 */
class LedMatrix : public FastLED_NeoMatrix
{
public:

    /**
     * Get LED matrix instance.
     * 
     * @return LED matrix
     */
    static LedMatrix& getInstance(void)
    {
        return m_instance;
    }

private:

    /** LedMatrix instance */
    static LedMatrix    m_instance;

    /**
     * Construct LED matrix.
     */
    LedMatrix();

    /**
     * Destroys LED matrix.
     */
    ~LedMatrix();

    LedMatrix(const LedMatrix& matrix);
    LedMatrix& operator=(const LedMatrix& matrix);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __LEDMATRIX_H__ */

/** @} */
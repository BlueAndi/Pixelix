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
@brief  Color
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
@see Color.h

*******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Color.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void Color::turnColorWheel(uint8_t wheelPos)
{
    const uint8_t COL_PARTS = 3u;
    const uint8_t COL_RANGE = UINT8_MAX / COL_PARTS;

    wheelPos = UINT8_MAX - wheelPos;

    /* Red + Blue ? */
    if (wheelPos < COL_RANGE) {
        m_red   = UINT8_MAX - wheelPos * COL_PARTS;
        m_green = 0u;
        m_blue  = COL_PARTS * wheelPos;
    }
    /* Green + Blue ? */
    else if (wheelPos < (2 * COL_RANGE)) {
        wheelPos -= COL_RANGE;
        
        m_red   = 0u;
        m_green = COL_PARTS * wheelPos;
        m_blue  = UINT8_MAX - wheelPos * COL_PARTS;
    }
    /* Red + Green */
    else {
        wheelPos -= ((COL_PARTS - 1u) * COL_RANGE);
        
        m_red   = COL_PARTS * wheelPos;
        m_green = UINT8_MAX - wheelPos * COL_PARTS;
        m_blue  = 0u;
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

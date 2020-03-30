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
@brief  Version
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
Provides the software version.

*******************************************************************************/
/** @defgroup version Version
 * Provides the software version.
 *
 * @{
 */

#ifndef __VERSION_H__
#define __VERSION_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

namespace Version
{

/******************************************************************************
 * Macros
 *****************************************************************************/

#ifndef GIT_REV

/**
 * GIT revision number (SHA-1) to identify the software revision.
 */
#define GIT_REV "Unknown"

#endif  /* GIT_REV */

/** Stringizing the value. */
#define Q(x) #x

/** Quote the given value to get a string literal. */
#define QUOTE(x) Q(x)

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

    /** Software revision */
    static const char* SOFTWARE         = QUOTE(GIT_REV);

    /** Software revision short */
    static const char* SOFTWARE_SHORT   = QUOTE(GIT_REV_SHORT);
}

#endif  /* __VERSION_H__ */

/** @} */
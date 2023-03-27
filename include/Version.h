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

#ifndef VERSION_H
#define VERSION_H

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

#ifndef SW_VERSION

/**
 * Software version number.
 */
#define SW_VERSION  "Unknown"

#endif  /* SW_VERSION */

#ifndef SW_REV

/**
 * Software revision number (git SHA-1).
 */
#define SW_REV      "Unknown"

#endif  /* SW_REV */

#ifndef SW_REV_SHORT

/**
 * Software revision number (git SHA-1) in short.
 */
#define SW_REV_SHORT    "Unknown"

#endif  /* SW_REV_SHORT */

#ifndef SW_BRANCH

/**
 * Software branch, the software was built from.
 */
#define SW_BRANCH "Unknown"

#endif  /* SW_BRANCH */

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
    static const char   SOFTWARE_REV[]          = QUOTE(SW_REV);

    /** Software revision short */
    static const char   SOFTWARE_REV_SHORT[]    = QUOTE(SW_REV_SHORT);

    /** Software version */
    static const char   SOFTWARE_VER[]          = QUOTE(SW_VERSION);

    /** Software branch, the software was built from. */
    static const char   SOFTWARE_BRANCH[]       = QUOTE(SW_BRANCH);

    /** The target of this build. */
    static const char   TARGET[]                = QUOTE(PIO_ENV);
}

#endif  /* VERSION_H */

/** @} */
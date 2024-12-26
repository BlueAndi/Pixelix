/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Get software revision.
 * 
 * @return Software revision
 */
const char* getSoftwareRevision();

/**
 * Get short software revision.
 * 
 * @return Short software revision
 */
const char* getSoftwareRevisionShort();

/**
 * Get software version.
 * 
 * @return Software version
 */
const char* getSoftwareVersion();

/**
 * Get software branch name, the software was built from.
 * 
 * @return Software branch name
 */
const char* getSoftwareBranchName();

/**
 * Get target name.
 * 
 * @return Target name
 */
const char* getTargetName();

} /* namespace Version */

#endif /* VERSION_H */

/** @} */
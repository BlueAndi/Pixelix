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
 * @brief  Utility
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Util.h"

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

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

extern bool Util::strToUInt8(const String& str, uint8_t& value)
{
    bool        success = false;
    uint32_t    tmp     = strtoul(str.c_str(), NULL, 0);

    if ((0 == errno) &&
        (0 <= tmp) &&
        (UINT8_MAX >= tmp))
    {
        value = static_cast<uint8_t>(tmp);
        success = true;
    }

    return success;
}

extern bool Util::strToUInt16(const String& str, uint16_t& value)
{
    bool        success = false;
    uint32_t    tmp     = strtoul(str.c_str(), NULL, 0);

    if ((0 == errno) &&
        (0 <= tmp) &&
        (UINT16_MAX >= tmp))
    {
        value = static_cast<uint16_t>(tmp);
        success = true;
    }

    return success;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/
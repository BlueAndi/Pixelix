/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @brief  UTF-8 conversion
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Utf8.h"

#include <stdint.h>

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

void Utf8::toIntern(const String& utf8, String& intern)
{
    size_t utf8Length = utf8.length();
    size_t utf8Index  = 0U;

    while (utf8Length > utf8Index)
    {
        uint16_t ucs2Char   = 0U;
        char     internChar = 0;

        if (0 == (utf8[utf8Index] & 0x80))
        {
            ucs2Char   = utf8[utf8Index];
            utf8Index += 1U;
        }
        else if (0xC0 == (utf8[utf8Index] & 0xE0))
        {
            ucs2Char   = ((utf8[utf8Index] & 0x1F) << 6) | (utf8[utf8Index + 1U] & 0x3F);
            utf8Index += 2U;
        }
        else if (0xE0 == (utf8[utf8Index] & 0xF0))
        {
            ucs2Char   = ((utf8[utf8Index] & 0x0F) << 12) | ((utf8[utf8Index + 1U] & 0x3F) << 6) | (utf8[utf8Index + 2U] & 0x3F);
            utf8Index += 3U;
        }
        else if (0xF0 == (utf8[utf8Index] & 0xF8))
        {
            ucs2Char   = ((utf8[utf8Index] & 0x07) << 18) | ((utf8[utf8Index + 1U] & 0x3F) << 12) | ((utf8[utf8Index + 2U] & 0x03) << 6) | (utf8[utf8Index + 3U] & 0x3F);
            utf8Index += 4U;
        }
        else
        {
            ucs2Char   = 0U;
            utf8Index += 1U;
        }

        /* U+0000 - U+001F */
        if (0x0020U > ucs2Char)
        {
            /* N/A */
        }
        /* U+0020 - U+007E */
        else if (0x007FU > ucs2Char)
        {
            internChar = static_cast<char>(ucs2Char);
        }
        /* U+007F - U+00A0 */
        else if (0x00A1U > ucs2Char)
        {
            /* N/A */
        }
        /* 0x00A1 - 0x00FF ? */
        else if (0x0100U > ucs2Char)
        {
            internChar = static_cast<char>(ucs2Char - 0x22U);
        }
        /* Single character extensions. */
        else
        {
            switch (ucs2Char)
            {
            case 0x011DU:
                internChar = 0xDE;
                break;
            case 0x152U:
                internChar = 0xDF;
                break;

            case 0x153U:
                internChar = 0xE0;
                break;

            case 0x160U:
                internChar = 0xE1;
                break;

            case 0x161U:
                internChar = 0xE2;
                break;

            case 0x178U:
                internChar = 0xE3;
                break;

            case 0x17DU:
                internChar = 0xE4;
                break;

            case 0x17EU:
                internChar = 0xE5;
                break;

            case 0x0EA4U:
                internChar = 0xE6;
                break;

            case 0x13A0U:
                internChar = 0xE7;
                break;

            case 0x2022U:
                internChar = 0xE8;
                break;

            case 0x2026U:
                internChar = 0xE9;
                break;

            case 0x20ACU:
                internChar = 0xEA;
                break;

            case 0xFFFDU:
                internChar = 0xEB;
                break;

            default:
                break;
            }
        }

        if (0 != internChar)
        {
            intern += internChar;
        }
    }
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

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
 * @brief  Utility
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Util.h"
#include <stdio.h>
#include <cctype>

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

extern bool Util::strToUInt8(const char* str, uint8_t& value)
{
    bool success = false;

    if (nullptr != str)
    {
        char*           endPtr  = nullptr;
        unsigned long   tmp     = 0UL;

        errno = 0;
        tmp = strtoul(str, &endPtr, 0);

        if ((0 == errno) &&
            (nullptr != endPtr) &&
            ('\0' == *endPtr) &&
            (str != endPtr) &&
            (UINT8_MAX >= tmp))
        {
            value = static_cast<uint8_t>(tmp);
            success = true;
        }
    }

    return success;
}

extern bool Util::strToUInt8(const String& str, uint8_t& value)
{
    return strToUInt8(str.c_str(), value);
}

extern bool Util::strToUInt16(const char* str, uint16_t& value)
{
    bool success = false;

    if (nullptr != str)
    {
        char*           endPtr  = nullptr;
        unsigned long   tmp     = 0UL;

        errno = 0;
        tmp = strtoul(str, &endPtr, 0);

        if ((0 == errno) &&
            (nullptr != endPtr) &&
            ('\0' == *endPtr) &&
            (str != endPtr) &&
            (UINT16_MAX >= tmp))
        {
            value = static_cast<uint16_t>(tmp);
            success = true;
        }
    }

    return success;
}

extern bool Util::strToUInt16(const String& str, uint16_t& value)
{
    return strToUInt16(str.c_str(), value);
}

extern bool Util::strToInt32(const char* str, int32_t& value)
{
    bool success = false;

    if (nullptr != str)
    {
        char*   endPtr  = nullptr;
        long    tmp     = 0L;

        errno = 0;
        tmp = strtol(str, &endPtr, 0);

        if ((0 == errno) &&
            (nullptr != endPtr) &&
            ('\0' == *endPtr) &&
            (str != endPtr) &&
            (INT32_MAX >= tmp))
        {
            value = static_cast<int32_t>(tmp);
            success = true;
        }
    }

    return success;
}

extern bool Util::strToInt32(const String& str, int32_t& value)
{
    return strToInt32(str.c_str(), value);
}

extern bool Util::strToUInt32(const char* str, uint32_t& value)
{
    bool success = false;

    if (nullptr != str)
    {
        char*           endPtr  = nullptr;
        unsigned long   tmp     = 0UL;

        errno = 0;
        tmp = strtoul(str, &endPtr, 0);

        if ((0 == errno) &&
            (nullptr != endPtr) &&
            ('\0' == *endPtr) &&
            (str != endPtr) &&
            (UINT32_MAX >= tmp))
        {
            value = static_cast<uint32_t>(tmp);
            success = true;
        }
    }

    return success;
}

extern bool Util::strToUInt32(const String& str, uint32_t& value)
{
    return strToUInt32(str.c_str(), value);
}

extern String Util::uint32ToHex(uint32_t value)
{
    char buffer[9];  /* Contains a 32-bit value in hex */

    (void)snprintf(buffer, UTIL_ARRAY_NUM(buffer), "%x", value);

    return String(buffer);
}

extern uint32_t Util::hexToUInt32(const String& str)
{
    uint32_t    value   = 0U;
    uint32_t    idx     = 0U;
    bool        isError = false;

    if ((true == str.startsWith("0x")) ||
        (true == str.startsWith("0X")))
    {
        idx = 2U;
    }

    while((str.length() > idx) && (false == isError))
    {
        value *= 16U;

        if (('0' <= str[idx]) &&
            ('9' >= str[idx]))
        {
            value += static_cast<uint32_t>(str[idx] - '0');
        }
        else if (('a' <= str[idx]) &&
                 ('f' >= str[idx]))
        {
            value += static_cast<uint32_t>(str[idx] - 'a') + 10U;
        }
        else if (('A' <= str[idx]) &&
                 ('F' >= str[idx]))
        {
            value += static_cast<uint32_t>(str[idx] - 'A') + 10U;
        }
        else
        {
            value = 0U;
            isError = true;
        }

        ++idx;
    }

    return value;
}

String Util::colorToHtml(const Color& color)
{
    char buffer[8]; /* '#' + 3x byte in hex + '\0' */

    (void)snprintf(buffer, sizeof(buffer), "#%02X%02X%02X", color.getRed(), color.getGreen(), color.getBlue());

    return String(buffer);
}

Color Util::colorFromHtml(const String& htmlColor)
{
    Color color;

    if ('#' == htmlColor[0])
    {
        color = Util::hexToUInt32(htmlColor.substring(1U));
    }

    return color;
}

bool Util::isFormatSpecifierInStr(const String& str, char specifier)
{
    bool         isFound    = false;
    int32_t      index      = 0U;
    const size_t STR_LENGTH = str.length();

    while (STR_LENGTH > index)
    {
        index = str.indexOf('%', index);

        if (0 > index)
        {
            break;
        }

        if (STR_LENGTH > (index + 1))
        {
            int32_t specifierIndex = index + 1;

            /* Skip flags, width, and precision */
            while ((STR_LENGTH > specifierIndex) &&
                   ((str[specifierIndex] == '-') || (str[specifierIndex] == '+') || (str[specifierIndex] == ' ') ||
                       (str[specifierIndex] == '#') || (str[specifierIndex] == '0') || (0 < std::isdigit(str[specifierIndex])) ||
                       (str[specifierIndex] == '.')))
            {
                ++specifierIndex;
            }

            if ((STR_LENGTH > specifierIndex) && (specifier == str[specifierIndex]))
            {
                isFound = true;
                break;
            }
        }

        ++index;
    }

    return isFound;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/
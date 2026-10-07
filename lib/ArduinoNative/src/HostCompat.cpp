/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   HostCompat.cpp
 * @brief  C library functions which are missing on some hosts
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "HostCompat.h"

#ifdef _WIN32

#include <ctype.h>
#include <stddef.h>

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

static const char* parseNumber(const char* str, int digits, int min, int max, int* value);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Year, which the tm_year member is relative to. */
static const int TM_YEAR_EPOCH = 1900;

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

extern "C" char* strptime(const char* str, const char* format, struct tm* timeInfo)
{
    const char* current   = str;
    const char* formatPtr = format;
    bool        isAborted = false;

    if ((nullptr == str) ||
        (nullptr == format) ||
        (nullptr == timeInfo))
    {
        return nullptr;
    }

    while (('\0' != *formatPtr) &&
           (false == isAborted))
    {
        if ('%' == *formatPtr)
        {
            int value = 0;

            ++formatPtr;

            switch (*formatPtr)
            {
            case 'Y':
                current = parseNumber(current, 4, 0, 9999, &value);

                if (nullptr != current)
                {
                    timeInfo->tm_year = value - TM_YEAR_EPOCH;
                }
                break;

            case 'm':
                current = parseNumber(current, 2, 1, 12, &value);

                if (nullptr != current)
                {
                    timeInfo->tm_mon = value - 1;
                }
                break;

            case 'd':
                current = parseNumber(current, 2, 1, 31, &value);

                if (nullptr != current)
                {
                    timeInfo->tm_mday = value;
                }
                break;

            case 'H':
                current = parseNumber(current, 2, 0, 23, &value);

                if (nullptr != current)
                {
                    timeInfo->tm_hour = value;
                }
                break;

            case 'M':
                current = parseNumber(current, 2, 0, 59, &value);

                if (nullptr != current)
                {
                    timeInfo->tm_min = value;
                }
                break;

            case 'S':
                /* A leap second is accepted, like the POSIX counterpart does. */
                current = parseNumber(current, 2, 0, 60, &value);

                if (nullptr != current)
                {
                    timeInfo->tm_sec = value;
                }
                break;

            case 'y':
                current = parseNumber(current, 2, 0, 99, &value);

                if (nullptr != current)
                {
                    /* Like the POSIX counterpart, 69 - 99 is the 20th century. */
                    timeInfo->tm_year = (69 <= value) ? value : (value + 100);
                }
                break;

            case '%':
                if ('%' == *current)
                {
                    ++current;
                }
                else
                {
                    current = nullptr;
                }
                break;

            default:
                /* Unsupported conversion specifier. */
                current = nullptr;
                break;
            }

            if (nullptr == current)
            {
                isAborted = true;
            }
            else
            {
                ++formatPtr;
            }
        }
        else if (0 != isspace(static_cast<unsigned char>(*formatPtr)))
        {
            /* Any amount of whitespace matches, including none. */
            while (0 != isspace(static_cast<unsigned char>(*current)))
            {
                ++current;
            }

            ++formatPtr;
        }
        else if (*formatPtr == *current)
        {
            ++formatPtr;
            ++current;
        }
        else
        {
            isAborted = true;
        }
    }

    return (true == isAborted) ? nullptr : const_cast<char*>(current);
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Parse a number with the given number of digits.
 *
 * Leading whitespace is skipped and less digits than requested are accepted,
 * like the POSIX strptime() does it.
 *
 * @param[in]  str      String, which to parse.
 * @param[in]  digits   Max. number of digits.
 * @param[in]  min      Min. valid value.
 * @param[in]  max      Max. valid value.
 * @param[out] value    Parsed number.
 *
 * @return Pointer to the first character after the number or nullptr in case
 *         there is no valid number.
 */
static const char* parseNumber(const char* str, int digits, int min, int max, int* value)
{
    const char* current = str;
    int         result  = 0;
    int         count   = 0;

    while (0 != isspace(static_cast<unsigned char>(*current)))
    {
        ++current;
    }

    while ((count < digits) &&
           (0 != isdigit(static_cast<unsigned char>(*current))))
    {
        result = (result * 10) + (*current - '0');

        ++current;
        ++count;
    }

    if ((0 == count) ||
        (min > result) ||
        (max < result))
    {
        return nullptr;
    }

    *value = result;

    return current;
}

#endif /* _WIN32 */

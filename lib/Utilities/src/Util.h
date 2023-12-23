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
 * @brief  Utilitiy
 * @author Yann Le Glaz <yann_le@web.de>
 *
 * @addtogroup utilities
 *
 * @{
 */

#ifndef UTILITY_H
#define UTILITY_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <WString.h>
#include <cerrno>

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Get number of array elements. */
#define UTIL_ARRAY_NUM(__arr)   (sizeof(__arr) / sizeof((__arr)[0]))

/** Use it to mark not used function parameters. */
#define UTIL_NOT_USED(__var)    (void)(__var)

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Utilitiy functions */
namespace Util
{

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Convert a string to uint8_t. String can contain integer number in decimal
 * or hexadecimal format.
 *
 * @param[in]   str     String
 * @param[out]  value   Converted value
 *
 * @return If conversion fails, it will return false otherwise true.
 */
extern bool strToUInt8(const String& str, uint8_t& value);

/**
 * Convert a string to uint16_t. String can contain integer number in decimal
 * or hexadecimal format.
 *
 * @param[in]   str     String
 * @param[out]  value   Converted value
 *
 * @return If conversion fails, it will return false otherwise true.
 */
extern bool strToUInt16(const String& str, uint16_t& value);

/**
 * Convert a string to uint32_t. String can contain integer number in decimal
 * or hexadecimal format.
 *
 * Note, negative values in the string will lead to a successful conversion.
 * This is a limitation due to fact that the underlying strtoul() cast the
 * result to unsigned long, which is on the esp32 equal to uint32_t.
 *
 * @param[in]   str     String
 * @param[out]  value   Converted value
 *
 * @return If conversion fails, it will return false otherwise true.
 */
extern bool strToUInt32(const String& str, uint32_t& value);

/**
 * Convert a string to int32_t. String can contain integer number in decimal
 * or hexadecimal format.
 *
 * @param[in]   str     String
 * @param[out]  value   Converted value
 *
 * @return If conversion fails, it will return false otherwise true.
 */
extern bool strToInt32(const String& str, int32_t& value);

/**
 * Convert uint32_t to hex string, but without "0x" as prefix.
 *
 * @param[in] value Value to convert
 *
 * @return Hex string
 */
extern String uint32ToHex(uint32_t value);

/**
 * Convert hex string to uint32_t. String may has the prefix "0x" or not.
 * If conversion fails, it will return 0.
 *
 * @param[in] str   String which contains a hex number
 *
 * @return 32 bit unsigned integer value
 */
extern uint32_t hexToUInt32(const String& str);

}

#endif  /* UTILITY_H */

/** @} */
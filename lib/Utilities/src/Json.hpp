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
 * @file   Json.hpp
 * @brief  Common JSON functions and constants
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup UTILITIES
 *
 * @{
 */

#ifndef JSON_HPP
#define JSON_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stddef.h>

/* JSON namespace */
namespace Json
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
 * Get the capacity for a JSON document, adapted to the platform's pointer width.
 *
 * The memory pool of a JSON document stores its values in slots, which contain
 * pointers. On a 64-bit platform (e.g. native simulation and tests) a slot is
 * therefore larger than on a 32-bit platform (e.g. ESP32). All capacities in
 * the code base are sized for the 32-bit target, so they are doubled on a
 * 64-bit platform. Doubling is a conservative estimate, because string copies
 * in the pool have the same size on both platforms.
 *
 * Use it only for fixed capacities in byte. Capacities calculated with
 * JSON_OBJECT_SIZE() or JSON_ARRAY_SIZE() already consider the slot size.
 *
 * @param[in] dataSize  The capacity in byte, which is required on a 32-bit platform.
 *
 * @return The capacity in byte, which is required on the current platform.
 */
inline constexpr size_t getJsonSize(size_t dataSize)
{
    return (8U == sizeof(void*)) ? (dataSize * 2U) : dataSize;
}

} /* namespace Json */

#endif /* JSON_HPP */

/** @} */

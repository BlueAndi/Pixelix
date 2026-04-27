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
 * @file   ColorUtil.hpp
 * @brief  Color utility functions
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup GFX
 *
 * @{
 */

#ifndef COLOR_UTIL_HPP
#define COLOR_UTIL_HPP

/*******************************************************************************
    INCLUDES
*******************************************************************************/
#include <stdint.h>

/*******************************************************************************
    COMPILER SWITCHES
*******************************************************************************/

/*******************************************************************************
    CONSTANTS
*******************************************************************************/

/*******************************************************************************
    MACROS
*******************************************************************************/

/*******************************************************************************
    CLASSES, TYPES AND STRUCTURES
*******************************************************************************/

/*******************************************************************************
    VARIABLES
*******************************************************************************/

/*******************************************************************************
    FUNCTIONS
*******************************************************************************/

/**
 * Color utility functions.
 */
namespace ColorUtil
{

/**
 * Get red component from RGB565 color.
 *
 * @param[in] value RGB565 color value
 *
 * @return uint8_t Red component [0; 255]
 */
static inline uint8_t rgb565Red(uint16_t value)
{
    return ((value >> 11U) & 0x1fU) << 3U;
}

/**
 * Get green component from RGB565 color.
 *
 * @param[in] value RGB565 color value
 *
 * @return uint8_t Green component [0; 255]
 */
static inline uint8_t rgb565Green(uint16_t value)
{
    return ((value >> 5U) & 0x3fU) << 2U;
}

/**
 * Get blue component from RGB565 color.
 *
 * @param[in] value RGB565 color value
 *
 * @return uint8_t Blue component [0; 255]
 */
static inline uint8_t rgb565Blue(uint16_t value)
{
    return ((value >> 0U) & 0x1fU) << 3U;
}

/**
 * Get red component from RGB888 color.
 *
 * @param[in] value RGB888 color value
 *
 * @return uint8_t Red component [0; 255]
 */
static inline uint8_t rgb888Red(uint32_t value)
{
    return (value >> 16U) & 0xffU;
}

/**
 * Get green component from RGB888 color.
 *
 * @param[in] value RGB888 color value
 *
 * @return uint8_t Green component [0; 255]
 */
static inline uint8_t rgb888Green(uint32_t value)
{
    return (value >> 8U) & 0xffU;
}

/**
 * Get blue component from RGB888 color.
 *
 * @param[in] value RGB888 color value
 *
 * @return uint8_t Blue component [0; 255]
 */
static inline uint8_t rgb888Blue(uint32_t value)
{
    return (value >> 0U) & 0xffU;
}

/**
 * Convert base colors to RGB565 color.
 *
 * @param[in] red   Red component [0; 255]
 * @param[in] green Green component [0; 255]
 * @param[in] blue  Blue component [0; 255]
 *
 * @return uint16_t RGB565 color value
 */
static inline uint16_t to565(uint8_t red, uint8_t green, uint8_t blue)
{
    const uint16_t RED5   = red >> 3U;   /* 5 bit */
    const uint16_t GREEN6 = green >> 2U; /* 6 bit */
    const uint16_t BLUE5  = blue >> 3U;  /* 5 bit */

    return ((RED5 & 0x001fU) << 11U) | ((GREEN6 & 0x003fU) << 5U) | ((BLUE5 & 0x001fU) << 0U);
}

/**
 * Convert base colors to RGB888 color.
 *
 * @param[in] red   Red component [0; 255]
 * @param[in] green Green component [0; 255]
 * @param[in] blue  Blue component [0; 255]
 *
 * @return uint32_t RGB888 color value
 */
static inline uint32_t to888(uint8_t red, uint8_t green, uint8_t blue)
{
    const uint32_t RED8   = red;   /* 8 bit */
    const uint32_t GREEN8 = green; /* 8 bit */
    const uint32_t BLUE8  = blue;  /* 8 bit */

    return (RED8 << 16U) | (GREEN8 << 8U) | (BLUE8 << 0U);
}

/**
 * Convert RGB888 color to RGB565 color.
 *
 * @param[in] rgb888 RGB888 color value
 *
 * @return uint16_t RGB565 color value
 */
static inline uint16_t to565(uint32_t rgb888)
{
    return to565(rgb888Red(rgb888), rgb888Green(rgb888), rgb888Blue(rgb888));
}

/**
 * Convert RGB565 color to RGB888 color.
 *
 * @param[in] rgb565 RGB565 color value
 *
 * @return RGB888 color value
 */
static inline uint32_t to888(uint16_t rgb565)
{
    const uint8_t RED8   = rgb565Red(rgb565);
    const uint8_t GREEN8 = rgb565Green(rgb565);
    const uint8_t BLUE8  = rgb565Blue(rgb565);

    return to888(RED8, GREEN8, BLUE8);
}

}; /* namespace ColorUtil */

#endif /* COLOR_UTIL_HPP */

/** @} */
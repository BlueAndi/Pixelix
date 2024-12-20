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
 * @brief  Layouts
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup plugin
 *
 * @{
 */

#ifndef LAYOUTS_H
#define LAYOUTS_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

#if (32U == CONFIG_LED_MATRIX_WIDTH) && (8U == CONFIG_LED_MATRIX_HEIGHT)

/** Layout type derived by LED matrix width and height. */
#define LAYOUT_TYPE LAYOUT_32X8

#elif (32U == CONFIG_LED_MATRIX_WIDTH) && (16U == CONFIG_LED_MATRIX_HEIGHT)

/** Layout type derived by LED matrix width and height. */
#define LAYOUT_TYPE LAYOUT_32X16

#elif (64U == CONFIG_LED_MATRIX_WIDTH) && (64U == CONFIG_LED_MATRIX_HEIGHT)

/** Layout type derived by LED matrix width and height. */
#define LAYOUT_TYPE LAYOUT_64X64

#else

/** Layout type derived by LED matrix width and height. */
#define LAYOUT_TYPE LAYOUT_GENERIC

#endif

/******************************************************************************
 * Includes
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Supported layouts.
 */
typedef enum
{
    LAYOUT_GENERIC = 0, /**< Generic layout, used in case there is no specific one. */
    LAYOUT_32X8,        /**< Layout for 32x8 pixel displays. */
    LAYOUT_32X16,       /**< Layout for 32x16 pixel displays. */
    LAYOUT_64X64        /**< Layout for 64x64 pixel displays. */

} Layout;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* LAYOUTS_H */

/** @} */

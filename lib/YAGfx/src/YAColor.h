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
 * @file   YAColor.h
 * @brief  Yet anoterh color class
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup GFX
 *
 * @{
 */

#ifndef YACOLOR_H
#define YACOLOR_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

#ifndef CONFIG_COLOR_DEPTH
/** Color depth default configuration. */
#define CONFIG_COLOR_DEPTH 32
#endif

/******************************************************************************
 * Includes
 *****************************************************************************/

#if (CONFIG_COLOR_DEPTH == 32)
#include <Rgb888.h>
#elif (CONFIG_COLOR_DEPTH == 16)
#include <Rgb565.h>
#endif /* CONFIG_COLOR_DEPTH */

#include <ColorDef.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

#if (CONFIG_COLOR_DEPTH == 32)

/**
 * Defines the general color to RGB888 format.
 */
typedef Rgb888 Color;

#elif (CONFIG_COLOR_DEPTH == 16)

/**
 * Defines the general color to RGB888 format.
 */
typedef Rgb565 Color;

#endif /* CONFIG_COLOR_DEPTH */

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* YACOLOR_H */

/** @} */
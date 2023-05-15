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
 * @brief  Yet anoterh GFX bitmap class
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef YAGFX_BITMAP_H
#define YAGFX_BITMAP_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <BaseGfxBitmap.hpp>
#include <YAColor.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** GFX bitmap interface with concrete color. */
using YAGfxBitmap = BaseGfxBitmap<Color>;

/** GFX static bitmap with concrete color.
 * 
 * @tparam width    Bitmap width in pixels.
 * @tparam height   Bitmap height in pixels.
 */
template < uint16_t width, uint16_t height >
using YAGfxStaticBitmap = BaseGfxStaticBitmap<Color, width, height>;

/** GFX dynamic bitmap with concrete color. */
using YAGfxDynamicBitmap = BaseGfxDynamicBitmap<Color>;

/** GFX overlay bitmap with concrete color. */
using YAGfxOverlayBitmap = BaseGfxOverlayBitmap<Color>;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* YAGFX_BITMAP_H */

/** @} */
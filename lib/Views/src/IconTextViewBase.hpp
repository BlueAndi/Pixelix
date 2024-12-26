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
 * @brief  Base class for view with icon and text.
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup plugin
 *
 * @{
 */

#ifndef ICON_TEXT_VIEW_BASE_HPP
#define ICON_TEXT_VIEW_BASE_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Layouts.h"
#include "./layouts/IconTextViewGeneric.h"
#include "./layouts/IconTextView32x8.h"
#include "./layouts/IconTextView32x16.h"
#include "./layouts/IconTextView64x64.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * View for icon and text.
 * 
 * @tparam option   Layout which to choose
 */
template< Layout option >
class IconTextView : public IconTextViewGeneric
{
public:
    /**
     * Destroys the view.
     */
    virtual ~IconTextView() = default;
};

/**
 * View for icon and text for 32x8 display.
 */
template<>
class IconTextView<LAYOUT_32X8> : public IconTextView32x8
{
public:
    /**
     * Destroys the view.
     */
    virtual ~IconTextView() = default;
};

/**
 * View for canvas and text for 32x16 display.
 */
template<>
class IconTextView<LAYOUT_32X16> : public IconTextView32x16
{
public:
    /**
     * Destroys the view.
     */
    virtual ~IconTextView() = default;
};

/**
 * View for icon and text for 64x64 display.
 */
template<>
class IconTextView<LAYOUT_64X64> : public IconTextView64x64
{
public:
    /**
     * Destroys the view.
     */
    virtual ~IconTextView() = default;
};

/** View for icon and text, considering the display size. */
using IconTextViewBase = IconTextView<LAYOUT_TYPE>;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* ICON_TEXT_VIEW_BASE_HPP */

/** @} */

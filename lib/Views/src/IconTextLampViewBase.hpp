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
 * @brief  Base class for view with icon, text and lamps.
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup plugin
 *
 * @{
 */

#ifndef ICON_TEXT_LAMP_VIEW_BASE_HPP
#define ICON_TEXT_LAMP_VIEW_BASE_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Layouts.h"
#include "./layouts/IconTextLampViewGeneric.h"
#include "./layouts/IconTextLampView32x8.h"
#include "./layouts/IconTextLampView32x16.h"
#include "./layouts/IconTextLampView64x64.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * View for icon, text and lamps.
 * 
 * @tparam option   Layout which to choose
 */
template< Layout option >
class IconTextLampView : public IconTextLampViewGeneric
{
public:
    /**
     * Destroys the view.
     */
    virtual ~IconTextLampView() = default;
};

/**
 * View for icon, text and lamps for 32x8 display.
 */
template<>
class IconTextLampView<LAYOUT_32X8> : public IconTextLampView32x8
{
public:
    /**
     * Destroys the view.
     */
    virtual ~IconTextLampView() = default;
};

/**
 * View for icon, text and lamps for 32x16 display.
 */
template<>
class IconTextLampView<LAYOUT_32X16> : public IconTextLampView32x16
{
public:
    /**
     * Destroys the view.
     */
    virtual ~IconTextLampView() = default;
};

/**
 * View for icon, text and lamps for 64x64 display.
 */
template<>
class IconTextLampView<LAYOUT_64X64> : public IconTextLampView64x64
{
public:
    /**
     * Destroys the view.
     */
    virtual ~IconTextLampView() = default;
};

/** View for icon, text and lamps, considering the display size. */
using IconTextLampViewBase = IconTextLampView<LAYOUT_TYPE>;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* ICON_TEXT_LAMP_VIEW_BASE_HPP */

/** @} */

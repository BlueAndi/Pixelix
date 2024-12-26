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
 * @brief  Base class for view with icon, text and progress bar.
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup plugin
 *
 * @{
 */

#ifndef PLAYER_VIEW_BASE_HPP
#define PLAYER_VIEW_BASE_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Layouts.h"
#include "./layouts/PlayerViewGeneric.h"
#include "./layouts/PlayerView32x8.h"
#include "./layouts/PlayerView32x16.h"
#include "./layouts/PlayerView64x64.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * View for icon, text and progress bar.
 * 
 * @tparam option   Layout which to choose
 */
template< Layout option >
class PlayerView : public PlayerViewGeneric
{
public:
    /**
     * Destroys the view.
     */
    virtual ~PlayerView() = default;
};

/**
 * View for icon, text and progress bar for 32x8 display.
 */
template<>
class PlayerView<LAYOUT_32X8> : public PlayerView32x8
{
public:
    /**
     * Destroys the view.
     */
    virtual ~PlayerView() = default;
};

/**
 * View for icon, text and progress bar for 32x16 display.
 */
template<>
class PlayerView<LAYOUT_32X16> : public PlayerView32x16
{
public:
    /**
     * Destroys the view.
     */
    virtual ~PlayerView() = default;
};

/**
 * View for icon, text and progress bar for 64x64 display.
 */
template<>
class PlayerView<LAYOUT_64X64> : public PlayerView64x64
{
public:
    /**
     * Destroys the view.
     */
    virtual ~PlayerView() = default;
};

/** View for icon, text and progress bar, considering the display size. */
using PlayerViewBase = PlayerView<LAYOUT_TYPE>;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* PLAYER_VIEW_BASE_HPP */

/** @} */

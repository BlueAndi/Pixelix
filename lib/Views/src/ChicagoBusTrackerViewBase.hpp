/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
 * Copyright (c) 2026        Julie Hill <queenkjuul@pm.me>
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
 * @file   ChicagoBusTrackerViewBase.hpp
 * @brief  Base class for view for Chicago Bus Tracker Plugin.
 * @author Julie Hill <queenkjuul@pm.me
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef CHICAGO_BUS_TRACKER_VIEW_BASE_HPP
#define CHICAGO_BUS_TRACKER_VIEW_BASE_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Layouts.h"
#include "./layout/ChicagoBusTrackerViewGeneric.h"
#include "./layout/ChicagoBusTrackerView32x8.h"
// #include "./layout/ChicagoBusTrackerView32x16.h"
// #include "./layout/ChicagoBusTrackerView64x64.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * View for Chicago Bus Tracker.
 *
 * @tparam option   Layout which to choose
 */
template < Layout option >
class ChicagoBusTrackerView : public ChicagoBusTrackerViewGeneric
{
    public:

    /**
     * Destroys the view.
     */
    virtual ~ChicagoBusTrackerView() = default;
};

/**
 * View for Chicago Bus Tracker for 32x8 display.
 */
template <>
class ChicagoBusTrackerView<LAYOUT_32X8> : public ChicagoBusTrackerView32x8
{
public:

    /**
     * Destroys the view.
     */
    virtual ~ChicagoBusTrackerView() = default;
};

/**
 * View for Chicago Bus Tracker for 32x16 display.
 */
// template <>
// class ChicagoBusTrackerView<LAYOUT_32X16> : public ChicagoBusTrackerView32x16
// {
// public:

//     /**
//      * Destroys the view.
//      */
//     virtual ~ChicagoBusTrackerView() = default;
// };

/**
 * View for Chicago Bus Tracker for 64x64 display.
 */
// template <>
// class ChicagoBusTrackerView<LAYOUT_64X64> : public ChicagoBusTrackerView64x64
// {
// public:

//     /**
//      * Destroys the view.
//      */
//     virtual ~ChicagoBusTrackerView() = default;
// };

/** View for Chicago Bus Tracker, considering the display size. */
using ChicagoBusTrackerViewBase = ChicagoBusTrackerView<LAYOUT_TYPE>;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* CHICAGO_BUS_TRACKER_VIEW_BASE_HPP */

/** @} */

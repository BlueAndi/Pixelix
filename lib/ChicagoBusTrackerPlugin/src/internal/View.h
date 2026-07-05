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
 * @file   View.h
 * @brief  ChicagoBusTrackerPlugin view
 * @author Julie Hill <queenkjuul@pm.me>
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef CHICAGO_BUS_TRACKER_PLUGIN_VIEW_H
#define CHICAGO_BUS_TRACKER_PLUGIN_VIEW_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <ChicagoBusTrackerViewBase.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Internal plugin functionality. */
namespace _ChicagoBusTrackerPlugin
{

/**
 * ChicagoBusTrackerPlugin view.
 */
class View : public ChicagoBusTrackerViewBase
{
public:

    /**
     * Construct the view.
     */
    View() :
        ChicagoBusTrackerViewBase()
    {
    }

    /**
     * Destroy the view.
     */
    ~View()
    {
    }

private:

    View(const View& other);
    View& operator=(const View& other);
};

} // namespace _ChicagoBusTrackerPlugin

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* CHICAGO_BUS_TRACKER_PLUGIN_VIEW_H */

/** @} */

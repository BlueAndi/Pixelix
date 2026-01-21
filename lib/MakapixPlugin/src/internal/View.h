/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @brief  MakapixPlugin view
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef MAKAPIX_PLUGIN_VIEW_H
#define MAKAPIX_PLUGIN_VIEW_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <CanvasViewBase.hpp>
#include <SimpleTimer.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Internal plugin functionality. */
namespace _MakapixPlugin
{

/**
 * MakapixPlugin view.
 */
class View : public CanvasViewBase
{
public:

    /**
     * Construct the view.
     */
    View() :
        CanvasViewBase(),
        m_actionTimer()
    {
    }

    /**
     * Destroy the view.
     */
    ~View()
    {
    }

    /**
     * Process view.
     * Should be called cyclically.
     */
    void process();

    /**
     * Show "next" action icon on screen.
     */
    void showActionIconNext();

    /**
     * Show "previous" action icon on screen.
     */
    void showActionIconPrev();

    /**
     * Show "play" action icon on screen.
     */
    void showActionIconPlay();

    /**
     * Show "fail" action icon on screen.
     */
    void showActionIconFail();

    /**
     * Clear any action icon on screen.
     */
    void clearActionIcon();

private:

    /** Action icon size in pixels.*/
    static const uint16_t ACTION_ICON_SIZE         = (64U <= CONFIG_LED_MATRIX_HEIGHT) ? ((16U / 2U) - 1U) : ((8U / 2U) - 1U); /* odd number */

    /** Action icone x-position in pixels. */
    static const int16_t ACTION_ICON_POS_X         = 1;

    /** Action icone y-position in pixels. */
    static const int16_t ACTION_ICON_POS_Y         = 1;

    /**
     * Action icon display time in milliseconds.
     */
    static const uint32_t ACTION_ICON_DISPLAY_TIME = SIMPLE_TIMER_SECONDS(3U);

    SimpleTimer           m_actionTimer; /**< Timer to show action icon for a limited time. */

    View(const View& other);
    View& operator=(const View& other);
};

} // namespace _MakapixPlugin

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* MAKAPIX_PLUGIN_VIEW_H */

/** @} */

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
 * @file   ScrollContainer.h
 * @brief  Scrollable widget container
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup GFX
 *
 * @{
 */

#ifndef SCROLLCONTAINER_H
#define SCROLLCONTAINER_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

#include "ScrollController.h"
#include "Widget.hpp"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Container which scrolls all child widgets by the same offset.
 */
class ScrollContainer : public Widget
{
public:

    /** Maximum number of child widgets. */
    static const uint8_t MAX_CHILDREN = 4U;

    /**
     * Constructs a scroll container.
     *
     * @param[in] width  Container width in pixels.
     * @param[in] height Container height in pixels.
     * @param[in] x      Container x-coordinate.
     * @param[in] y      Container y-coordinate.
     */
    ScrollContainer(uint16_t width = 0U, uint16_t height = 0U, int16_t x = 0, int16_t y = 0);

    /**
     * Adds a child widget without taking ownership.
     *
     * @param[in] widget Child widget.
     * @return True if the child was added, otherwise false.
     */
    bool add(Widget& widget);

    /**
     * Removes all child widgets. Required to rebuild the scrolling content,
     * e.g. if the layout changed.
     */
    void clear();

    /**
     * Sets the content size used to determine the scroll requirement.
     *
     * @param[in] contentSize Content width or height in pixels.
     */
    void setContentSize(uint16_t contentSize);

    /**
     * Enables scrolling in the selected direction.
     *
     * @param[in] direction Scroll direction.
     */
    void enableScrolling(ScrollController::Direction direction);

    /** Disables scrolling and resets the content position. */
    void disableScrolling();

    /**
     * Gets whether scrolling is enabled.
     *
     * @return True if scrolling is enabled, otherwise false.
     */
    bool isScrollingEnabled() const;

    /**
     * Gets the number of completed scroll cycles.
     *
     * @return Completed scroll cycles.
     */
    uint32_t getScrollingCount() const;

    /**
     * Gets scroll information.
     *
     * @param[out] isScrollingEnabled True if scrolling is enabled.
     * @param[out] scrollingCnt Number of completed scroll cycles.
     * @return True when the information is available.
     */
    bool getScrollInfo(bool& isScrollingEnabled, uint32_t& scrollingCnt) const;

    /**
     * Changes the global scroll pause.
     *
     * @param[in] pause Scroll pause in milliseconds.
     * @return True if the pause is valid and was set.
     */
    static bool setScrollPause(uint32_t pause);

    /** Widget type string. */
    static const char* WIDGET_TYPE;

private:

    Widget*          m_children[MAX_CHILDREN]; /**< Child widgets, not owned. */
    uint8_t          m_childCount;             /**< Number of child widgets. */
    uint16_t         m_contentSize;            /**< Width or height of content. */
    uint16_t         m_canvasSize;             /**< Visible width or height, used to enable scrolling. */
    ScrollController m_scrollController;       /**< Shared scroll state. */

    /** Paints all child widgets through the scrolled canvas. */
    void paint(YAGfx& gfx) override;

    ScrollContainer(const ScrollContainer& other);
    ScrollContainer& operator=(const ScrollContainer& other);
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* SCROLLCONTAINER_H */

/** @} */

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
 * @file   ScrollContainer.cpp
 * @brief  Scrollable widget container
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ScrollContainer.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

const char* ScrollContainer::WIDGET_TYPE = "scrollContainer";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

ScrollContainer::ScrollContainer(uint16_t width, uint16_t height, int16_t x, int16_t y) :
    Widget(WIDGET_TYPE, width, height, x, y),
    m_children{ nullptr, nullptr, nullptr, nullptr },
    m_childCount(0U),
    m_contentSize(0U),
    m_scrollController()
{
}

bool ScrollContainer::add(Widget& widget)
{
    bool isAdded = false;

    if (MAX_CHILDREN > m_childCount)
    {
        m_children[m_childCount] = &widget;
        ++m_childCount;
        isAdded = true;
    }

    return isAdded;
}

void ScrollContainer::setContentSize(uint16_t contentSize)
{
    m_contentSize = contentSize;
}

void ScrollContainer::enableScrolling(ScrollController::Direction direction)
{
    const uint16_t canvasSize = (ScrollController::DIRECTION_HORIZONTAL == direction) ? getWidth() : getHeight();

    if (canvasSize < m_contentSize)
    {
        if ((false == m_scrollController.isEnabled()) ||
            (direction != m_scrollController.getDirection()) ||
            (m_contentSize != m_scrollController.getContentSize()))
        {
            m_scrollController.enable(direction, canvasSize, m_contentSize);
        }
    }
    else
    {
        m_scrollController.disable();
    }
}

void ScrollContainer::disableScrolling()
{
    m_scrollController.disable();
}

bool ScrollContainer::isScrollingEnabled() const
{
    return m_scrollController.isEnabled();
}

uint32_t ScrollContainer::getScrollingCount() const
{
    return m_scrollController.getScrollingCount();
}

bool ScrollContainer::getScrollInfo(bool& isScrollingEnabled, uint32_t& scrollingCnt) const
{
    isScrollingEnabled = m_scrollController.isEnabled();
    scrollingCnt       = m_scrollController.getScrollingCount();

    return true;
}

bool ScrollContainer::setScrollPause(uint32_t pause)
{
    return ScrollController::setScrollPause(pause);
}

void ScrollContainer::paint(YAGfx& gfx)
{
    const ScrollController::Direction direction  = m_scrollController.getDirection();
    const uint16_t                    canvasSize = (ScrollController::DIRECTION_HORIZONTAL == direction) ? getWidth() : getHeight();

    (void)m_scrollController.update(canvasSize);

    const int16_t offsetX = (ScrollController::DIRECTION_HORIZONTAL == direction) ? m_scrollController.getOffset() : 0;
    const int16_t offsetY = (ScrollController::DIRECTION_VERTICAL == direction) ? m_scrollController.getOffset() : 0;
    YAGfxCanvas   scrolledCanvas(&gfx, offsetX, offsetY, getWidth(), getHeight());

    for (uint8_t idx = 0U; idx < m_childCount; ++idx)
    {
        if (nullptr != m_children[idx])
        {
            m_children[idx]->update(scrolledCanvas);
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

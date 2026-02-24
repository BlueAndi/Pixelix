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
 * @file   ScrollController.cpp
 * @brief  Scroll Controller Helper
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ScrollController.h"

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

/* Set default scroll pause in ms. */
uint32_t ScrollController::m_scrollPause = ScrollController::DEFAULT_SCROLL_PAUSE;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

ScrollController::ScrollController() :
    m_isEnabled(false),
    m_direction(DIRECTION_HORIZONTAL),
    m_offset(0),
    m_offsetDest(0),
    m_contentSize(0U),
    m_scrollingCnt(0U),
    m_timer()
{
}

void ScrollController::enable(Direction direction, uint16_t canvasSize, uint16_t contentSize)
{
    m_isEnabled    = true;
    m_direction    = direction;
    m_contentSize  = contentSize;
    m_scrollingCnt = 0U;

    /* Start scrolling from outside the visible area. */
    m_offset       = canvasSize;
    m_offsetDest   = -static_cast<int16_t>(contentSize);

    m_timer.start(m_scrollPause);
}

void ScrollController::disable()
{
    m_isEnabled    = false;
    m_offset       = 0;
    m_offsetDest   = 0;
    m_scrollingCnt = 0U;
    m_timer.stop();
}

void ScrollController::reset(uint16_t canvasSize)
{
    m_scrollingCnt = 0U;

    if (true == m_isEnabled)
    {
        /* Reset to start position. */
        m_offset = canvasSize;
    }
    else
    {
        m_offset = 0;
    }
}

bool ScrollController::update(uint16_t canvasSize)
{
    bool updated = false;

    if ((true == m_isEnabled) && (true == m_timer.isTimeout()))
    {
        /* Move scroll offset towards destination. */
        if (m_offsetDest < m_offset)
        {
            --m_offset;
            updated = true;
        }
        else if (m_offsetDest > m_offset)
        {
            ++m_offset;
            updated = true;
        }
        else
        {
            /* Reached destination, wrap around. */
            m_offset = canvasSize;
            updated  = true;

            /* Count number of times the content was scrolled completely. */
            if (UINT32_MAX > m_scrollingCnt)
            {
                ++m_scrollingCnt;
            }
        }

        m_timer.restart();
    }

    return updated;
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

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
 * @file   ScrollController.h
 * @brief  Scroll Controller Helper
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup GFX
 *
 * @{
 */

#ifndef SCROLLCONTROLLER_H
#define SCROLLCONTROLLER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <SimpleTimer.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * A helper class that manages scrolling state.
 * This can be used by widgets that need to implement custom scroll rendering.
 */
class ScrollController
{
public:

    /**
     * Scroll direction.
     */
    enum Direction
    {
        DIRECTION_HORIZONTAL = 0, /**< Scroll horizontally (left to right) */
        DIRECTION_VERTICAL        /**< Scroll vertically (bottom to top) */
    };

    /**
     * Constructs a scroll controller.
     */
    ScrollController();

    /**
     * Destroys the scroll controller.
     */
    ~ScrollController()
    {
    }

    /**
     * Enable scrolling.
     *
     * @param[in] direction         Scroll direction (horizontal or vertical)
     * @param[in] canvasSize        Size of the canvas in pixels
     * @param[in] contentSize       Size of the content to scroll in pixels
     */
    void enable(Direction direction, uint16_t canvasSize, uint16_t contentSize);

    /**
     * Disable scrolling and reset to initial position.
     */
    void disable();

    /**
     * Is scrolling enabled?
     *
     * @return If scrolling is enabled, it will return true otherwise false.
     */
    bool isEnabled() const
    {
        return m_isEnabled;
    }

    /**
     * Get the scroll direction.
     *
     * @return Scroll direction
     */
    Direction getDirection() const
    {
        return m_direction;
    }

    /**
     * Get the current scroll offset.
     *
     * @return Scroll offset in pixels
     */
    int16_t getOffset() const
    {
        return m_offset;
    }

    /**
     * Set the scroll offset destination.
     *
     * @param[in] offset Destination offset in pixels
     */
    void setOffsetDestination(int16_t offset)
    {
        m_offsetDest = offset;
    }

    /**
     * Get content size.
     *
     * @return Content size in pixels
     */
    uint16_t getContentSize() const
    {
        return m_contentSize;
    }

    /**
     * Reset scrolling to the start position.
     *
     * @param[in] canvasSize Size of the canvas in pixels
     */
    void reset(uint16_t canvasSize);

    /**
     * Update scroll position if timer has elapsed.
     *
     * @param[in] canvasSize Size of the canvas in pixels
     *
     * @return True if scroll position was updated, false otherwise
     */
    bool update(uint16_t canvasSize);

    /**
     * Get scrolling count.
     *
     * @return How often was the content completely scrolled over the canvas
     */
    uint32_t getScrollingCount() const
    {
        return m_scrollingCnt;
    }

    /**
     * Change scroll speed by changing the pause between each movement.
     * This is a static method, which affects all scroll controller instances.
     *
     * @param[in] pause Scroll pause in ms
     *
     * @return If successful set, it will return true otherwise false.
     */
    static bool setScrollPause(uint32_t pause)
    {
        bool status = false;

        if ((MIN_SCROLL_PAUSE <= pause) &&
            (MAX_SCROLL_PAUSE >= pause))
        {
            m_scrollPause = pause;
            status        = true;
        }

        return status;
    }

    /**
     * Get current scroll pause.
     *
     * @return Scroll pause in ms
     */
    static uint32_t getScrollPause()
    {
        return m_scrollPause;
    }

    /** Default pause between scroll steps in ms */
    static const uint32_t DEFAULT_SCROLL_PAUSE = 80U;

    /** Minimal scroll pause in ms */
    static const uint32_t MIN_SCROLL_PAUSE     = 20U;

    /** Maximal scroll pause in ms */
    static const uint32_t MAX_SCROLL_PAUSE     = 500U;

private:

    bool        m_isEnabled;    /**< Is scrolling enabled? */
    Direction   m_direction;    /**< Scroll direction */
    int16_t     m_offset;       /**< Current scroll offset in pixels */
    int16_t     m_offsetDest;   /**< Destination scroll offset in pixels */
    uint16_t    m_contentSize;  /**< Size of content to scroll (width or height depending on direction) */
    uint32_t    m_scrollingCnt; /**< Counts how often content was completely scrolled */
    SimpleTimer m_timer;        /**< Timer for scroll animation */

    /**
     * Pause in ms between each scroll movement.
     * Used by all scroll controller instances.
     */
    static uint32_t m_scrollPause;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* SCROLLCONTROLLER_H */

/** @} */

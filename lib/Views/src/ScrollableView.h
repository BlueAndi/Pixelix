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
 * @file   ScrollableView.h
 * @brief  Scrollable view helper
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef SCROLLABLEVIEW_H
#define SCROLLABLEVIEW_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

#include <TextWidget.h>
#include <ScrollContainer.h>

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
 * Helper for views which scroll a text widget and optional sibling widgets.
 */
class ScrollableView
{
public:

    /**
     * Constructs a scrollable view helper.
     *
     * @param[in] width  View width in pixels.
     * @param[in] height View height in pixels.
     */
    ScrollableView(uint16_t width, uint16_t height) :
        m_scrollContainer(width, height),
        m_width(width),
        m_height(height)
    {
    }

    /**
     * Constructs a helper for one existing widget.
     *
     * @param[in] widget Widget which shall be scrolled.
     */
    explicit ScrollableView(Widget& widget) :
        m_scrollContainer(widget.getWidth(), widget.getHeight()),
        m_width(widget.getWidth()),
        m_height(widget.getHeight())
    {
        int16_t offsetX = 0;
        int16_t offsetY = 0;

        widget.getPos(offsetX, offsetY);
        m_scrollContainer.move(offsetX, offsetY);
        widget.move(0, 0);
        addScrollableWidget(widget);
    }

    /**
     * Adds a non-owned widget to the scrolling content.
     *
     * @param[in] widget Widget which shall be added.
     */
    void addScrollableWidget(Widget& widget)
    {
        (void)m_scrollContainer.add(widget);
    }

    /**
     * Measures and updates the scrolling content.
     *
     * @param[in] gfx         Graphics interface.
     * @param[in] textWidget  Text widget used to determine content size.
     * @param[in] contentX    Horizontal content offset in pixels.
     */
    void update(YAGfx& gfx, TextWidget& textWidget, uint16_t contentX = 0U)
    {
        uint16_t textWidth             = 0U;
        uint16_t textHeight            = 0U;
        int16_t  textOffsetX           = 0;
        int16_t  textOffsetY           = 0;
        uint16_t textWidgetWidth       = textWidget.getWidth();
        bool     isHorizontalScrolling = false;

        textWidget.getPos(textOffsetX, textOffsetY);
        (void)textOffsetY;

        if (true == textWidget.getTextSize(textWidth, textHeight))
        {
            if (m_height < textHeight)
            {
                m_scrollContainer.setContentSize(textHeight);
                m_scrollContainer.enableScrolling(ScrollController::DIRECTION_VERTICAL);
            }
            else
            {
                (void)textWidget.getTextSize(textWidth, textHeight, false);
                const uint16_t widgetOffsetX  = (0 < textOffsetX) ? static_cast<uint16_t>(textOffsetX) : 0U;
                const uint16_t contentOffsetX = (contentX > widgetOffsetX) ? contentX : widgetOffsetX;
                const uint16_t contentWidth   = contentOffsetX + textWidth;

                m_scrollContainer.setContentSize(contentWidth);
                m_scrollContainer.enableScrolling(ScrollController::DIRECTION_HORIZONTAL);
                isHorizontalScrolling = (m_width < contentWidth);

                if (true == isHorizontalScrolling)
                {
                    textWidget.setWidth(textWidth);
                }
            }
        }
        else
        {
            m_scrollContainer.disableScrolling();
        }

        m_scrollContainer.update(gfx);

        if (true == isHorizontalScrolling)
        {
            textWidget.setWidth(textWidgetWidth);
        }
    }

    /**
     * Disables scrolling and resets the content position.
     */
    void disableScrolling()
    {
        m_scrollContainer.disableScrolling();
    }

    /**
     * Gets scroll information.
     *
     * @param[out] isScrollingEnabled True if scrolling is enabled.
     * @param[out] scrollingCnt Number of completed cycles.
     * @return True when information is available.
     */
    bool getScrollInfo(bool& isScrollingEnabled, uint32_t& scrollingCnt) const
    {
        return m_scrollContainer.getScrollInfo(isScrollingEnabled, scrollingCnt);
    }

protected:

    ScrollContainer m_scrollContainer; /**< Shared scroll container. */
    uint16_t        m_width;           /**< View width. */
    uint16_t        m_height;          /**< View height. */

private:

    ScrollableView(const ScrollableView& other);
    ScrollableView& operator=(const ScrollableView& other);
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* SCROLLABLEVIEW_H */

/** @} */

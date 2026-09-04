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
 * @file   View.h
 * @brief  SysMsgPlugin view
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef SYS_MSG_PLUGIN_VIEW_H
#define SYS_MSG_PLUGIN_VIEW_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <TextViewBase.hpp>
#include <ScrollContainer.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Internal plugin functionality. */
namespace _SysMsgPlugin
{

/**
 * SysMsgPlugin view.
 */
class View : public TextViewBase
{
public:

    /**
     * Construct the view.
     */
    View() :
        TextViewBase()
    {
        m_scrollContainer.add(m_textWidget);
        m_textWidget.setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_CENTER);
        m_textWidget.setVerticalAlignment(Alignment::Vertical::VERTICAL_CENTER);
    }

    /**
     * Destroy the view.
     */
    ~View()
    {
    }

    /**
     * Clear text.
     */
    void clear()
    {
        m_textWidget.clear();
        m_scrollContainer.disableScrolling();
    }

    /**
     * Update the view and its scroll container.
     *
     * @param[in] gfx Graphics interface.
     */
    void update(YAGfx& gfx) override
    {
        uint16_t textWidth  = 0U;
        uint16_t textHeight = 0U;

        gfx.fillScreen(ColorDef::BLACK);

        if (true == m_textWidget.getTextSize(textWidth, textHeight))
        {
            (void)textHeight;
            m_scrollContainer.setContentSize(textWidth);
            m_scrollContainer.enableScrolling(ScrollController::DIRECTION_HORIZONTAL);
        }
        else
        {
            m_scrollContainer.disableScrolling();
        }

        m_scrollContainer.update(gfx);
    }

    /**
     * Get scrolling informations.
     *
     * @param[out] isScrollingEnabled   Is scrolling enabled or not?
     * @param[out] scrollingCnt         How often was the text complete scrolled over the display?
     *
     * @return If scroll information is ready, it will return true otherwise false.
     */
    bool getScrollInfo(bool& isScrollingEnabled, uint32_t& scrollingCnt)
    {
        return m_scrollContainer.getScrollInfo(isScrollingEnabled, scrollingCnt);
    }

private:

    ScrollContainer m_scrollContainer{TEXT_WIDTH, TEXT_HEIGHT, TEXT_X, TEXT_Y}; /**< Container for the text widget. */

    View(const View& other);
    View& operator=(const View& other);
};

} // namespace _SysMsgPlugin

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* SYS_MSG_PLUGIN_VIEW_H */

/** @} */

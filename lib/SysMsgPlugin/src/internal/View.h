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
        return m_textWidget.getScrollInfo(isScrollingEnabled, scrollingCnt);
    }

private:

    View(const View& other);
    View& operator=(const View& other);
};

} /* _SysMsgPlugin */

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* SYS_MSG_PLUGIN_VIEW_H */

/** @} */

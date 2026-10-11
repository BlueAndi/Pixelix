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
#include <IconTextViewBase.hpp>

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
 *
 * It is based on the icon and text view, because a notification may carry a
 * icon. As long as no icon is loaded, the text uses the full display width,
 * which keeps the appearance of the system messages without a icon.
 */
class View : public IconTextViewBase
{
public:

    /**
     * Construct the view.
     */
    View() :
        IconTextViewBase()
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
     * Clear text and icon.
     */
    void clear()
    {
        m_textWidget.clear();
        clearIcon();
        disableScrolling();
    }

private:

    View(const View& other);
    View& operator=(const View& other);
};

} // namespace _SysMsgPlugin

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* SYS_MSG_PLUGIN_VIEW_H */

/** @} */

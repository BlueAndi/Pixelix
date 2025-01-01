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
 * @brief  GrabViaRestPlugin view
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef GRAB_VIA_REST_PLUGIN_VIEW_H
#define GRAB_VIA_REST_PLUGIN_VIEW_H

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
namespace _GrabViaRestPlugin
{

/**
 * GrabViaRestPlugin view.
 */
class View : public IconTextViewBase
{
public:

    /**
     * Construct the view.
     */
    View() :
        IconTextViewBase(),
        m_isTextOnly(false)
    {
    }

    /**
     * Destroy the view.
     */
    ~View()
    {
    }

    /**
     * Initialize view, which will prepare the widgets and the default values.
     * 
     * @param[in] width     Display width in pixel.
     * @param[in] height    Display height in pixel.
     */
    void init(uint16_t width, uint16_t height) final
    {
        IconTextViewBase::init(width, height);

        setFormatText("{hc}?");
    }

    /**
     * Setup layout for text only.
     */
    void setupTextOnly()
    {
        m_textWidget.move(0, 0);
        m_textWidget.setWidth(CONFIG_LED_MATRIX_WIDTH);
        m_textWidget.setHeight(CONFIG_LED_MATRIX_HEIGHT);
        m_bitmapWidget.clear(ColorDef::BLACK);

        m_isTextOnly = true;
    }

    /**
     * Setup layout for bitmap and text.
     */
    void setupBitmapAndText()
    {
        m_textWidget.move(TEXT_X, TEXT_Y);
        m_textWidget.setWidth(TEXT_WIDTH);
        m_textWidget.setHeight(TEXT_HEIGHT);

        m_isTextOnly = false;
    }

private:

    bool    m_isTextOnly;   /**< Is text only layout selected? */

    View(const View& other);
    View& operator=(const View& other);
};

} /* _GrabViaRestPlugin */

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* GRAB_VIA_REST_PLUGIN_VIEW_H */

/** @} */

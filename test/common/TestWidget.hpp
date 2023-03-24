/* MIT License
 *
 * Copyright (c) 2019 - 2023 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Widget for test purposes.
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup common
 *
 * @{
 */

#ifndef TEST_WIDGET_HPP
#define TEST_WIDGET_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Widget.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Widget for test purposes.
 * It draws only a rectangle with a specific color.
 */
class TestWidget : public Widget
{
public:

    /**
     * Constructs a widget for testing purposes at position (0, 0).
     * The drawing pen color is set to black.
     */
    TestWidget() :
        Widget(WIDGET_TYPE, 0, 0),
        m_color(0U)
    {
    }

    /**
     * Destroys widget for testing purposes.
     */
    ~TestWidget()
    {
    }

    /**
     * Get pen color, used to draw the widget.
     *
     * @return Pen color
     */
    const Color& getPenColor() const
    {
        return m_color;
    }

    /**
     * Set pen color, used to draw the widget.
     *
     * @param[in] color Pen color
     */
    void setPenColor(const Color& color)
    {
        m_color = color;
        return;
    }

    static const uint16_t           WIDTH       = 10U;  /**< Widget width in pixel */
    static const uint16_t           HEIGHT      = 5U;   /**< Widget height in pixel */
    static constexpr const char*    WIDGET_TYPE = "test";   /**< Widget type string */

private:

    Color m_color;  /**< Pen color, used to draw the widget. */

    /**
     * Update widget drawing.
     *
     * @param[in] gfx Graphics interface, which to use.
     */
    void paint(YAGfx& gfx)
    {
        int16_t x = 0;
        int16_t y = 0;

        for(y = 0; y < HEIGHT; ++y)
        {
            for(x = 0; x < WIDTH; ++x)
            {
                gfx.drawPixel(m_posX + x, m_posY + y, m_color);
            }
        }

        return;
    }
    
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* TEST_WIDGET_HPP */

/** @} */
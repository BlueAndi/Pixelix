/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Canvas
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef __CANVAS_H__
#define __CANVAS_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <WString.h>
#include <LinkedList.hpp>
#include <Widget.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This class defines a drawing canvas. The canvas can contain several widgets
 * and will update their drawings.
 */
class Canvas : public IGfx, public Widget
{
public:

    /**
     * Constructs a canvas.
     *
     * @param[in] width     Canvas width in pixel.
     * @param[in] height    Canvas height in pixel.
     * @param[in] x         x-coordinate position in the matrix.
     * @param[in] y         y-coordinate position in the matrix.
     */
    Canvas(int16_t width, int16_t height, int16_t x, int16_t y) :
        IGfx(width, height),
        Widget(WIDGET_TYPE, x, y),
        m_gfx(nullptr),
        m_widgets()
    {
    }

    /**
     * Destroys the canvas.
     */
    ~Canvas()
    {
        /* Remove all widgets */
        m_widgets.clear();
    }

    /**
     * Add a widget to the canvas.
     *
     * @param[in] widget Widget
     *
     * @return If successful, it will return true, otherwise false.
     */
    bool addWidget(Widget& widget)
    {
        Widget* ptr = &widget;
        return m_widgets.append(ptr);
    }

    /**
     * Remove a widget from the canvas.
     *
     * @param[in] widget Widget
     *
     * @return If successful, it will return true, otherwise false.
     */
    bool removeWidget(const Widget& widget)
    {
        bool                            status = false;
        DLinkedListIterator<Widget*>    it(m_widgets);

        /* Find widget in the list */
        if (true == it.find(&const_cast<Widget&>(widget)))
        {
            /* Remove widget */
            it.remove();
            status = true;
        }

        return status;
    }

    /**
     * Get all widget children.
     *
     * @return Children
     */
    const DLinkedList<Widget*>& children() const
    {
        return m_widgets;
    }

    /**
     * Update/Draw the widgets in the canvas with the
     * given graphics interface.
     *
     * @param[in] gfx   Graphics interface
     */
    void update(IGfx& gfx) override
    {
        DLinkedListIterator<Widget*> it(m_widgets);

        /* Walk through all widgets and draw them in the priority as
         * they were added.
         */
        if (true == it.first())
        {
            m_gfx = &gfx;

            do
            {
                (*it.current())->update(*this);
            }
            while(true == it.next());

            m_gfx = nullptr;
        }

        return;
    }

    /**
     * Get pixel color at given position.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color in RGB565 format.
     */
    uint16_t getColor(int16_t x, int16_t y) override
    {
        uint16_t color565 = 0U;

        if ((0 <= x) &&
            (0 <= y) &&
            (width() > x) &&
            (height() > y) &&
            (nullptr != m_gfx))
        {
            color565 = m_gfx->getColor(x, y);
        }

        return color565;
    }

    /**
     * Find widget by its name.
     *
     * @param[in] name  Widget name to search for
     *
     * @return If widget is found, it will be returned otherwise nullptr.
     */
    Widget* find(const String& name) override
    {
        Widget* widget = nullptr;

        if (name == m_name)
        {
            widget = this;
        }

        /* If its not the canvas itself, continue searching in the widget list. */
        if (nullptr == widget)
        {
            DLinkedListIterator<Widget*> it(m_widgets);

            if (true == it.first())
            {
                do
                {
                    widget = (*it.current())->find(name);
                }
                while(  (nullptr == widget) &&
                        (true == it.next()));
            }
        }

        return widget;
    }

    /** Widget type string */
    static const char*      WIDGET_TYPE;

private:

    IGfx*                   m_gfx;      /**< Graphics interface of the underlying layer */
    DLinkedList<Widget*>    m_widgets;  /**< Widgets in the canvas */

    Canvas(const Canvas& canvas);
    Canvas& operator=(const Canvas& canvas);

    /**
     * Draw a single pixel in the matrix and ensure that the drawing borders
     * are not violated.
     *
     * @param[in] x     x-coordinate
     * @param[in] y     y-coordinate
     * @param[in] color Pixel color
     */
    void drawPixel(int16_t x, int16_t y, uint16_t color)
    {
        if (nullptr != m_gfx)
        {
            /* Don't draw outside the canvas. */
            if ((0 <= x) &&
                (width() > x) &&
                (0 <= y) &&
                (height() > y))
            {
                m_gfx->drawPixel(m_posX + x, m_posY + y, color);
            }
        }

        return;
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __CANVAS_H__ */

/** @} */
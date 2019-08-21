/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
@brief  Canvas
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
This module provides a drawing canvas, which can contain several widgets.

*******************************************************************************/
/** @defgroup canvas Canvas
 * This module provides a drawing canvas, which can contain several widgets.
 *
 * @{
 */

#ifndef __CANVAS_HPP__
#define __CANVAS_HPP__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
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
class Canvas : public Adafruit_GFX, public Widget
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
        Adafruit_GFX(width, height),
        Widget(WIDGET_TYPE, x, y),
        m_gfx(NULL),
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
        bool status = false;

        /* Find widget in the list */
        if (true == m_widgets.selectFirstElement())
        {
            /* Widget found? */
            if (true == m_widgets.find(&const_cast<Widget&>(widget)))
            {
                /* Remove widget */
                m_widgets.removeSelected();
                status = true;
            }
        }

        return status;
    }

    /**
     * Get all widget children.
     * 
     * @return Children
     */
    const LinkedList<Widget*>& children(void) const
    {
        return m_widgets;
    }

    /**
     * Update/Draw the widgets in the canvas with the
     * given graphics interface.
     * 
     * @param[in] gfx   Graphics interface
     */
    void update(Adafruit_GFX& gfx)
    {
        /* Walk through all widgets and draw them in the priority as
         * they were added.
         */
        if (true == m_widgets.selectFirstElement())
        {
            m_gfx = &gfx;

            do
            {
                (*m_widgets.current())->update(*this);
            }
            while(true == m_widgets.next());

            m_gfx = NULL;
        }

        return;
    }

    /** Widget type string */
    static const char*      WIDGET_TYPE;

private:

    Adafruit_GFX*       m_gfx;      /**< Graphics interface of the underlying layer */
    LinkedList<Widget*> m_widgets;  /**< Widgets in the canvas */

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
        int16_t absPosX     = m_posX;
        int16_t absPosY     = m_posX;
        bool    outOfCanvas = false;

        /* Absolute position in the underlying area is the canvas position and
         * the pixel coordinates inside the canvas.
         * Prevent overflow here for x-coordinate.
         */
        if ((0 <= x) &&
            ((INT16_MAX - x) >= absPosX))
        {
            absPosX += x;
        }
        else if ((0 > x) &&
                ((INT16_MIN - x) <= absPosX))
        {
            absPosX += x;
        }
        else
        {
            outOfCanvas = true;
        }

        /* Don't draw outside the canvas width. */
        if (false == outOfCanvas)
        {
            if (0 >= width())
            {
                outOfCanvas = true;
            }
            else if (width() <= absPosX)
            {
                outOfCanvas = true;
            }
            else
            {
                ;
            }
        }

        /* Absolute position in the underlying area is the canvas position and
         * the pixel coordinates inside the canvas.
         * Prevent overflow here for y-coordinate.
         */
        if ((0 <= y) &&
            ((INT16_MAX - y) >= absPosX))
        {
            absPosY += y;
        }
        else if ((0 > y) &&
                ((INT16_MIN - y) <= absPosX))
        {
            absPosY += y;
        }
        else
        {
            outOfCanvas = true;
        }

        /* Don't draw outside the canvas height. */
        if (false == outOfCanvas)
        {
            if (0 >= height())
            {
                outOfCanvas = true;
            }
            else if (height() <= absPosY)
            {
                outOfCanvas = true;
            }
            else
            {
                ;
            }
        }

        /* Draw inside the canvas? */
        if ((NULL != m_gfx) &&
            (false == outOfCanvas))
        {
            m_gfx->drawPixel(absPosX, absPosY, color);
        }

        return;
    }
};

const char* Canvas::WIDGET_TYPE = "canvas";

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __CANVAS_HPP__ */

/** @} */
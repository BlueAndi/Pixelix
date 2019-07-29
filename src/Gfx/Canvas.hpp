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
#include <Adafruit_GFX.h>

#include "Widget.hpp"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This class defines a drawing canvas. The canvas can contain several widgets
 * and will update their drawings.
 * 
 * @param[in] maxWidgets Max. widgets on the canvas
 */
template < uint8_t maxWidgets >
class Canvas : public Adafruit_GFX
{
public:

    /**
     * Constructs a canvas.
     * 
     * @param[in] x         x-coordinate position in the matrix.
     * @param[in] y         y-coordinate position in the matrix.
     * @param[in] width     Canvas width in pixel
     * @param[in] height    Canvas height in pixel
     * @param[in] matrix    LED matrix, where to draw
     */
    Canvas(uint16_t x, uint16_t y, uint16_t width, uint16_t height, LedMatrix& matrix) :
        Adafruit_GFX(width, height),
        m_x(x),
        m_y(y),
        m_matrix(matrix),
        m_widgets()
    {
        uint8_t index = 0u;

        for(index = 0u; index < maxWidgets; ++index)
        {
            m_widgets[index] = NULL;
        }
    }

    /**
     * Destroys the canvas.
     */
    ~Canvas()
    {
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
        uint8_t index   = 0u;
        bool    isAdded = false;

        for(index = 0u; index < maxWidgets; ++index)
        {
            if (NULL != m_widgets[index])
            {
                m_widgets[index] = &widget;
                isAdded = true;
                break;
            }
        }

        return isAdded;
    }

    /**
     * Remove a widget from the canvas.
     * 
     * @param[in] widget Widget
     * 
     * @return If successful, it will return true, otherwise false.
     */
    void removeWidget(Widget& widget)
    {
        uint8_t index       = 0u;
        bool    isRemoved   = false;

        for(index = 0u; index < maxWidgets; ++index)
        {
            if (&widget == m_widgets[index])
            {
                m_widgets[index] = NULL;
                isRemoved = true;
                break;
            }
        }

        return isRemoved;
    }

    /**
     * Update the canvas, which update/draw all contained widgets.
     */
    void update(void)
    {
        uint8_t index = 0u;

        for(index = 0u; index < maxWidgets; ++index)
        {
            if (NULL != m_widgets[index])
            {
                m_widgets[index]->update(*this);
            }
        }

        return;
    }

private:

    uint16_t    m_x;                    /**< x-coordinate of upper left corner in the matrix */
    uint16_t    m_y;                    /**< y-coordinate of upper left corner in the matrix */
    LedMatrix&  m_matrix;               /**< LED matrix, where to draw everything */
    Widget*     m_widgets[maxWidgets];  /**< Widgets in the canvas */

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
        if ((0 <= x) &&
            (_width > x) &&
            (0 <= y) &&
            (_height > y))
        {
            m_matrix.drawPixel(m_x + x, m_y + y, color);
        }

        return;
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __CANVAS_HPP__ */

/** @} */
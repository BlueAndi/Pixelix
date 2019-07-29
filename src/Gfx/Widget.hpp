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
@brief  Base Widget
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
This module provides the basic widget.

*******************************************************************************/
/** @defgroup widget Base Widget
 * This module provides the basic widget.
 *
 * @{
 */

#ifndef __WIDGET_HPP__
#define __WIDGET_HPP__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <Adafruit_GFX.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Base widget, which contains the position
 * inside a canvas and declares the graphics interface.
 */
class Widget
{
public:

    /**
     * Constructs a widget at position (0, 0) in the canvas.
     */
    Widget() :
        m_posX(0),
        m_posY(0)
    {
    }

    /**
     * Constructs a widget a the given position in the canvas.
     * 
     * @param[in] x Upper left corner (x-coordinate) of the widget in a canvas.
     * @param[in] y Upper left corner (y-coordinate) of the widget in a canvas.
    */
    Widget(uint16_t x, uint16_t y) :
        m_posX(x),
        m_posY(y)
    {
    }

    /**
     * Constructs a widget by copying a widget.
     * 
     * @param[in] widget The widget, which to copy.
    */
    Widget(const Widget& widget) :
        m_posX(widget.m_posX),
        m_posY(widget.m_posY)
    {
    }

    /**
     * Destroys a widget.
     */
    virtual ~Widget()
    {
    }

    /**
     * Assign content of another widget.
     * 
     * @param[in] widget The widget, which to copy.
     * 
     * @return Widget
    */
    Widget& operator=(const Widget& widget)
    {
        m_posX = widget.m_posX;
        m_posY = widget.m_posY;
        return *this;
    }

    /**
     * Move the widget to the given position in the canvas.
     * 
     * @param[in] x Upper left corner (x-coordinate) of the widget in a canvas.
     * @param[in] y Upper left corner (y-coordinate) of the widget in a canvas.
     */
    void move(uint16_t x, uint16_t y)
    {
        m_posX = x;
        m_posY = y;
        return;
    }

    /**
     * Get current position in the canvas.
     * 
     * @param[in] x Upper left corner (x-coordinate) of the widget in a canvas.
     * @param[in] y Upper left corner (y-coordinate) of the widget in a canvas.
     */
    void getPos(uint16_t& x, uint16_t& y) const
    {
        x = m_posX;
        y = m_posY;
        return;
    }

    /**
     * Update/Draw the widget in the canvas with the
     * given graphics interface.
     * 
     * @param[in] gfx   Graphics interface
     */
    virtual void update(Adafruit_GFX& gfx) = 0;

protected:

    uint16_t    m_posX; /**< Upper left corner (x-coordinate) of the widget in a canvas. */
    uint16_t    m_posY; /**< Upper left corner (y-coordinate) of the widget in a canvas. */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __WIDGET_HPP__ */

/** @} */
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
@brief  Lamp Widget
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
This module provides the a lamp widget, showing the behaviour like a colored lamp.
Its size is fixed set.

*******************************************************************************/
/** @defgroup lampwidget Lamp Widget
 * This module provides the a lamp widget, showing the behaviour like a colored lamp.
 * Its size is fixed set.
 *
 * @{
 */

#ifndef __LAMPWIDGET_HPP__
#define __LAMPWIDGET_HPP__

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
 * The lamp widget, showing the behaviour like a lamp with a fixed size.
 */
class LampWidget : public Widget
{
public:

    /**
     * Constructs a lamp widget, being black in off state and white in on state.
     */
    LampWidget() :
        m_isOn(false),
        m_colorOff(0x0000),
        m_colorOn(0xFFFF)
    {
    }

    /**
     * Constructs a lamp widget.
     * 
     * @param[in] isOn      Initial on state
     * @param[in] colorOff  Lamp color in off state
     * @param[in] colorOn   Lamp color in on state
     */
    LampWidget(bool isOn, uint16_t colorOff, uint16_t colorOn) :
        m_isOn(isOn),
        m_colorOff(colorOff),
        m_colorOn(colorOn)
    {
    }

    /**
     * Constructs a lamp widget, by copying another one.
     * 
     * @param[in] widget Lamp widget, which to copy
     */
    LampWidget(const LampWidget& widget) :
        m_isOn(widget.m_isOn),
        m_colorOff(m_colorOff),
        m_colorOn(widget.m_colorOn)
    {
    }

    /**
     * Assigns the content of another lamp widget.
     * 
     * @param[in] widget Lamp widget, which to assign
     */
    LampWidget& operator=(const LampWidget& widget)
    {
        m_isOn      = widget.m_isOn;
        m_colorOff  = widget.m_colorOff;
        m_colorOn   = widget.m_colorOn;

        return *this;
    }

    /**
     * Update/Draw the lamp widget with the given graphics interface.
     * 
     * @param[in] gfx Graphics interface.
     */
    void update(Adafruit_GFX& gfx)
    {
        const uint16_t  x0      = 0u;
        const uint16_t  y0      = 0u;
        uint16_t        color   = m_colorOn;

        if (false == m_isOn)
        {
            color = m_colorOff;
        }

        gfx.fillRect(x0, y0, x0 + WIDTH, y0 + HEIGHT, color);

        return;
    }

    /**
     * Set the on state.
     * 
     * @param[in] state On state
     */
    void setOnState(bool state)
    {
        m_isOn = state;

        return;
    }

    /**
     * Get the curren on state.
     * 
     * @return On state
     */
    bool getOnState(void) const
    {
        return m_isOn;
    }

    /**
     * Set lamp color in off state.
     * 
     * @param[in] color Lamp color in off state
     */
    void setColorOff(uint16_t color)
    {
        m_colorOff = color;

        return;
    }

    /**
     * Get color in off state.
     * 
     * @return Color in off state
     */
    uint16_t getColorOff(void) const
    {
        return m_colorOff;
    }

    /**
     * Set lamp color in on state.
     * 
     * @param[in] color Lamp color in on state
     */
    void setColorOn(uint16_t color)
    {
        m_colorOn = color;

        return;
    }

    /**
     * Get color in on state.
     * 
     * @return Color in on state
     */
    uint16_t getColorOn(void) const
    {
        return m_colorOn;
    }

    /** Lamp width in pixel */
    static const uint16_t WIDTH     = 4u;

    /** Lamp height in pixel */
    static const uint16_t HEIGHT    = 1u;

private:

    bool        m_isOn;     /**< Lamp on state */
    uint16_t    m_colorOff; /**< Lamp color in off state */
    uint16_t    m_colorOn;  /**< Lamp color in on state */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __LAMPWIDGET_HPP__ */

/** @} */
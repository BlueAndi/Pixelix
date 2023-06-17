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
 * @brief  Lamp Widget
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef LAMPWIDGET_H
#define LAMPWIDGET_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <Widget.hpp>
#include <YAColor.h>

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
        Widget(WIDGET_TYPE),
        m_isOn(false),
        m_colorOff(ColorDef::BLACK),
        m_colorOn(ColorDef::RED),
        m_width(DEFAULT_WIDTH)
    {
    }

    /**
     * Constructs a lamp widget.
     *
     * @param[in] isOn      Initial on state
     * @param[in] colorOff  Lamp color in off state
     * @param[in] colorOn   Lamp color in on state
     * @param[in] width     Lamp width in pixel
     */
    LampWidget(bool isOn, const Color& colorOff, const Color& colorOn, uint16_t width) :
        Widget(WIDGET_TYPE),
        m_isOn(isOn),
        m_colorOff(colorOff),
        m_colorOn(colorOn),
        m_width(width)
    {
    }

    /**
     * Constructs a lamp widget, by copying another one.
     *
     * @param[in] widget Lamp widget, which to copy
     */
    LampWidget(const LampWidget& widget) :
        Widget(WIDGET_TYPE),
        m_isOn(widget.m_isOn),
        m_colorOff(widget.m_colorOff),
        m_colorOn(widget.m_colorOn),
        m_width(widget.m_width)
    {
    }

    /**
     * Destroys the lamp widget.
     */
    ~LampWidget()
    {
    }

    /**
     * Assigns the content of another lamp widget.
     *
     * @param[in] widget Lamp widget, which to assign
     */
    LampWidget& operator=(const LampWidget& widget)
    {
        if (&widget != this)
        {
            Widget::operator=(widget);
            
            m_isOn      = widget.m_isOn;
            m_colorOff  = widget.m_colorOff;
            m_colorOn   = widget.m_colorOn;
            m_width     = widget.m_width;
        }

        return *this;
    }

    /**
     * Set the on state.
     *
     * @param[in] state On state
     */
    void setOnState(bool state)
    {
        m_isOn = state;
    }

    /**
     * Get the current on state.
     *
     * @return On state
     */
    bool getOnState() const
    {
        return m_isOn;
    }

    /**
     * Set lamp color in off state.
     *
     * @param[in] color Lamp color in off state
     */
    void setColorOff(const Color& color)
    {
        m_colorOff = color;
    }

    /**
     * Get color in off state.
     *
     * @return Color in off state
     */
    const Color& getColorOff() const
    {
        return m_colorOff;
    }

    /**
     * Set lamp color in on state.
     *
     * @param[in] color Lamp color in on state
     */
    void setColorOn(const Color& color)
    {
        m_colorOn = color;
    }

    /**
     * Get color in on state.
     *
     * @return Color in on state
     */
    const Color& getColorOn() const
    {
        return m_colorOn;
    }

     /**
     * Set the width of the lamp.
     *
     * @param[in] width width of the lamp
     */
    void setWidth(uint16_t width)
    {
        m_width = width;
    }

    /**
     * Get the width of the lamp.
     *
     * @return width
     */
    uint16_t getWidth() const
    {
        return m_width;
    }

    /** Widget type string */
    static const char*      WIDGET_TYPE;

    /** Lamp width in pixel */
    static const uint16_t   DEFAULT_WIDTH   = 4U;

    /** Lamp height in pixel */
    static const uint16_t   HEIGHT          = 1U;

private:

    bool        m_isOn;     /**< Lamp on state */
    Color       m_colorOff; /**< Lamp color in off state */
    Color       m_colorOn;  /**< Lamp color in on state */
    uint16_t    m_width;    /**< Lamp width in pixel */

    /**
     * Paint the widget with the given graphics interface.
     * 
     * @param[in] gfx   Graphics interface
     */
    void paint(YAGfx& gfx) override
    {
        Color color = m_colorOn;

        if (false == m_isOn)
        {
            color = m_colorOff;
        }

        gfx.fillRect(m_posX, m_posY, m_width, HEIGHT, color);
    }

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* LAMPWIDGET_H */

/** @} */
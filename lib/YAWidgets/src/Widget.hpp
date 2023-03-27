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
 * @brief  Base Widget
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup gfx
 *
 * @{
 */

#ifndef WIDGET_HPP
#define WIDGET_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <WString.h>
#include <YAGfx.h>

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
     * Destroys a widget.
     */
    virtual ~Widget()
    {
    }

    /**
     * Assign content of another widget.
     * Note, its name won't be assigned.
     * 
     * @param[in] widget The widget, which to copy.
     * 
     * @return Widget
    */
    Widget& operator=(const Widget& widget)
    {
        if (&widget != this)
        {
            m_type      = widget.m_type;
            m_posX      = widget.m_posX;
            m_posY      = widget.m_posY;
            /* m_name is not assigned! */
            m_isEnabled = widget.m_isEnabled;
        }

        return *this;
    }

    /**
     * Move the widget to the given position in the canvas.
     * 
     * @param[in] x Upper left corner (x-coordinate) of the widget in a canvas.
     * @param[in] y Upper left corner (y-coordinate) of the widget in a canvas.
     */
    void move(int16_t x, int16_t y)
    {
        m_posX = x;
        m_posY = y;
    }

    /**
     * Get current position in the canvas.
     * 
     * @param[in] x Upper left corner (x-coordinate) of the widget in a canvas.
     * @param[in] y Upper left corner (y-coordinate) of the widget in a canvas.
     */
    void getPos(int16_t& x, int16_t& y) const
    {
        x = m_posX;
        y = m_posY;
    }

    /**
     * Update/Draw the widget in the canvas with the
     * given graphics interface.
     * 
     * @param[in] gfx   Graphics interface
     */
    void update(YAGfx& gfx)
    {
        if (true == m_isEnabled)
        {
            paint(gfx);
        }
    }

    /**
     * Get widget type as string.
     * 
     * @return Widget type string
     */
    const char* getType() const
    {
        return m_type;
    }

    /**
     * Get widget name.
     * If no name is set, an empty string will be returned.
     * 
     * @return Name
     */
    const String& getName() const
    {
        return m_name;
    }

    /**
     * Set widget name.
     * 
     * @param[in] name Name to set
     */
    void setName(const String& name)
    {
        m_name = name;
    }

    /**
     * Is widget enabled?
     * 
     * @return If widget is enabled, it will return true otherwise false.
     */
    bool isEnabled() const
    {
        return m_isEnabled;
    }

    /**
     * Enable widget.
     * If the widget is enabled, it will be drawn.
     */
    void enable()
    {
        m_isEnabled = true;
    }

    /**
     * Disable widget.
     * If the widget is disabled, it won't be drawn.
     */
    void disable()
    {
        m_isEnabled = false;
    }

    /**
     * Find widget by its name.
     * Note, it must be overriden by the inherited widget, if it is like a
     * container of widgets.
     * 
     * @param[in] name  Widget name to search for
     * 
     * @return If widget is found, it will be returned otherwise nullptr.
     */
    virtual Widget* find(const String& name)
    {
        Widget* widget = nullptr;

        if (name == m_name)
        {
            widget = this;
        }

        return widget;
    }

protected:

    const char* m_type;         /**< Widget type string */
    int16_t     m_posX;         /**< Upper left corner (x-coordinate) of the widget in a canvas. */
    int16_t     m_posY;         /**< Upper left corner (y-coordinate) of the widget in a canvas. */
    String      m_name;         /**< Widget name for identification. */
    bool        m_isEnabled;    /**< If widget is enabled, it will be drawn otherwise not. */

    /**
     * Constructs a widget at position (0, 0) in the canvas.
     * 
     * @param[in] type Widget type name
     */
    Widget(const char* type) :
        m_type(type),
        m_posX(0),
        m_posY(0),
        m_name(),
        m_isEnabled(true)
    {
    }

    /**
     * Constructs a widget a the given position in the canvas.
     * 
     * @param[in] type  Widget type name
     * @param[in] x     Upper left corner (x-coordinate) of the widget in a canvas.
     * @param[in] y     Upper left corner (y-coordinate) of the widget in a canvas.
    */
    Widget(const char* type, int16_t x, int16_t y) :
        m_type(type),
        m_posX(x),
        m_posY(y),
        m_name(),
        m_isEnabled(true)
    {
    }

    /**
     * Constructs a widget by copying a widget.
     * Note, its name won't be assigned.
     * 
     * @param[in] widget The widget, which to copy.
    */
    Widget(const Widget& widget) :
        m_type(widget.m_type),
        m_posX(widget.m_posX),
        m_posY(widget.m_posY),
        m_name(),
        m_isEnabled(widget.m_isEnabled)
    {
    }

    /**
     * Paint the widget with the given graphics interface.
     * 
     * @param[in] gfx   Graphics interface
     */
    virtual void paint(YAGfx& gfx) = 0;

private:

    /* Default constructor not allowed. */
    Widget();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* WIDGET_HPP */

/** @} */
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
 * @brief  Base Widget
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup GFX
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
#include <YAGfxCanvas.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The widget class is the base class of all interface objects.
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
            m_canvas    = widget.m_canvas;
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
        m_canvas.setOffsetX(x);
        m_canvas.setOffsetY(y);
    }

    /**
     * Get current position in the canvas.
     * 
     * @param[in] x Upper left corner (x-coordinate) of the widget in a canvas.
     * @param[in] y Upper left corner (y-coordinate) of the widget in a canvas.
     */
    void getPos(int16_t& x, int16_t& y) const
    {
        x = m_canvas.getOffsetX();
        y = m_canvas.getOffsetY();
    }

    /**
     * Get widget width.
     * 
     * @return Width in pixel
     */
    uint16_t getWidth() const
    {
        return m_canvas.getWidth();
    }

    /**
     * Set widget width.
     * 
     * @param[in] width Width in pixel
     */
    virtual void setWidth(uint16_t width)
    {
        m_canvas.setWidth(width);
    }

    /**
     * Get widget height.
     * 
     * @return Width in pixel
     */
    uint16_t getHeight() const
    {
        return m_canvas.getHeight();
    }

    /**
     * Set widget height.
     * 
     * @param[in] height Height in pixel
     */
    virtual void setHeight(uint16_t height)
    {
        m_canvas.setHeight(height);
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
            m_canvas.setParentGfx(gfx);
            paint(m_canvas);
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
    YAGfxCanvas m_canvas;       /**< Canvas used to draw the widget. */
    String      m_name;         /**< Widget name for identification. */
    bool        m_isEnabled;    /**< If widget is enabled, it will be drawn otherwise not. */

    /**
     * Constructs a widget a the given position in the canvas.
     * 
     * @param[in] type      Widget type name
     * @param[in] width     Widget width in pixel.
     * @param[in] height    Widget height in pixel.
     * @param[in] x         Upper left corner (x-coordinate) of the widget in a canvas.
     * @param[in] y         Upper left corner (y-coordinate) of the widget in a canvas.
    */
    Widget(const char* type, uint16_t width, uint16_t height, int16_t x = 0, int16_t y = 0) :
        m_type(type),
        m_canvas(nullptr, x, y, width, height),
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
        m_canvas(widget.m_canvas),
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
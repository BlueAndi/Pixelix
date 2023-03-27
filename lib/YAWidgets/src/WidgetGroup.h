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
 * @brief  Widget group
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef WIDGET_GROUP_H
#define WIDGET_GROUP_H

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
 * This class defines a widget group and can contain several widgets.
 */
class WidgetGroup : public Widget, private YAGfx
{
public:

    /**
     * Constructs a empty widget group.
     * 
     * @param[in] width     Canvas width in pixels
     * @param[in] height    Canvas height in pixels
     * @param[in] x         x-coordinate position in the underlying canvas.
     * @param[in] y         y-coordinate position in the underlying canvas.
     */
    WidgetGroup(uint16_t width = 0U, uint16_t height = 0U, int16_t x = 0, int16_t y = 0) :
        Widget(WIDGET_TYPE, x, y),
        m_width(width),
        m_height(height),
        m_widgets(),
        m_gfx(nullptr)
    {
    }

    /**
     * Constructs a widget group by copy.
     * 
     * @param[in] group Source group which to copy.
     */
    WidgetGroup(const WidgetGroup& group) :
        Widget(group),
        m_width(group.m_width),
        m_height(group.m_height),
        m_widgets(group.m_widgets),
        m_gfx(group.m_gfx)
    {
    }

    /**
     * Destroys the widget group.
     */
    ~WidgetGroup()
    {
    }

    /**
     * Assigns a widget group.
     * 
     * @param[in] group Source group which to assign.
     * 
     * @return Wiget group
     */
    WidgetGroup& operator=(const WidgetGroup& group)
    {
        if (&group != this)
        {
            Widget::operator=(group);

            m_width     = group.m_width;
            m_height    = group.m_height;
            m_widgets   = group.m_widgets;
            m_gfx       = group.m_gfx;
        }

        return *this;
    }

    /**
     * Get canvas width in pixel.
     *
     * @return Canvas width in pixel
     */
    uint16_t getWidth() const final
    {
        return m_width;
    }

    /**
     * Get canvas height in pixel.
     *
     * @return Canvas height in pixel
     */
    uint16_t getHeight() const final
    {
        return m_height;
    }

    /**
     * Get pixel color at given position.
     * Attention, it will only return a valid color in case of active painting!
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color in RGB888 format.
     */
    Color& getColor(int16_t x, int16_t y) final
    {
        static Color    trash;
        Color*          pixel   = &trash;

        /* Underlying canvas? */
        if (nullptr != m_gfx)
        {
            pixel = &m_gfx->getColor(x, y);
        }

        return *pixel;
    }

    /**
     * Get pixel color at given position.
     * Attention, it will only return a valid color in case of active painting!
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color in RGB888 format.
     */
    const Color& getColor(int16_t x, int16_t y) const final
    {
        static Color    trash;
        const Color*    pixel   = &trash;

        /* Underlying canvas? */
        if (nullptr != m_gfx)
        {
            pixel = &m_gfx->getColor(x, y);
        }

        return *pixel;
    }

    /**
     * Set canvas width in pixels.
     * 
     * @param[in] width Canvas width in pixels
     */
    void setWidth(uint16_t width)
    {
        m_width = width;
    }

    /**
     * Set canvas height in pixels.
     * 
     * @param[in] height Canvas height in pixels
     */
    void setHeight(uint16_t height)
    {
        m_height = height;
    }

    /**
     * Set widget group position and size.
     * 
     * @param[in] offsX     x-coordinate position in the underlying canvas.
     * @param[in] offsY     y-coordinate position in the underlying canvas.
     * @param[in] width     Canvas width in pixels
     * @param[in] height    Canvas height in pixels
     */
    void setPosAndSize(int16_t offsX, int16_t offsY, uint16_t width, uint16_t height)
    {
        m_posX      = offsX;
        m_posY      = offsY;
        m_width     = width;
        m_height    = height;
    }

    /**
     * Add a widget to the group.
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
     * Remove a widget from the group.
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

        /* If its not the group itself, continue searching in the widget list. */
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

    uint16_t                m_width;    /**< Canvas width in pixels */
    uint16_t                m_height;   /**< Canvas height in pixels */
    DLinkedList<Widget*>    m_widgets;  /**< Widgets in the group */
    YAGfx*                  m_gfx;      /**< Graphics interface of the underlying layer */

    /**
     * Paint the widget with the given graphics interface.
     * 
     * @param[in] gfx   Graphics interface
     */
    void paint(YAGfx& gfx) override
    {
        DLinkedListIterator<Widget*> it(m_widgets);

        m_gfx = &gfx;

        /* Walk through all widgets and draw them in the priority as
         * they were added.
         */
        if (true == it.first())
        {
            do
            {
                (*it.current())->update(*this);
            }
            while(true == it.next());
        }

        m_gfx = nullptr;
    }

    /**
     * Draw a single pixel and ensure that the drawing borders are not violated.
     *
     * @param[in] x     x-coordinate
     * @param[in] y     y-coordinate
     * @param[in] color Pixel color
     */
    void drawPixel(int16_t x, int16_t y, const Color& color) final
    {
        /* Don't draw outside the canvas. */
        if ((nullptr != m_gfx) &&
            (0 <= x) &&
            (0 <= y) &&
            (m_width > x) &&
            (m_height > y))
        {    
            m_gfx->drawPixel(m_posX + x, m_posY + y, color);
        }
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* WIDGET_GROUP_H */

/** @} */
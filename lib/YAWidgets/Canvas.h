/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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
class Canvas : public YAGfx, public Widget
{
public:

    /**
     * Constructs a canvas.
     *
     * @param[in] width         Canvas width in pixel.
     * @param[in] height        Canvas height in pixel.
     * @param[in] x             x-coordinate position in the matrix.
     * @param[in] y             y-coordinate position in the matrix.
     * @param[in] isBuffered    Create a buffered (true) canvas or not (false)
     */
    Canvas(uint16_t width, uint16_t height, int16_t x, int16_t y, bool isBuffered = false) :
        YAGfx(width, height),
        Widget(WIDGET_TYPE, x, y),
        m_gfx(nullptr),
        m_widgets(),
        m_buffer(nullptr)
    {
        if (true == isBuffered)
        {
            m_buffer = new Color[width * height];
        }
    }

    /**
     * Destroys the canvas.
     */
    ~Canvas()
    {
        /* Remove all widgets */
        m_widgets.clear();

        /* Release buffer */
        if (nullptr != m_buffer)
        {
            delete[] m_buffer;
            m_buffer = nullptr;
        }
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
     * Update from the canvas buffer with the given graphics interface.
     * Note, only useable in case the canvas is buffered.
     *
     * @param[in] gfx   Graphics interface
     */
    void updateFromBuffer(YAGfx& gfx)
    {
        /* In a buffered canvas, only the buffer into the underlying canvas. */
        if (nullptr != m_buffer)
        {
            int16_t x = 0;
            int16_t y = 0;

            for(y = 0; y < getHeight(); ++y)
            {
                for(x = 0; x < getWidth(); ++x)
                {
                    gfx.drawPixel(x, y, m_buffer[x + y * getWidth()]);
                }
            }
        }

        return;
    }

    /**
     * Get pixel color at given position.
     * Note, only useable in case the canvas is buffered.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color in RGB888 format.
     */
    Color getColor(int16_t x, int16_t y) const final
    {
        Color color;

        if ((0 <= x) &&
            (0 <= y) &&
            (getWidth() > x) &&
            (getHeight() > y) &&
            (nullptr != m_buffer))
        {
            color = m_buffer[x + y * getWidth()];
        }

        return color;
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

    YAGfx*                  m_gfx;      /**< Graphics interface of the underlying layer */
    DLinkedList<Widget*>    m_widgets;  /**< Widgets in the canvas */
    Color*                  m_buffer;   /**< Buffer */

    Canvas(const Canvas& canvas);
    Canvas& operator=(const Canvas& canvas);

    /**
     * Paint the widget with the given graphics interface.
     * 
     * @param[in] gfx   Graphics interface
     */
    void paint(YAGfx& gfx) override
    {
        DLinkedListIterator<Widget*> it(m_widgets);

        /* Walk through all widgets and draw them in the priority as
         * they were added.
         */
        if (true == it.first())
        {
            /* If canvas is not buffered, draw directly on the underlying canvas. */
            if (nullptr == m_buffer)
            {
                m_gfx = &gfx;
            }

            do
            {
                (*it.current())->update(*this);
            }
            while(true == it.next());

            m_gfx = nullptr;
        }

        /* In a buffered canvas, only the buffer into the underlying canvas. */
        if (nullptr != m_buffer)
        {
            int16_t x = 0;
            int16_t y = 0;

            for(y = 0; y < getHeight(); ++y)
            {
                for(x = 0; x < getWidth(); ++x)
                {
                    gfx.drawPixel(x, y, m_buffer[x + y * getWidth()]);
                }
            }
        }

        return;
    }

    /**
     * Draw a single pixel in the matrix and ensure that the drawing borders
     * are not violated.
     *
     * @param[in] x     x-coordinate
     * @param[in] y     y-coordinate
     * @param[in] color Pixel color
     */
    void drawPixel(int16_t x, int16_t y, const Color& color) final
    {
        /* Don't draw outside the canvas. */
        if ((0 <= x) &&
            (getWidth() > x) &&
            (0 <= y) &&
            (getHeight() > y))
        {
            /* Draw on the real underlying canvas? */
            if (nullptr != m_gfx)
            {
                m_gfx->drawPixel(m_posX + x, m_posY + y, color);
            }
            /* Draw into buffer? */
            else if (nullptr != m_buffer)
            {
                m_buffer[x + y * getWidth()] = color;
            }
            /* Skip drawing */
            else
            {
                ;
            }
        }

        return;
    }

    /**
     * Dim color to black.
     * A dim ratio of 255 means no change.
     * 
     * Note, the base colors may be destroyed, depends on the color type.
     *
     * @param[in] x     x-coordinate
     * @param[in] y     y-coordinate
     * @param[in] ratio Dim ration [0; 255]
     */
    void dimPixel(int16_t x, int16_t y, uint8_t ratio) final
    {
        /* Don't draw outside the canvas. */
        if ((0 <= x) &&
            (getWidth() > x) &&
            (0 <= y) &&
            (getHeight() > y))
        {
            /* Draw on the real underlying canvas? */
            if (nullptr != m_gfx)
            {
                m_gfx->dimPixel(m_posX + x, m_posY + y, ratio);
            }
            /* Draw into buffer? */
            else if (nullptr != m_buffer)
            {
                m_buffer[x + y * getWidth()].setIntensity(ratio);
            }
            /* Skip drawing */
            else
            {
                ;
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
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
 * @brief  Bitmap Widget
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup gfx
 *
 * @{
 */

#ifndef __BITMAPWIDGET_H__
#define __BITMAPWIDGET_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <Widget.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Bitmap widget, showing a simple bitmap.
 */
class BitmapWidget : public Widget
{
public:

    /**
     * Constructs a bitmap widget, which is empty.
     */
    BitmapWidget() :
        Widget(WIDGET_TYPE),
        m_buffer(NULL),
        m_bufferSize(0U),
        m_width(0U),
        m_height(0U)
    {
    }

    /**
     * Constructs a bitmap widget by copying another one.
     * 
     * @param[in] widget Bitmap widge, which to copy
     */
    BitmapWidget(const BitmapWidget& widget) :
        Widget(WIDGET_TYPE),
        m_buffer(NULL),
        m_bufferSize(widget.m_bufferSize),
        m_width(widget.m_width),
        m_height(widget.m_height)
    {
        if (NULL != widget.m_buffer)
        {
            m_buffer = new uint16_t[m_bufferSize];

            if (NULL == m_buffer)
            {
                m_bufferSize = 0U;
            }
            else
            {
                memcpy(m_buffer, widget.m_buffer, m_bufferSize * sizeof(uint16_t));
            }
        }
    }

    /**
     * Destroys the bitmap widget.
     */
    ~BitmapWidget()
    {
        if (NULL != m_buffer)
        {
            delete[] m_buffer;
            m_buffer = NULL;
            m_bufferSize = 0U;
        }
    }

    /**
     * Assigns a existing bitmap widget.
     * 
     * @param[in] widget Bitmap widge, which to assign
     */
    BitmapWidget& operator=(const BitmapWidget& widget);

    /**
     * Update/Draw the bitmap widget on the canvas.
     * 
     * @param[in] gfx Graphics interface
     */
    void update(IGfx& gfx) override
    {
        if (NULL != m_buffer)
        {
            gfx.drawRGBBitmap(m_posX, m_posY, m_buffer, m_width, m_height);
        }
        
        return;
    }

    /**
     * Set a new bitmap.
     * 
     * @param[in] bitmap    Ext. bitmap buffer
     * @param[in] width     Bitmap width in pixel
     * @param[in] height    Bitmap height in pixel
     */
    void set(const uint16_t* bitmap, uint16_t width, uint16_t height);

    /**
     * Get the bitmap.
     * 
     * @param[out] width    Bitmap width in pixel
     * @param[out] height   Bitmap height in pixel
     * 
     * @return Bitmap buffer
     */
    const uint16_t* get(uint16_t& width, uint16_t& height) const
    {
        width   = m_width;
        height  = m_height;

        return m_buffer;
    }

    #ifndef NATIVE

    /**
     * Load bitmap image from filesystem.
     * 
     * @param[in] filename  Filename with full path
     * 
     * @return If successful loaded it will return true otherwise false.
     */
    bool load(const String& filename);

    #endif  /* NATIVE */

    /** Widget type string */
    static const char* WIDGET_TYPE;

private:

    uint16_t*   m_buffer;       /**< Raw bitmap buffer */
    size_t      m_bufferSize;   /**< Raw bitmap buffer size in number of elements */
    uint16_t    m_width;        /**< Bitmap width in pixel */
    uint16_t    m_height;       /**< Bitmap height in pixel */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __BITMAPWIDGET_H__ */

/** @} */
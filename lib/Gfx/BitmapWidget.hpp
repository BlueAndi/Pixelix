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
@brief  Bitmap Widget
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
This module provides the a bitmap widget.

*******************************************************************************/
/** @defgroup bitmapwidget Bitmap Widget
 * This module provides the a bitmap widget.
 *
 * @{
 */

#ifndef __BITMAPWIDGET_HPP__
#define __BITMAPWIDGET_HPP__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

#include "IGfx.hpp"
#include "Widget.hpp"

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
        m_buffer(NULL),
        m_width(0u),
        m_height(0u)
    {
    }

    /**
     * Constructs a bitmap widget.
     * 
     * @param[in] bitmap    Ext. bitmap buffer
     * @param[in] width     Bitmap width in pixel
     * @param[in] height    Bitmap height in pixel
     */
    BitmapWidget(const uint16_t* bitmap, uint16_t width, uint16_t height) :
        m_buffer(bitmap),
        m_width(width),
        m_height(height)
    {
    }

    /**
     * Constructs a bitmap widget by copying another one.
     * 
     * @param[in] widget Bitmap widge, which to copy
     */
    BitmapWidget(const BitmapWidget& widget) :
        m_buffer(widget.m_buffer),
        m_width(widget.m_width),
        m_height(widget.m_height)
    {
    }

    /**
     * Assigns a existing bitmap widget.
     * 
     * @param[in] widget Bitmap widge, which to assign
     */
    BitmapWidget& operator=(const BitmapWidget& widget)
    {
        m_buffer    = widget.m_buffer;
        m_width     = widget.m_width;
        m_height    = widget.m_height;

        return *this;
    }

    /**
     * Update/Draw the bitmap widget on the canvas.
     * 
     * @param[in] gfx Graphics interface
     */
    void update(IGfx& gfx)
    {
        gfx.drawRGBBitmap(m_posX, m_posY, m_buffer, m_width, m_height);
        return;
    }

    /**
     * Set a new bitmap.
     * 
     * @param[in] bitmap    Ext. bitmap buffer
     * @param[in] width     Bitmap width in pixel
     * @param[in] height    Bitmap height in pixel
     */
    void set(const uint16_t* bitmap, uint16_t width, uint16_t height)
    {
        m_buffer    = bitmap;
        m_width     = width;
        m_height    = height;

        return;
    }

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

private:

    const uint16_t* m_buffer;   /**< Ext. bitmap buffer */
    uint16_t        m_width;    /**< Bitmap width in pixel */
    uint16_t        m_height;   /**< Bitmap height in pixel */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __BITMAPWIDGET_HPP__ */

/** @} */
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
 * @brief  Bitmap Widget
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup GFX
 *
 * @{
 */

#ifndef BITMAPWIDGET_H
#define BITMAPWIDGET_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <FS.h>

#include "Widget.hpp"
#include "GifImgPlayer.h"
#include "Alignment.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Bitmap widget, showing a simple bitmap.
 * Supported are the following formats:
 * - Bitmap (.bmp)
 * - GIF image (.gif)
 */
class BitmapWidget : public Widget
{
public:

    /**
     * Constructs a bitmap widget, which is empty.
     * 
     * @param[in] width     Widget width in pixel.
     * @param[in] height    Widget height in pixel.
     * @param[in] x         Upper left corner (x-coordinate) of the widget in a canvas.
     * @param[in] y         Upper left corner (y-coordinate) of the widget in a canvas.
     */
    BitmapWidget(uint16_t width = 0U, uint16_t height = 0U, int16_t x = 0, int16_t y = 0) :
        Widget(WIDGET_TYPE, width, height, x, y),
        m_imgType(IMG_TYPE_NO_IMAGE),
        m_bitmap(),
        m_gifPlayer(),
        m_hAlign(Alignment::Horizontal::HORIZONTAL_LEFT),
        m_vAlign(Alignment::Vertical::VERTICAL_TOP),
        m_hAlignPosX(0),
        m_vAlignPosY(0)
    {
    }

    /**
     * Constructs a bitmap widget by copying another one.
     *
     * @param[in] widget Bitmap widge, which to copy
     */
    BitmapWidget(const BitmapWidget& widget) :
        Widget(widget),
        m_imgType(widget.m_imgType),
        m_bitmap(widget.m_bitmap),
        m_gifPlayer(widget.m_gifPlayer),
        m_hAlign(widget.m_hAlign),
        m_vAlign(widget.m_vAlign),
        m_hAlignPosX(widget.m_hAlignPosX),
        m_vAlignPosY(widget.m_vAlignPosY)
    {
    }

    /**
     * Destroys the bitmap widget.
     */
    virtual ~BitmapWidget()
    {
    }

    /**
     * Assigns a existing bitmap widget.
     *
     * @param[in] widget Bitmap widge, which to assign
     * 
     * @return Bitmap widget
     */
    BitmapWidget& operator=(const BitmapWidget& widget);

    /**
     * Set widget width.
     * 
     * @param[in] width Width in pixel
     */
    void setWidth(uint16_t width) override
    {
        Widget::setWidth(width);
        alignWidget();
    }

    /**
     * Set widget height.
     * 
     * @param[in] height Height in pixel
     */
    void setHeight(uint16_t height) override
    {
        Widget::setHeight(height);
        alignWidget();
    }

    /**
     * Get the bitmap.
     *
     * @return Bitmap
     */
    const YAGfxBitmap& get() const
    {
        return m_bitmap;
    }

    /**
     * Set a bitmap.
     * 
     * The canvas width and height won't be updated. If required, update them
     * explicit.
     * 
     * @param[in] bitmap    Bitmap
     */
    void set(const YAGfxBitmap& bitmap);

    /**
     * Clear the image.
     * 
     * @param[in] color Color used for clearing.
     */
    void clear(const Color& color);

    /**
     * Load image from filesystem.
     * 
     * The canvas width and height won't be updated. If required, update them
     * explicit.
     *
     * @param[in] fs        Filesystem
     * @param[in] filename  Filename with full path
     *
     * @return If successful loaded it will return true otherwise false.
     */
    bool load(FS& fs, const String& filename);

    /**
     * Set the horizontal alignment.
     * 
     * @param[in] align The horizontal aligment.
     */
    void setHorizontalAlignment(Alignment::Horizontal align)
    {
        m_hAlign = align;
        alignWidget();
    }

    /**
     * Set the vertical alignment.
     * 
     * @param[in] align The vertical aligment.
     */
    void setVerticalAlignment(Alignment::Vertical align)
    {
        m_vAlign = align;
        alignWidget();
    }

    /**
     * Is bitmap widget empty, means no image is shown?
     * 
     * @return If empty, it will return true otherwise false.
     */
    bool isEmpty() const
    {
        return IMG_TYPE_NO_IMAGE == m_imgType;
    }

    /** Widget type string */
    static const char*  WIDGET_TYPE;

    /**
     * Filename extension of bitmap image file.
     */
    static const char*  FILE_EXT_BITMAP;

    /**
     * Filename extension of GIF image file.
     */
    static const char*  FILE_EXT_GIF;

private:

    /**
     * Supported image types.
     */
    enum ImgType
    {
        IMG_TYPE_NO_IMAGE = 0,  /**< No image */
        IMG_TYPE_BMP,           /**< BMP image */
        IMG_TYPE_GIF            /**< GIF image */
    };

    ImgType                 m_imgType;      /**< Current image type. */
    YAGfxDynamicBitmap      m_bitmap;       /**< Bitmap image. */
    GifImgPlayer            m_gifPlayer;    /**< GIF image player. */
    Alignment::Horizontal   m_hAlign;       /**< Horizontal alignment. */
    Alignment::Vertical     m_vAlign;       /**< Vertical alignment. */
    int16_t                 m_hAlignPosX;   /**< x-coordinate derived from horizontal alignment. */
    int16_t                 m_vAlignPosY;   /**< y-coordinate derived from vertical alignment. */

    /**
     * Paint the widget with the given graphics interface.
     * 
     * @param[in] gfx   Graphics interface
     */
    void paint(YAGfx& gfx) override
    {
        if (IMG_TYPE_BMP == m_imgType)
        {
            gfx.drawBitmap(m_hAlignPosX, m_vAlignPosY, m_bitmap);
        }
        else if (IMG_TYPE_GIF == m_imgType)
        {
            (void)m_gifPlayer.play(gfx, m_hAlignPosX, m_vAlignPosY);
        }
        else
        {
            ;
        }
    }

    /**
     * Align the widget dependend on the bitmap size.
     * It will adapt the m_hAlignPosX and m_vAlignPosY.
     */
    void alignWidget();

    /**
     * Load BMP image from filesystem.
     *
     * @param[in] fs        Filesystem
     * @param[in] filename  Filename with full path
     *
     * @return If successful loaded it will return true otherwise false.
     */
    bool loadBMP(FS& fs, const String& filename);

    /**
     * Load GIF image from filesystem.
     *
     * @param[in] fs        Filesystem
     * @param[in] filename  Filename with full path
     *
     * @return If successful loaded it will return true otherwise false.
     */
    bool loadGIF(FS& fs, const String& filename);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* BITMAPWIDGET_H */

/** @} */
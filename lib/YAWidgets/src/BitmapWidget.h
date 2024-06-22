/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
        m_gifPlayer()
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
        m_gifPlayer(widget.m_gifPlayer)
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
     */
    BitmapWidget& operator=(const BitmapWidget& widget);

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
    void set(const YAGfxBitmap& bitmap)
    {
        /* Release unused memory. */
        m_bitmap.release();
        m_gifPlayer.close();

        if (true == m_bitmap.create(bitmap.getWidth(), bitmap.getHeight()))
        {
            m_bitmap.copy(bitmap);
            m_imgType = IMG_TYPE_BMP;
        }
    }

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

    /** Widget type string */
    static const char* WIDGET_TYPE;

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

    ImgType             m_imgType;      /**< Current image type. */
    YAGfxDynamicBitmap  m_bitmap;       /**< Bitmap image. */
    GifImgPlayer        m_gifPlayer;    /**< GIF image player. */

    /**
     * Paint the widget with the given graphics interface.
     * 
     * @param[in] gfx   Graphics interface
     */
    void paint(YAGfx& gfx) override
    {
        if (IMG_TYPE_BMP == m_imgType)
        {
            gfx.drawBitmap(0, 0, m_bitmap);
        }
        else if (IMG_TYPE_GIF == m_imgType)
        {
            (void)m_gifPlayer.play(gfx);
        }
        else
        {
            ;
        }
    }

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
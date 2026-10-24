/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   BitmapWidget.h
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
#include "GifFileLoader.h"
#include "GifFileToMemLoader.h"
#include "GifImgPlayer.h"
#include "Alignment.h"

/* The WebP image player is only available if the build switch is enabled. Its
 * header pulls in the libwebp headers, therefore the include must stay inside
 * the guard.
 */
#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)
#include "WebpImgPlayer.h"
#endif /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */

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
 * - PNG image (.png), only if CONFIG_IMAGE_FORMAT_PNG_ENABLE is enabled.
 * - WebP image (.webp), only if CONFIG_IMAGE_FORMAT_WEBP_ENABLE is enabled.
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
        m_vAlignPosY(0),
        m_gifFileLoader()
#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)
        ,
        m_webpPlayer()
#endif /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */
    {
        /* Animated GIF images shall run infinite by default. */
        m_gifPlayer.setInfiniteAnimation(true);

#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)
        /* Animated WebP images shall run infinite by default, same as GIF. */
        m_webpPlayer.setInfiniteAnimation(true);
#endif /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */
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
        m_vAlignPosY(widget.m_vAlignPosY),
        m_gifFileLoader(widget.m_gifFileLoader)
#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)
        ,
        m_webpPlayer(widget.m_webpPlayer)
#endif /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */
    {
    }

    /**
     * Destroys the bitmap widget.
     */
    ~BitmapWidget() override
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
     * @param[in] filename  Filename with full path
     * @param[in] fs        Filesystem instance
     *
     * @return If successful loaded it will return true otherwise false.
     */
    bool load(const String& filename, FS& fs);

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

    /**
     * Check, if the image type is supported.
     *
     * @param[in] path  Image file path
     *
     * @return If image type is supported, it will return true otherwise false.
     */
    static bool isImageTypeSupported(const String& path);

    /** Widget type string */
    static const char* WIDGET_TYPE;

    /**
     * Filename extension of bitmap image file.
     */
    static const char* FILE_EXT_BITMAP;

    /**
     * Filename extension of GIF image file.
     */
    static const char* FILE_EXT_GIF;

#if (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE)

    /**
     * Filename extension of PNG image file.
     */
    static const char* FILE_EXT_PNG;

#endif /* (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE) */

#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)

    /**
     * Filename extension of WebP image file.
     */
    static const char* FILE_EXT_WEBP;

#endif /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */

    /**
     * Supported image file extensions.
     */
    static const char* IMAGE_FILE_EXTENSIONS[];

    /**
     * Number of supported image file extensions.
     */
    static const size_t IMAGE_FILE_EXTENSIONS_COUNT;

private:

    /**
     * Supported image types.
     */
    enum ImgType
    {
        IMG_TYPE_NO_IMAGE = 0, /**< No image */
        IMG_TYPE_BMP,          /**< BMP image */
        IMG_TYPE_GIF,          /**< GIF image */
#if (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE)
        IMG_TYPE_PNG, /**< PNG image */
#endif                /* (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE) */
#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)
        IMG_TYPE_WEBP, /**< WebP image */
#endif                 /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */
    };

    ImgType               m_imgType;    /**< Current image type. */
    YAGfxDynamicBitmap    m_bitmap;     /**< Bitmap image. */
    GifImgPlayer          m_gifPlayer;  /**< GIF image player. */
    Alignment::Horizontal m_hAlign;     /**< Horizontal alignment. */
    Alignment::Vertical   m_vAlign;     /**< Vertical alignment. */
    int16_t               m_hAlignPosX; /**< x-coordinate derived from horizontal alignment. */
    int16_t               m_vAlignPosY; /**< y-coordinate derived from vertical alignment. */

#ifdef CONFIG_ENABLE_FILE_TO_MEM_LOADER
    GifFileToMemLoader m_gifFileLoader; /**< GIF file loader used to read the file from memory. */
#else                                   /* CONFIG_ENABLE_FILE_TO_MEM_LOADER */
    GifFileLoader m_gifFileLoader; /**< GIF file loader used to read the file from filesystem. */
#endif                                  /* CONFIG_ENABLE_FILE_TO_MEM_LOADER */

#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)
    WebpImgPlayer m_webpPlayer; /**< WebP image player. */
#endif                          /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */

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
#if (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE)
        /* A PNG image is decoded into the same bitmap as a BMP image. */
        else if (IMG_TYPE_PNG == m_imgType)
        {
            gfx.drawBitmap(m_hAlignPosX, m_vAlignPosY, m_bitmap);
        }
#endif /* (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE) */
        else if (IMG_TYPE_GIF == m_imgType)
        {
            (void)m_gifPlayer.play(gfx, m_hAlignPosX, m_vAlignPosY);
        }
#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)
        else if (IMG_TYPE_WEBP == m_imgType)
        {
            (void)m_webpPlayer.play(gfx, m_hAlignPosX, m_vAlignPosY);
        }
#endif /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */
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

#if (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE)

    /**
     * Load PNG image from filesystem.
     *
     * @param[in] fs        Filesystem
     * @param[in] filename  Filename with full path
     *
     * @return If successful loaded it will return true otherwise false.
     */
    bool loadPNG(FS& fs, const String& filename);

#endif /* (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE) */

#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)

    /**
     * Load WebP image from filesystem.
     *
     * @param[in] fs        Filesystem
     * @param[in] filename  Filename with full path
     *
     * @return If successful loaded it will return true otherwise false.
     */
    bool loadWEBP(FS& fs, const String& filename);

#endif /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */

    /**
     * Release the memory which the WebP image player holds.
     *
     * Without WebP support there is no player and nothing is released.
     */
    void releaseWebp()
    {
#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)
        m_webpPlayer.close();
#endif /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* BITMAPWIDGET_H */

/** @} */
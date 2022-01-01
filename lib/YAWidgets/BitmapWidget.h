/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle Merkle <web@blue-andi.de>
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
#include <FS.h>
#include <SimpleTimer.hpp>

#include "Widget.hpp"
#include "BmpImg.h"
#include "SpriteSheet.h"

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
        m_image(),
        m_spriteSheet(),
        m_timer(),
        m_duration(0U)
    {
    }

    /**
     * Constructs a bitmap widget by copying another one.
     *
     * @param[in] widget Bitmap widge, which to copy
     */
    BitmapWidget(const BitmapWidget& widget) :
        Widget(WIDGET_TYPE),
        m_image(widget.m_image),
        m_spriteSheet(widget.m_spriteSheet),
        m_timer(widget.m_timer),
        m_duration(widget.m_duration)
    {
    }

    /**
     * Destroys the bitmap widget.
     */
    ~BitmapWidget()
    {
    }

    /**
     * Assigns a existing bitmap widget.
     *
     * @param[in] widget Bitmap widge, which to assign
     */
    BitmapWidget& operator=(const BitmapWidget& widget);

    /**
     * Set a new bitmap by copying the ext. bitmap buffer.
     *
     * @param[in] bitmap    Ext. bitmap buffer
     * @param[in] width     Bitmap width in pixel
     * @param[in] height    Bitmap height in pixel
     */
    void set(const Color* bitmap, uint16_t width, uint16_t height);

    /**
     * Get the bitmap buffer.
     *
     * @param[out] width    Bitmap width in pixel
     * @param[out] height   Bitmap height in pixel
     *
     * @return Bitmap buffer
     */
    const Color* get(uint16_t& width, uint16_t& height) const
    {
        const Color*    buffer = nullptr;

        /* Bitmap image loaded or sprite sheet? */
        if (false == m_image.isEmpty())
        {
            width   = m_image.getWidth();
            height  = m_image.getHeight();
            buffer  = m_image.get();
        }
        else
        {
            width   = m_spriteSheet.getFrameWidth();
            height  = m_spriteSheet.getFrameHeight();
            buffer  = m_spriteSheet.getFrame();
        }

        return buffer;
    }

    /**
     * Load bitmap image from filesystem.
     * If a sprite sheet is active, it will be disabled.
     *
     * @param[in] fs        Filesystem
     * @param[in] filename  Filename with full path
     *
     * @return If successful loaded it will return true otherwise false.
     */
    bool load(FS& fs, const String& filename);

    /**
     * Load sprite sheet file (.sprite) from filesystem.
     *
     * @param[in] fs                    Filesystem
     * @param[in] spriteSheetFileName   Name of the sprite sheet file in the filesystem
     * @param[in] textureFileName       Name of the texture image file in the filesystem
     *
     * @return If successful loaded it will return true otherwise false.
     */
    bool loadSpriteSheet(FS& fs, const String& spriteSheetFileName, const String& textureFileName);

    /** Widget type string */
    static const char* WIDGET_TYPE;

private:

    BmpImg      m_image;        /**< The bitmap image may contain one single image or a texture for the sprite sheet. */
    SpriteSheet m_spriteSheet;  /**< Sprite sheet for animation with texture. */
    SimpleTimer m_timer;        /**< Timer used for sprite sheet. */
    uint32_t    m_duration;     /**< Duration of one frame in ms. */

    /**
     * Paint the widget with the given graphics interface.
     * 
     * @param[in] gfx   Graphics interface
     */
    void paint(YAGfx& gfx) override
    {
        if (false == m_image.isEmpty())
        {
            gfx.drawBitmap(m_posX, m_posY, m_image.get(), m_image.getWidth(), m_image.getHeight());
        }
        else if (false == m_spriteSheet.isEmpty())
        {
            gfx.drawBitmap(m_posX, m_posY, m_spriteSheet.getFrame(), m_spriteSheet.getFrameWidth(), m_spriteSheet.getFrameHeight());

            /* If timer is not running, start it. */
            if (false == m_timer.isTimerRunning())
            {
                m_timer.start(m_duration);
            }
            /* If the timer has a timeout, select next sprite and restart timer. */
            else if (true == m_timer.isTimeout())
            {
                m_spriteSheet.next();
                m_timer.start(m_duration);
            }
            else
            {
                /* Nothing to do. */
                ;
            }
        }
        else
        {
            m_timer.stop();
        }

        return;
    }
    
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __BITMAPWIDGET_H__ */

/** @} */
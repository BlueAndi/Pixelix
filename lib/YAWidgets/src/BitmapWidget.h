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
#include <SimpleTimer.hpp>

#include "Widget.hpp"
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
        m_bitmap(),
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
        m_bitmap(widget.m_bitmap),
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
     * Set a bitmap.
     *
     * @param[in] bitmap    Bitmap
     */
    void set(const YAGfxBitmap& bitmap)
    {
        if (true == m_bitmap.create(bitmap.getWidth(), bitmap.getHeight()))
        {
            m_bitmap.copy(bitmap);
        }

        /* Release sprite sheet to avoid wasting memory. The widget can
         * only show one of them.
         */
        m_spriteSheet.release();
        m_timer.stop();
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
     * Clear the bitmap.
     * 
     * @param[in] color Color used for clearing.
     */
    void clear(const Color& color);

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

    /**
     * Get the animation control flag FORWARD of a sprite sheet.
     * 
     * @return If forward, it will return true otherwise false.
     */
    bool isSpriteSheetForward() const;

    /**
     * Set the animation control flag FORWARD of a sprite sheet.
     * 
     * @param[in] forward The state to be set.
     */
    void setSpriteSheetForward(bool forward);

    /**
     * Get the animation control flag REPEAT of a sprite sheet.
     * 
     * @return If its repeated, it will return true otherwise false.
     */
    bool isSpriteSheetRepeatInfinite() const;

    /**
     * Set the animation control flag REPEAT of a sprite sheet.
     * 
     * @param[in] repeat The repeat flat to be set.
     */
    void setSpriteSheetRepeatInfinite(bool repeat);
    
    /** Widget type string */
    static const char* WIDGET_TYPE;

private:

    YAGfxDynamicBitmap  m_bitmap;       /**< Bitmap image which is shown if no sprite sheet is loaded. */
    SpriteSheet         m_spriteSheet;  /**< Sprite sheet for animation with texture. */
    SimpleTimer         m_timer;        /**< Timer used for sprite sheet. */
    uint32_t            m_duration;     /**< Duration of one frame in ms. */

    /**
     * Paint the widget with the given graphics interface.
     * 
     * @param[in] gfx   Graphics interface
     */
    void paint(YAGfx& gfx) override
    {
        if (true == m_spriteSheet.isEmpty())
        {
            gfx.drawBitmap(m_posX, m_posY, m_bitmap);
        }
        else
        {
            gfx.drawBitmap(m_posX, m_posY, m_spriteSheet.getFrame());

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
    }
    
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* BITMAPWIDGET_H */

/** @} */
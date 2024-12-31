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
 * @brief  Generic view with multiple icons for LED matrix
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef MULTI_ICON_VIEW_GENERIC_H
#define MULTI_ICON_VIEW_GENERIC_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <YAGfx.h>
#include <IMultiIconView.h>
#include <BitmapWidget.h>
#include <Util.h>
#include <FileSystem.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Generic view for LED matrix with multiple icons.
 */
class MultiIconViewGeneric : public IMultiIconView
{
public:

    /**
     * Construct the view.
     */
    MultiIconViewGeneric() :
        IMultiIconView(),
        m_bitmapWidgets{
            {BITMAP_WIDTH, BITMAP_HEIGHT, BITMAP_0_X, BITMAP_Y},
            {BITMAP_WIDTH, BITMAP_HEIGHT, BITMAP_1_X, BITMAP_Y},
            {BITMAP_WIDTH, BITMAP_HEIGHT, BITMAP_2_X, BITMAP_Y}
        }
    {
        uint8_t slot = 0U;

        while(MAX_ICON_SLOTS > slot)
        {
            m_bitmapWidgets[slot].setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_CENTER);
            m_bitmapWidgets[slot].setVerticalAlignment(Alignment::Vertical::VERTICAL_CENTER);

            ++slot;
        }
    }

    /**
     * Destroy the view.
     */
    virtual ~MultiIconViewGeneric()
    {
    }

    /**
     * Initialize view, which will prepare the widgets and the default values.
     */
    void init(uint16_t width, uint16_t height) override
    {
        UTIL_NOT_USED(width);
        UTIL_NOT_USED(height);
    }

    /**
     * Update the underlying canvas.
     * 
     * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
     */
    void update(YAGfx& gfx) override
    {
        uint8_t idx = 0U;

        gfx.fillScreen(ColorDef::BLACK);

        while(MAX_ICON_SLOTS > idx)
        {
            m_bitmapWidgets[idx].update(gfx);
            ++idx;
        }
    }

    /**
     * Load icon image from filesystem and show in the slot with the given id.
     *
     * @param[in] slotId    The id of the slot.
     * @param[in] filename  Image filename
     *
     * @return If successul, it will return true otherwise false.
     */
    bool loadIcon(uint8_t slotId, const String& filename) override
    {
        if (MAX_ICON_SLOTS <= slotId)
        {
            slotId = 0U;
        }

        return m_bitmapWidgets[slotId].load(FILESYSTEM, filename);
    }

    /**
     * Clear icon in the slot with the given id.
     * 
     * @param[in] slotId    The id of the slot.
     */
    void clearIcon(uint8_t slotId) override
    {
        if (MAX_ICON_SLOTS <= slotId)
        {
            slotId = 0U;
        }

        m_bitmapWidgets[slotId].clear(ColorDef::BLACK);
    }

    /**
     * Max. number of icons.
     */
    static const uint8_t    MAX_ICON_SLOTS  = 3U;

protected:

    /**
     * Bitmap width in pixels.
     */
    static const uint16_t   BITMAP_WIDTH    = 8U;

    /**
     * Bitmap height in pixels.
     */
    static const uint16_t   BITMAP_HEIGHT   = 8U;

    /** Distance between two bitmaps in pixel. */
    static const uint8_t    BITMAP_DISTANCE = (CONFIG_LED_MATRIX_WIDTH - (MAX_ICON_SLOTS * BITMAP_WIDTH)) / MAX_ICON_SLOTS;

    /** Bitmap 0 x-coordinate in pixel. */
    static const uint8_t    BITMAP_0_X      = 0U * (BITMAP_WIDTH + BITMAP_DISTANCE);

    /** Bitmap 1 x-coordinate in pixel. */
    static const uint8_t    BITMAP_1_X      = 1U * (BITMAP_WIDTH + BITMAP_DISTANCE);

    /** Bitmap 2 x-coordinate in pixel. */
    static const uint8_t    BITMAP_2_X      = 2U * (BITMAP_WIDTH + BITMAP_DISTANCE);

    /** Bitmap y-coordindate in pixel. */
    static const uint8_t    BITMAP_Y        = 0U;

    BitmapWidget    m_bitmapWidgets[MAX_ICON_SLOTS]; /**< Bitmap widgets used to show the icons. */

    MultiIconViewGeneric(const MultiIconViewGeneric& other);
    MultiIconViewGeneric& operator=(const MultiIconViewGeneric& other);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* MULTI_ICON_VIEW_GENERIC_H */

/** @} */

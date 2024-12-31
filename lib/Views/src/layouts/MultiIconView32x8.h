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
 * @brief  View with multiple icons for 32x8 LED matrix
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef MULTI_ICON_VIEW_32X8_H
#define MULTI_ICON_VIEW_32X8_H

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
 * View for 32x8 LED matrix with multiple icons.
 */
class MultiIconView32x8 : public IMultiIconView
{
public:

    /**
     * Construct the view.
     */
    MultiIconView32x8() :
        IMultiIconView(),
        m_bitmapWidgets{
            { 0U, 0U, 0, 0 },
            { 0U, 0U, 0, 0 },
            { 0U, 0U, 0, 0 },
            { 0U, 0U, 0, 0 }
        }
    {
        uint8_t slot = 0U;

        while (MAX_ICON_SLOTS > slot)
        {
            m_bitmapWidgets[slot].setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_CENTER);
            m_bitmapWidgets[slot].setVerticalAlignment(Alignment::Vertical::VERTICAL_CENTER);

            ++slot;
        }
    }

    /**
     * Destroy the view.
     */
    virtual ~MultiIconView32x8()
    {
    }

    /**
     * Initialize view, which will prepare the widgets and the default values.
     * 
     * @param[in] width     Display width in pixel.
     * @param[in] height    Display height in pixel.
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

        while (MAX_ICON_SLOTS > idx)
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
        bool isSuccessful = false;

        if (MAX_ICON_SLOTS <= slotId)
        {
            slotId = 0U;
        }

        isSuccessful = m_bitmapWidgets[slotId].load(FILESYSTEM, filename);

        if (true == isSuccessful)
        {
            reorder();
        }

        return isSuccessful;
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
        reorder();
    }

    /**
     * Max. number of icons.
     */
    static const uint8_t MAX_ICON_SLOTS = 4U;

protected:

    BitmapWidget m_bitmapWidgets[MAX_ICON_SLOTS]; /**< Bitmap widgets used to show the icons. */

private:
    MultiIconView32x8(const MultiIconView32x8& other);
    MultiIconView32x8& operator=(const MultiIconView32x8& other);

    /**
     * Get the active number of icon slosts.
     *
     * @return Number of active icon slots
     */
    uint8_t getActiveIconSlots();

    /**
     * Re-order the icons, depended on the number of active icon slots.
     */
    void reorder();

    /**
     * Apply layout with four icons.
     *
     * @param[in] widgetCnt Number of active icons.
     */
    void applyLayout(uint8_t widgetCnt);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* MULTI_ICON_VIEW_32X8_H */

/** @} */

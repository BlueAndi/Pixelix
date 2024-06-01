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
 * @brief  Plugin view for 32x8 LED matrix
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup plugin
 *
 * @{
 */

#ifndef THREE_ICON_PLUGIN_VIEW_32X8_H
#define THREE_ICON_PLUGIN_VIEW_32X8_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <YAGfx.h>
#include <BitmapWidget.h>
#include <Util.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Internal plugin functionality. */
namespace _ThreeIconPlugin
{

/**
 * View for 32x8 LED matrix.
 */
class View
{
public:

    /**
     * Construct the layout.
     */
    View() :
        m_bitmapWidgets{
            {BITMAP_WIDTH, BITMAP_HEIGHT, BITMAP_0_X, BITMAP_Y},
            {BITMAP_WIDTH, BITMAP_HEIGHT, BITMAP_1_X, BITMAP_Y},
            {BITMAP_WIDTH, BITMAP_HEIGHT, BITMAP_2_X, BITMAP_Y}
        }
    {
    }

    /**
     * Destroy the layout.
     */
    ~View()
    {
    }

    /**
     * Initialize view, which will prepare the widgets and the default values.
     */
    void init(uint16_t width, uint16_t height)
    {
        UTIL_NOT_USED(width);
        UTIL_NOT_USED(height);
    }

    /**
     * Update the underlying canvas.
     * 
     * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
     */
    void update(YAGfx& gfx)
    {
        uint8_t idx = 0U;

        gfx.fillScreen(ColorDef::BLACK);

        while(MAX_ICONS > idx)
        {
            m_bitmapWidgets[idx].update(gfx);
            ++idx;
        }
    }

    /**
     * Get bitmap widget by icon id.
     * 
     * @param[in] iconId    The icon id of the slot.
     * 
     * @return Bitmap widget
     */
    const BitmapWidget& getBitmapWidget(uint8_t iconId) const
    {
        if (MAX_ICONS <= iconId)
        {
            iconId = 0U;
        }

        return m_bitmapWidgets[iconId];
    }

    /**
     * Get bitmap widget by icon id.
     * 
     * @param[in] iconId    The icon id of the slot.
     * 
     * @return Bitmap widget
     */
    BitmapWidget& getBitmapWidget(uint8_t iconId)
    {
        if (MAX_ICONS <= iconId)
        {
            iconId = 0U;
        }

        return m_bitmapWidgets[iconId];
    }

    /**
     * Max. number of icons.
     */
    static const uint8_t    MAX_ICONS       = 3U;

private:

    /**
     * Bitmap width in pixels.
     */
    static const uint16_t   BITMAP_WIDTH    = CONFIG_LED_MATRIX_HEIGHT;

    /**
     * Bitmap height in pixels.
     */
    static const uint16_t   BITMAP_HEIGHT   = CONFIG_LED_MATRIX_HEIGHT;

    /** Distance between two bitmaps in pixel. */
    static const uint8_t    BITMAP_DISTANCE = (CONFIG_LED_MATRIX_WIDTH - (_ThreeIconPlugin::View::MAX_ICONS * BITMAP_WIDTH)) / _ThreeIconPlugin::View::MAX_ICONS;

    /** Bitmap 0 x-coordinate in pixel. */
    static const uint8_t    BITMAP_0_X      = 0U * (BITMAP_WIDTH + BITMAP_DISTANCE);

    /** Bitmap 1 x-coordinate in pixel. */
    static const uint8_t    BITMAP_1_X      = 1U * (BITMAP_WIDTH + BITMAP_DISTANCE);

    /** Bitmap 2 x-coordinate in pixel. */
    static const uint8_t    BITMAP_2_X      = 2U * (BITMAP_WIDTH + BITMAP_DISTANCE);

    /** Bitmap y-coordindate in pixel. */
    static const uint8_t    BITMAP_Y        = 0U;

    BitmapWidget    m_bitmapWidgets[MAX_ICONS]; /**< Bitmap widgets used to show the icons. */

    View(const View& other);
    View& operator=(const View& other);
};

} /* _ThreeIconPlugin */

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* THREE_ICON_PLUGIN_VIEW_32X8_H */

/** @} */

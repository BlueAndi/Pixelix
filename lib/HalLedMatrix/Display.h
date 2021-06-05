/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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
 * @brief  LED matrix display
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IDisplay.hpp>
#include <NeoPixelBrightnessBus.h>
#include <ColorDef.hpp>

#include "Board.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This display represents a LED matrix of 32x8 NeoPixels (WS2812B).
 */
class Display : public IDisplay
{
public:

    /**
     * Get display instance.
     *
     * @return Display
     */
    static Display& getInstance()
    {
        static Display instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
    }

    /**
     * Initialize base driver for the display.
     *
     * @return If successful, returns true otherwise false.
     */
    bool begin() final
    {
        m_strip.Begin();
        m_strip.Show();

        return true;
    }

    /**
     * Show framebuffer on physical display. This may be synchronous
     * or asynchronous.
     */
    void show() final
    {
        m_strip.Show();
        return;
    }

    /**
     * The display is ready, when the last physical pixel update is finished.
     * A asynchronous display update, triggered by show() can be observed this way.
     *
     * @return If ready for another update via show(), it will return true otherwise false.
     */
    bool isReady() const final
    {
        return m_strip.CanShow();
    }

    /**
     * Set brightness from 0 to 255.
     *
     * @param[in] brightness    Brightness value [0; 255]
     */
    void setBrightness(uint8_t brightness) final
    {
        /* To protect the the electronic parts, the brigntness will be scaled down
         * according to the max. supply current.
         */
        const uint8_t SAFE_BRIGHTNESS =
            (Board::LedMatrix::supplyCurrentMax * brightness) /
            (Board::LedMatrix::maxCurrentPerLed * Board::LedMatrix::width *Board::LedMatrix::height);

        m_strip.SetBrightness(SAFE_BRIGHTNESS);
        return;
    }

    /**
     * Clear display.
     */
    void clear() final
    {
        m_strip.ClearTo(ColorDef::BLACK);
        return;
    }

    /**
     * Get pixel color at given position.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color in RGB888 format.
     */
    Color getColor(int16_t x, int16_t y) const final;

private:

    /** Pixel representation of the LED matrix */
    NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod>  m_strip;

    /** Panel topology, used to map coordinates to the framebuffer. */
    NeoTopology<ColumnMajorAlternatingLayout>               m_topo;

    /**
     * Construct display.
     */
    Display();

    /**
     * Destroys display.
     */
    ~Display();

    Display(const Display& display);
    Display& operator=(const Display& display);

    /**
     * Draw a single pixel on the display.
     *
     * @param[in] x     x-coordinate
     * @param[in] y     y-coordinate
     * @param[in] color Pixel color in RGB888 format
     */
    void drawPixel(int16_t x, int16_t y, const Color& color) final
    {
        if ((0 <= x) &&
            (Board::LedMatrix::width > x) &&
            (0 <= y) &&
            (Board::LedMatrix::height > y))
        {
            HtmlColor htmlColor = static_cast<uint32_t>(color);

            m_strip.SetPixelColor(m_topo.Map(x, y), htmlColor);
        }

        return;
    }

    /**
     * Dim color to black.
     * A dim ratio of 0 means no change.
     * 
     * Note, the base colors may be destroyed, depends on the color type.
     *
     * @param[in] x     x-coordinate
     * @param[in] y     y-coordinate
     * @param[in] ratio Dim ratio [0; 255]
     */
    void dimPixel(int16_t x, int16_t y, uint8_t ratio) final
    {
        if ((0 <= x) &&
            (Board::LedMatrix::width > x) &&
            (0 <= y) &&
            (Board::LedMatrix::height > y))
        {
            RgbColor rgbColor = m_strip.GetPixelColor(m_topo.Map(x, y)).Dim(UINT8_MAX - ratio);

            m_strip.SetPixelColor(m_topo.Map(x, y), rgbColor);
        }

        return;
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __DISPLAY_H__ */

/** @} */
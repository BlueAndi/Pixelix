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
 * @brief  LED matrix
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup gfx
 *
 * @{
 */

#ifndef __LEDMATRIX_H__
#define __LEDMATRIX_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <Adafruit_GFX.h>
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
 * Specific LED matrix.
 */
class LedMatrix : public Adafruit_GFX
{
public:

    /**
     * Get LED matrix instance.
     * 
     * @return LED matrix
     */
    static LedMatrix& getInstance(void)
    {
        return m_instance;
    }

    /**
     * Initialize base driver for the LED matrix.
     */
    void begin(void)
    {
        m_strip.Begin();
        m_strip.Show();
        return;
    }

    /**
     * Show internal framebuffer on physical LED matrix.
     */
    void show(void)
    {
        m_strip.Show();
        return;
    }

    /**
     * Set brightness from 0 to 255.
     *
     * @param[in] brightness    Brightness value [0; 255]
     */
    void setBrightness(uint8_t brightness)
    {
        m_strip.SetBrightness(brightness);
        return;
    }

    /**
     * Clear LED matrix.
     */
    void clear(void)
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
    uint32_t getColor(int16_t x, int16_t y);

private:

    /** LedMatrix instance */
    static LedMatrix    m_instance;

    /** Pixel representation of the LED matrix */
    NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod>  m_strip;

    /** Panel topology, used to map coordinates to the framebuffer. */
    NeoTopology<ColumnMajorAlternatingLayout>               m_topo;

    /**
     * Construct LED matrix.
     */
    LedMatrix();

    /**
     * Destroys LED matrix.
     */
    ~LedMatrix();

    LedMatrix(const LedMatrix& matrix);
    LedMatrix& operator=(const LedMatrix& matrix);

    /**
     * Draw a single pixel in the matrix.
     * 
     * @param[in] x     x-coordinate
     * @param[in] y     y-coordinate
     * @param[in] color Pixel color in RGB565 format
     */
    void drawPixel(int16_t x, int16_t y, uint16_t color)
    {
        if ((0 <= x) &&
            (Board::LedMatrix::width > x) &&
            (0 <= y) &&
            (Board::LedMatrix::height > y))
        {
            HtmlColor colorRGB888 = ColorDef::convert565To888(color);

            m_strip.SetPixelColor(m_topo.Map(x, y), colorRGB888);
        }

        return;
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __LEDMATRIX_H__ */

/** @} */
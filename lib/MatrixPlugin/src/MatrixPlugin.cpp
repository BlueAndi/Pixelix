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
 * @brief  Matrix demo plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MatrixPlugin.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void MatrixPlugin::active(YAGfx& gfx)
{
    /* Clear display */
    gfx.fillScreen(ColorDef::BLACK);
}

void MatrixPlugin::inactive()
{
    /* Nothing to do. */
}

void MatrixPlugin::update(YAGfx& gfx)
{
    if ((false == m_timer.isTimerRunning()) ||
        (true == m_timer.isTimeout()))
    {
        const Color     CODE_COLOR(175U, 255U, 175U);
        const Color     TRAIL_COLOR(27U, 130U, 39U);
        const uint16_t  SCALE_FACTOR_NUMERATOR      = 192U;
        const uint16_t  SCALE_FACTOR_DENOMINATOR    = 256U;
        int16_t         x       = 0;
        int16_t         y       = 0;
        Color           color;
        uint8_t         red     = 0U;
        uint8_t         green   = 0U;
        uint8_t         blue    = 0U;

        /* Move "matrix code" one pixel row down (higher y value) and fade each
         * pixel a little more to dark to achieve a color trail.
         */
        for(y = gfx.getHeight() - 1; y > 0; --y)
        {
            for(x = 0; x < gfx.getWidth(); ++x)
            {
                /* Get pixel from one row above. */
                color = gfx.getColor(x, y - 1);

                /* If the pixel has the code color, change to first trail color. */
                if (CODE_COLOR == color)
                {
                    color = TRAIL_COLOR;
                }

                /* Fade color (destructive) to dark for the trail effect. */
                color.get(red, green, blue);
                red = static_cast<uint16_t>(red) * SCALE_FACTOR_NUMERATOR / SCALE_FACTOR_DENOMINATOR;
                green = static_cast<uint16_t>(green) * SCALE_FACTOR_NUMERATOR / SCALE_FACTOR_DENOMINATOR;
                blue = static_cast<uint16_t>(blue) * SCALE_FACTOR_NUMERATOR / SCALE_FACTOR_DENOMINATOR;
                color.set(red, green, blue);

                /* Draw pixel at current position. */
                gfx.drawPixel(x, y, color);
            }
        }

        /* The lowest row is handled separately, because the code color must
         * move one row down (higher y value) for the lightning effect.
         */
        for(x = 0; x < gfx.getWidth(); ++x)
        {
            /* Get Pixel */
            color = gfx.getColor(x, 0);

            /* Create color trail and lightning effect. */
            if (CODE_COLOR == color)
            {
                color = TRAIL_COLOR;

                gfx.drawPixel(x, y + 1, CODE_COLOR);
            }

            /* Fade color (destructive) to dark. */
            color.get(red, green, blue);
            red = static_cast<uint16_t>(red) * SCALE_FACTOR_NUMERATOR / SCALE_FACTOR_DENOMINATOR;
            green = static_cast<uint16_t>(green) * SCALE_FACTOR_NUMERATOR / SCALE_FACTOR_DENOMINATOR;
            blue = static_cast<uint16_t>(blue) * SCALE_FACTOR_NUMERATOR / SCALE_FACTOR_DENOMINATOR;
            color.set(red, green, blue);

            gfx.drawPixel(x, y, color);
        }

        /* Spawn new falling "matrix code". */
        if (0 == random(2))
        {
            x = random(gfx.getWidth());
            gfx.drawPixel(x, 0, CODE_COLOR);
        }

        m_timer.start(UPDATE_PERIOD);
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

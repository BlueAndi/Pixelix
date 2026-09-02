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
 * @file   DisplayDrv.cpp
 * @brief  Graphic TFT display driver
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "DisplayDrv.h"
#include "Pin.h"

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

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

DisplayDrv::DisplayDrv() :
    IDisplayDrv(),
    m_tft(),
    m_brightness(DEFAULT_BRIGHTNESS),
    m_isOn(false)
{
}

DisplayDrv::~DisplayDrv()
{
}

void DisplayDrv::show(const YAGfxBitmap& bitmap)
{
    const int16_t height = bitmap.getHeight();
    const int16_t width  = bitmap.getWidth();

    for (int16_t y = 0; y < height; ++y)
    {
        for (int16_t x = 0; x < width; ++x)
        {
#if CONFIG_DISPLAY_ROTATE180 != 0
            Color brightnessAdjustedColor = bitmap.getColor(width - x - 1, height - y - 1);
#else
            Color brightnessAdjustedColor = bitmap.getColor(x, y);
#endif
            const int32_t xNative = y * (PIXEL_HEIGHT + PIXEL_DISTANCE) + BORDER_Y;
            const int32_t yNative = TFT_HEIGHT - (x * (PIXEL_WIDTH + PIXEL_DISTANCE) + BORDER_X) - 1;

            /* Apply the display brightness on the base color, as it is no longer stored as a per-pixel intensity. */
            brightnessAdjustedColor.dim(m_brightness);

            m_tft.fillRect(
                xNative,
                yNative,
                PIXEL_HEIGHT,
                PIXEL_WIDTH,
                brightnessAdjustedColor.toRgb565());
        }
    }
}

void DisplayDrv::off()
{
    m_tft.writecommand(TFT_DISPOFF);

#if defined(TFT_BL) && defined(TFT_BACKLIGHT_ON)

#if (LOW == TFT_BACKLIGHT_ON)

    /* Turn off the back-light LED */
    Pin::tftBackLightOut.write(HIGH);

#else /* (LOW == TFT_BACKLIGHT_ON) */

    /* Turn off the back-light LED */
    Pin::tftBackLightOut.write(LOW);

#endif /* (LOW == TFT_BACKLIGHT_ON) */

#endif /* defined (TFT_BL) && defined (TFT_BACKLIGHT_ON) */

    m_isOn = false;
}

void DisplayDrv::on()
{
    m_tft.writecommand(TFT_DISPON);

#if defined(TFT_BL) && defined(TFT_BACKLIGHT_ON)

    /* Turn on the back-light LED */
    Pin::tftBackLightOut.write(TFT_BACKLIGHT_ON);

#endif /* defined (TFT_BL) && defined (TFT_BACKLIGHT_ON) */

    m_isOn = true;
}

bool DisplayDrv::isOn() const
{
    return m_isOn;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
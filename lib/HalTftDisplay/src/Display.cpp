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
 * @brief  Graphic TFT display
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Display.h"

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

Display::Display() :
    IDisplay(),
    m_tft(),
    m_ledMatrix(),
    m_brightness(DEFAULT_BRIGHTNESS),
    m_isOn(false)
{
}

Display::~Display()
{
}

void Display::show()
{
    int32_t x = 0;
    int32_t y = 0;

    for(y = 0; y < MATRIX_HEIGHT; ++y)
    {
        for(x = 0; x < MATRIX_WIDTH; ++x)
        {
#if CONFIG_DISPLAY_ROTATE180 != 0
            Color       brightnessAdjustedColor = m_ledMatrix.getColor(MATRIX_WIDTH - x - 1, MATRIX_HEIGHT - y - 1);
#else
            Color       brightnessAdjustedColor = m_ledMatrix.getColor(x, y);
#endif           
    	    uint16_t    intensity               = brightnessAdjustedColor.getIntensity();
            int32_t     xNative                 = y * (PIXEL_HEIGHT + PiXEL_DISTANCE) + BORDER_Y;
            int32_t     yNative                 = TFT_HEIGHT - (x * (PIXEL_WIDTH  + PiXEL_DISTANCE) + BORDER_X) - 1;

            intensity *= (static_cast<uint16_t>(m_brightness) + 1U);
            intensity /= 256U;
            brightnessAdjustedColor.setIntensity(static_cast<uint8_t>(intensity));

            m_tft.fillRect( xNative,
                            yNative,
                            PIXEL_HEIGHT,
                            PIXEL_WIDTH,
                            brightnessAdjustedColor.to565());
        }
    }
}

void Display::off()
{
    m_tft.writecommand(TFT_DISPOFF);

#if defined (TFT_BL) && defined (TFT_BACKLIGHT_ON)

#if (LOW == TFT_BACKLIGHT_ON)

    /* Turn off the back-light LED */
    Board::tftBackLightOut.write(HIGH);

#else   /* (LOW == TFT_BACKLIGHT_ON) */

    /* Turn off the back-light LED */
    Board::tftBackLightOut.write(LOW);

#endif  /* (LOW == TFT_BACKLIGHT_ON) */

#endif /* defined (TFT_BL) && defined (TFT_BACKLIGHT_ON) */

    m_isOn = false;
}

void Display::on()
{
    m_tft.writecommand(TFT_DISPON);

#if defined (TFT_BL) && defined (TFT_BACKLIGHT_ON)

    /* Turn off the back-light LED */
    Board::tftBackLightOut.write(TFT_BACKLIGHT_ON);

#endif /* defined (TFT_BL) && defined (TFT_BACKLIGHT_ON) */

    m_isOn = true;
}

bool Display::isOn() const
{
    return m_isOn;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

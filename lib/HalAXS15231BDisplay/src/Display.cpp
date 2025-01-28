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
 * @brief  Graphic TFT display T Display S3 Long
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Display.h"
#include <Arduino_GFX_Library.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

#if CONFIG_DISPLAY_ROTATE180 != 0
/** TFT rotation value, see Arduino_GFX::setRotation() */
#define TFT_ROTATION    0
#else
/** TFT rotation value, see Arduino_GFX::setRotation() */
#define TFT_ROTATION    2
#endif

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
    m_bus(TFT_QSPI_CS /* CS */, TFT_QSPI_SCK /* SCK */, TFT_QSPI_D0 /* SDIO0 */, TFT_QSPI_D1 /* SDIO1 */, TFT_QSPI_D2 /* SDIO2 */, TFT_QSPI_D3 /* SDIO3 */),
    m_g(&m_bus, TFT_QSPI_RST /* RST */, 0 /* rotation */, false /* IPS */, TFT_WIDTH, TFT_HEIGHT),
    m_gfx(TFT_WIDTH /* width */, TFT_HEIGHT /* height */, &m_g, 0 /* output_x */, 0 /* output_y */, TFT_ROTATION /* rotation */),
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
    int32_t x;
    int32_t y;

    for(y = 0; y < MATRIX_HEIGHT; ++y)
    {
        for(x = 0; x < MATRIX_WIDTH; ++x)
        {
            Color       brightnessAdjustedColor = m_ledMatrix.getColor(x, y);
            uint16_t    intensity               = brightnessAdjustedColor.getIntensity();
            int32_t     xNative                 = y * (PIXEL_HEIGHT + PIXEL_DISTANCE) + BORDER_Y;
            int32_t     yNative                 = TFT_HEIGHT - (x * (PIXEL_WIDTH  + PIXEL_DISTANCE) + BORDER_X) - 1;
            
            intensity *= (static_cast<uint16_t>(m_brightness) + 1U);
            intensity /= 256U;
            brightnessAdjustedColor.setIntensity(static_cast<uint8_t>(intensity));
            
            m_gfx.fillRect( xNative,
                            yNative, 
                            PIXEL_HEIGHT,
                            PIXEL_WIDTH, brightnessAdjustedColor.to565());
       }
    }

    m_gfx.flush();
}

void Display::off()
{
    m_gfx.displayOff(); 

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
    m_gfx.displayOn(); 

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

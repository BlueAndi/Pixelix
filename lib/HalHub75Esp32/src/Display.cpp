/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @brief  HUB75 matrix display
 * @author Mariano Dupont <marianomd@gmail.com>
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

const HUB75_I2S_CFG::i2s_pins   Display::I2S_PINS =
{
    CONFIG_HUB75_R1_PIN,
    CONFIG_HUB75_G1_PIN,
    CONFIG_HUB75_B1_PIN,
    CONFIG_HUB75_R2_PIN,
    CONFIG_HUB75_G2_PIN,
    CONFIG_HUB75_B2_PIN,
    CONFIG_HUB75_A_PIN,
    CONFIG_HUB75_B_PIN,
    CONFIG_HUB75_C_PIN,
    CONFIG_HUB75_D_PIN,
    CONFIG_HUB75_E_PIN,
    CONFIG_HUB75_LAT_PIN,
    CONFIG_HUB75_OE_PIN,
    CONFIG_HUB75_CLK_PIN
};

const HUB75_I2S_CFG             Display::MATRIX_CFG  =
{
    CONFIG_LED_MATRIX_WIDTH,            /* Panel width */
    CONFIG_LED_MATRIX_HEIGHT,           /* Panel height */
    CONFIG_HUB75_CHAIN_LENGTH,          /* Chain length */
    I2S_PINS,                           /* Pin mapping */
    CONFIG_HUB75_DRIVER,                /* Driver */
    false,                              /* Use DMA double buffer */
    HUB75_I2S_CFG::HZ_8M,               /* I2S clock speed */
    DEFAULT_LAT_BLANKING,               /* How many clock cycles to blank OE before/after LAT signal change. */
    CONFIG_HUB75_CLOCK_PHASE,           /* Clock phase */
    60U,                                /* Min. refresh/scan rate */
    CONFIG_HUB75_PIXEL_COLOR_DEPTH_BITS /* Pixel color depth bits, e.g. 8 bits means 8 bit per color, therefore 24 bit for RGB. */
};

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
    m_panel(MATRIX_CFG),
    m_ledMatrix(),
    m_isOn(true)
{
}

Display::~Display()
{
}

void Display::show()
{
    if (true == m_isOn)
    {
        int16_t y;
        int16_t x;

        for(y = 0; y < Board::LedMatrix::height; ++y)
        {
            for(x = 0; x < Board::LedMatrix::width; ++x)
            {
                Color& color = m_ledMatrix.getColor(x, y);

#if CONFIG_DISPLAY_ROTATE180 != 0
                m_panel.drawPixelRGB888(
                    Board::LedMatrix::width - x - 1, 
                    Board::LedMatrix::height - y - 1,
                    color.getRed(), color.getGreen(), color.getBlue());
#else
                m_panel.drawPixelRGB888(x, y, color.getRed(), color.getGreen(), color.getBlue());
#endif
            }
        }
    }
}

void Display::off()
{
    m_isOn = false;

    /* Simulate powered off display. */
    m_panel.fillScreen(ColorDef::BLACK);
}

void Display::on()
{
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

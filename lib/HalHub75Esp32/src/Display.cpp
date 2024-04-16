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
    R1_PIN,
    G1_PIN,
    B1_PIN,
    R2_PIN,
    G2_PIN,
    B2_PIN,
    A_PIN,
    B_PIN,
    C_PIN,
    D_PIN,
    E_PIN,
    LAT_PIN,
    OE_PIN,
    CLK_PIN
};

const HUB75_I2S_CFG             Display::MATRIX_CFG  =
{
    CONFIG_LED_MATRIX_WIDTH,    /* Panel width */
    CONFIG_LED_MATRIX_HEIGHT,   /* Panel height */
    1U,                         /* Chain length */
    I2S_PINS                    /* Pin mapping */
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

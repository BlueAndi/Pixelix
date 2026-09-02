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
 * @brief  LED matrix display driver
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

DisplayDrv::DisplayDrv() :
    IDisplayDrv(),
    m_strip(CONFIG_LED_MATRIX_WIDTH * CONFIG_LED_MATRIX_HEIGHT, PinNo::ledMatrixDataOutPinNo),
    m_topo(CONFIG_LED_MATRIX_WIDTH, CONFIG_LED_MATRIX_HEIGHT),
    m_isOn(true)
{
}

DisplayDrv::~DisplayDrv()
{
}

void DisplayDrv::show(const YAGfxBitmap& bitmap)
{
    if (true == m_isOn)
    {
        const int16_t height = bitmap.getHeight();
        const int16_t width  = bitmap.getWidth();

        for (int16_t y = 0; y < height; ++y)
        {
            for (int16_t x = 0; x < width; ++x)
            {
                HtmlColor htmlColor = static_cast<uint32_t>(bitmap.getColor(x, y));
#if CONFIG_DISPLAY_ROTATE180 != 0
                m_strip.SetPixelColor(m_topo.Map(width - x - 1, height - y - 1), htmlColor);
#else
                m_strip.SetPixelColor(m_topo.Map(x, y), htmlColor);
#endif
            }
        }

        m_strip.Show();
    }
}

void DisplayDrv::off()
{
    m_isOn = false;

    /* Simulate powered off display. */
    m_strip.ClearTo(ColorDef::BLACK);
    m_strip.Show();
}

void DisplayDrv::on()
{
    m_isOn = true;
}

bool DisplayDrv::isOn() const
{
    return m_isOn;
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

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
 * @brief  LED matrix display
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
    m_strip(Board::LedMatrix::width * Board::LedMatrix::height, Board::Pin::ledMatrixDataOutPinNo),
    m_topo(Board::LedMatrix::width, Board::LedMatrix::height),
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
        const int16_t height = m_ledMatrix.getHeight();
        const int16_t width = m_ledMatrix.getWidth();

        for (int16_t y = 0; y < height; ++y)
        {
            for (int16_t x = 0; x < width; ++x)
            {
                HtmlColor htmlColor = static_cast<uint32_t>(m_ledMatrix.getColor(x, y));
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

void Display::off()
{
    m_isOn = false;

    /* Simulate powered off display. */
    m_strip.ClearTo(ColorDef::BLACK);
    m_strip.Show();
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

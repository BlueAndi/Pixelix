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
 * @brief  Test plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "TestPlugin.h"

#include <YAColor.h>

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

void TestPlugin::update(YAGfx& gfx)
{
    const uint32_t  PERIOD      = 2000U;
    bool            showPage    = false;

    if (false == m_timer.isTimerRunning())
    {
        m_timer.start(PERIOD);
        showPage = true;
    }
    else if (true == m_timer.isTimeout())
    {
        ++m_page;
        m_page %= 2U;

        showPage = true;
        m_timer.restart();
    }
    else
    {
        ;
    }

    if (true == showPage)
    {
        int16_t min = (gfx.getWidth() > gfx.getHeight()) ? gfx.getHeight() : gfx.getWidth();

        /* Clear display */
        gfx.fillScreen(ColorDef::BLACK);

        switch(m_page)
        {
        case 0U:
            gfx.drawPixel(0, 0, ColorDef::RED);
            gfx.drawLine(1, 0, min - 1, 0, ColorDef::GREEN);
            break;

        case 1U:
            gfx.drawPixel(0, 0, ColorDef::RED);
            gfx.drawLine(0, 1, 0, min - 1, ColorDef::GREEN);
            break;

        default:
            break;
        }
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

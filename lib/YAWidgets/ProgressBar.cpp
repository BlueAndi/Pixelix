/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Progress bar
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ProgressBar.h"

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

/* Initialize text widget type. */
const char* ProgressBar::WIDGET_TYPE = "progressBar";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void ProgressBar::update(YAGfx& gfx)
{
    switch(m_algorithm)
    {
    case ALGORITHM_PIXEL_WISE:
        showProgressPixel(gfx);
        break;

    case ALGORITHM_PROGRESS_BAR:
        showProgressBar(gfx);
        break;

    case ALGORITHM_MAX:
        /* Should never happen. */
        break;

    default:
        /* Should never happen. */
        break;
    }

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void ProgressBar::showProgressPixel(YAGfx& gfx)
{
    uint16_t    pixelCount  = gfx.getWidth() * gfx.getHeight() * m_progress / 100U;
    int16_t     x           = 0;
    int16_t     y           = 0;

    while((0U < pixelCount) && (gfx.getHeight() > y))
    {
        gfx.drawPixel(x, y, m_color);

        --pixelCount;

        ++x;
        if (gfx.getWidth() <= x)
        {
            x = 0;
            ++y;
        }
    }

    return;
}

void ProgressBar::showProgressBar(YAGfx& gfx)
{
    if (gfx.getWidth() > gfx.getHeight())
    {
        uint16_t width = (gfx.getWidth() * m_progress) / 100;

        gfx.fillRect(0, 0, width, gfx.getHeight(), m_color);
    }
    else
    {
        uint16_t height = (gfx.getWidth() * m_progress) / 100;

        gfx.fillRect(0, 0, gfx.getWidth(), height, m_color);
    }

    return;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

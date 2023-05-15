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
 * @brief  Fade in/out effect by moving the old content out and the new one in.
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "FadeMoveY.h"

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

void FadeMoveY::init()
{
    m_state = FADE_STATE_INIT;
}

bool FadeMoveY::fadeIn(YAGfx& gfx, YAGfxBitmap& prev, YAGfxBitmap& next)
{
    (void)prev;

    gfx.copy(next);

    return true;
}

bool FadeMoveY::fadeOut(YAGfx& gfx, YAGfxBitmap& prev, YAGfxBitmap& next)
{
    bool    isFinished  = false;
    int16_t x           = 0;
    int16_t y           = 0;

    if (FADE_STATE_OUT != m_state)
    {
        m_state     = FADE_STATE_OUT;
        m_yOffset   = 0;
    }

    for(y = 0; y < (gfx.getHeight() - m_yOffset); ++y)
    {
        for(x = 0; x < gfx.getWidth(); ++x)
        {
            gfx.drawPixel(x, y, prev.getColor(x , (y + m_yOffset)));
        }
    }

    for(y = gfx.getHeight() - m_yOffset; y < gfx.getHeight(); ++y)
    {
        for(x = 0; x < gfx.getWidth(); ++x)
        {
            gfx.drawPixel(x, y, next.getColor(x, ((y + m_yOffset) - gfx.getHeight())));
        }
    }

    ++m_yOffset;

    if (gfx.getHeight() <= m_yOffset)
    {
        m_state     = FADE_STATE_INIT;
        isFinished  = true;
    }

    return isFinished;
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

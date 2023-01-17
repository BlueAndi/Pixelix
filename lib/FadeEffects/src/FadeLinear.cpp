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
 * @brief  Linear fade in/out effect
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "FadeLinear.h"

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

void FadeLinear::init()
{
    m_state = FADE_STATE_INIT;
}

bool FadeLinear::fadeIn(YAGfx& gfx, YAGfxBitmap& prev, YAGfxBitmap& next)
{
    bool isFinished = false;

    (void)prev;

    /* Copy next framebuffer initial to display once and fade it smooth in. */
    if (FADE_STATE_IN != m_state)
    {
        m_intensity = Color::MIN_BRIGHT;
        m_state     = FADE_STATE_IN;
    }

    if ((Color::MAX_BRIGHT - FADING_STEP) <= m_intensity)
    {
        dimBitmap(next, Color::MAX_BRIGHT);
        m_state     = FADE_STATE_INIT;
        isFinished  = true;
    }
    else
    {
        dimBitmap(next, m_intensity);
        m_intensity += FADING_STEP;
    }

    gfx.copy(next);

    return isFinished;
}

bool FadeLinear::fadeOut(YAGfx& gfx, YAGfxBitmap& prev, YAGfxBitmap& next)
{
    bool isFinished = false;

    (void)next;

    /* Copy previous framebuffer initial to display once and fade it smooth out. */
    if (FADE_STATE_OUT != m_state)
    {
        m_intensity = Color::MAX_BRIGHT;
        m_state     = FADE_STATE_OUT;
    }

    if ((Color::MIN_BRIGHT + FADING_STEP) >= m_intensity)
    {
        dimBitmap(prev, Color::MIN_BRIGHT);
        m_state     = FADE_STATE_INIT;
        isFinished  = true;
    }
    else
    {
        dimBitmap(prev, m_intensity);
        m_intensity -= FADING_STEP;
    }

    gfx.copy(prev);

    return isFinished;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void FadeLinear::dimBitmap(YAGfxBitmap& bitmap, uint8_t intensity)
{
    uint16_t    width   = bitmap.getWidth();
    uint16_t    height  = bitmap.getHeight();
    int16_t     x       = 0;
    int16_t     y       = 0;

    for(y = 0; y < height; ++y)
    {
        for(x = 0; x < width; ++x)
        {
            bitmap.getColor(x, y).setIntensity(intensity);
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

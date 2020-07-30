/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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

bool FadeLinear::fadeIn(IGfx& gfx)
{
    bool isFinished = false;

    if (FADE_STATE_IN != m_state)
    {
        m_intensity = Color::MIN_BRIGHT;
        m_state     = FADE_STATE_IN;
    }

    if ((Color::MAX_BRIGHT - FADING_STEP) <= m_intensity)
    {
        gfx.dimScreen(Color::MAX_BRIGHT);
        m_state     = FADE_STATE_INIT;
        isFinished  = true;
    }
    else
    {
        gfx.dimScreen(m_intensity);
        m_intensity += FADING_STEP;
    }

    return isFinished;
}

bool FadeLinear::fadeOut(IGfx& gfx)
{
    bool isFinished = false;

    if (FADE_STATE_OUT != m_state)
    {
        m_intensity = Color::MAX_BRIGHT;
        m_state     = FADE_STATE_OUT;
    }

    if ((Color::MIN_BRIGHT + FADING_STEP) >= m_intensity)
    {
        gfx.dimScreen(Color::MIN_BRIGHT);
        m_state     = FADE_STATE_INIT;
        isFinished  = true;
    }
    else
    {
        gfx.dimScreen(m_intensity);
        m_intensity -= FADING_STEP;
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

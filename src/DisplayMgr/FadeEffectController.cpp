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
 * @file   FadeEffectController.cpp
 * @brief  Fade effect controller
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "FadeEffectController.h"

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

void FadeEffectController::selectFadeEffect(FadeEffect effect)
{
    if (FADE_EFFECT_COUNT <= effect)
    {
        selectNextFadeEffect();
    }
    else
    {
        m_nextFadeEffectIndex = effect;
    }
}

void FadeEffectController::selectNextFadeEffect()
{
    uint8_t fadeEffectIndex = static_cast<uint8_t>(m_nextFadeEffectIndex);

    fadeEffectIndex         = (fadeEffectIndex + 1U) % FADE_EFFECT_COUNT;
    m_nextFadeEffectIndex   = static_cast<FadeEffect>(fadeEffectIndex);
}

void FadeEffectController::update(YAGfx& gfx)
{
    YAGfxDynamicBitmap& selectedFrameBuffer = m_doubleFrameBuffer.getSelectedFramebuffer();

    /* No fade effect? */
    if (nullptr == m_fadeEffect)
    {
        gfx.drawBitmap(0, 0, selectedFrameBuffer);
        m_state = FADE_IDLE;
    }
    /* Process fade effect. */
    else
    {
        YAGfxDynamicBitmap& prevFrameBuffer = m_doubleFrameBuffer.getPreviousFramebuffer();

        /* Handle fading */
        switch (m_state)
        {
        /* No fading at all */
        case FADE_IDLE:
            gfx.drawBitmap(0, 0, selectedFrameBuffer);
            break;

        /* Fade new display content in */
        case FADE_IN:
            if (true == m_fadeEffect->fadeIn(gfx, prevFrameBuffer, selectedFrameBuffer))
            {
                m_state = FADE_IDLE;
            }
            break;

        /* Fade old display content out! */
        case FADE_OUT:
            if (true == m_fadeEffect->fadeOut(gfx, prevFrameBuffer, selectedFrameBuffer))
            {
                m_state = FADE_IN;
            }
            break;

        default:
            break;
        }
    }

    if (FADE_IDLE == m_state)
    {
        /* Change fade effect on demand. */
        changeFadeEffectOnDemand();
    }
}

void FadeEffectController::start()
{
    /* Select next framebuffer and keep old content, until the fade effect is
     * finished.
     */
    m_doubleFrameBuffer.selectNextFramebuffer();

    m_state = FADE_OUT;

    if (nullptr != m_fadeEffect)
    {
        m_fadeEffect->init();
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void FadeEffectController::changeFadeEffectOnDemand()
{
    if (m_fadeEffectIndex != m_nextFadeEffectIndex)
    {
        /* Select the next fade effect */
        m_fadeEffectIndex = m_nextFadeEffectIndex;

        switch (m_fadeEffectIndex)
        {
        case FADE_EFFECT_NONE:
            m_fadeEffect = nullptr;
            break;

        case FADE_EFFECT_LINEAR:
            m_fadeEffect = &m_fadeLinearEffect;
            break;

        case FADE_EFFECT_MOVE_X:
            m_fadeEffect = &m_fadeMoveXEffect;
            break;

        case FADE_EFFECT_MOVE_Y:
            m_fadeEffect = &m_fadeMoveYEffect;
            break;

        default:
            break;
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

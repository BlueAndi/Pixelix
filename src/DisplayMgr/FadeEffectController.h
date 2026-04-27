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
 * @file   FadeEffectController.h
 * @brief  Fade effect controller
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup DISPLAY_MGR
 *
 * @{
 */

#ifndef FADE_EFFECT_CONTROLLER_H
#define FADE_EFFECT_CONTROLLER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <YAGfx.h>
#include <FadeLinear.h>
#include <FadeMoveX.h>
#include <FadeMoveY.h>

#include "DoubleFrameBuffer.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This class provides a fade effect controller.
 * It manages the fade effects and provides the interface to use them.
 */
class FadeEffectController
{
public:

    /** Fade effects */
    enum FadeEffect : uint8_t
    {
        FADE_EFFECT_NONE = 0, /**< No fade effect */
        FADE_EFFECT_LINEAR,   /**< Linear dimming fade effect. */
        FADE_EFFECT_MOVE_X,   /**< Moving fade effect into the direction of negative x-coordinates. */
        FADE_EFFECT_MOVE_Y,   /**< Moving fade effect into the direction of negative y-coordinates. */
        FADE_EFFECT_COUNT     /**< Number of fade effects. */
    };

    /**
     * Constructs the fade effect controller.
     *
     * @param[in] doubleFrameBuffer  Double framebuffer for double buffering.
     */
    FadeEffectController(DoubleFrameBuffer& doubleFrameBuffer) :
        m_doubleFrameBuffer(doubleFrameBuffer),
        m_state(FADE_IDLE),
        m_fadeEffect(nullptr),
        m_fadeLinearEffect(),
        m_fadeMoveXEffect(),
        m_fadeMoveYEffect(),
        m_fadeEffectIndex(FADE_EFFECT_NONE),
        m_nextFadeEffectIndex(m_fadeEffectIndex)
    {
        /* Nothing to do */
    }

    /**
     * Destroys the fade effect controller.
     */
    ~FadeEffectController()
    {
        /* Nothing to do */
    }

    /**
     * Get the selected fade effect.
     * It might be not the currently running fade effect. But the one which is selected
     * to be used next.
     *
     * @return Current fade effect which is selected.
     */
    FadeEffect getFadeEffect() const
    {
        return m_nextFadeEffectIndex;
    }

    /**
     * Select the fade effect.
     *
     * If the fade effect is FADE_EFFECT_COUNT, it will select the next
     * fade effect.
     *
     * @param[in] effect  Fade effect to select
     */
    void selectFadeEffect(FadeEffect effect);

    /**
     * Select next fade effect.
     * If the current fade effect is the last one, it will select the first one.
     */
    void selectNextFadeEffect();

    /**
     * Update the display.
     *
     * @param[in] gfx  Graphics interface to draw on the display.
     */
    void update(YAGfx& gfx);

    /**
     * Start fade effect.
     */
    void start();

    /**
     * Is fade effect running?
     *
     * @return If fade effect is running, it will return true otherwise false.
     */
    bool isRunning() const
    {
        return (FADE_IDLE != m_state);
    }

private:

    /** Fade state */
    enum FadeState
    {
        FADE_IDLE = 0, /**< No fading */
        FADE_IN,       /**< Fade in */
        FADE_OUT       /**< Fade out */
    };

    DoubleFrameBuffer& m_doubleFrameBuffer;   /**< Double framebuffer for double buffering. */
    FadeState          m_state;               /**< Current fade state. */
    IFadeEffect*       m_fadeEffect;          /**< The current selected fade effect. */
    FadeLinear         m_fadeLinearEffect;    /**< Linear fade effect. */
    FadeMoveX          m_fadeMoveXEffect;     /**< Moving along x-axis fade effect. */
    FadeMoveY          m_fadeMoveYEffect;     /**< Moving along y-axis fade effect. */
    FadeEffect         m_fadeEffectIndex;     /**< Current fade effect index, used to determine the next fade effect. */
    FadeEffect         m_nextFadeEffectIndex; /**< Next fade effect index, selected by the user. */

    /**
     * Default constructor is not allowed.
     */
    FadeEffectController()                                             = delete;

    /**
     * Copy constructor is not allowed.
     *
     * @param[in] other  Other instance, which to copy
     */
    FadeEffectController(const FadeEffectController& other)            = delete;

    /**
     * Assignment operator is not allowed.
     *
     * @param[in] other  Other instance, which to assign
     *
     * @return Reference to this instance
     */
    FadeEffectController& operator=(const FadeEffectController& other) = delete;

    /**
     * Change fade effect on demand.
     * If the user has selected a new fade effect, it will change the current fade effect.
     */
    void changeFadeEffectOnDemand();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* FADE_EFFECT_CONTROLLER_H */

/** @} */
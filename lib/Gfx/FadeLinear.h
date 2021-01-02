/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle Merkle <web@blue-andi.de>
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
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef __FADE_LINEAR_H__
#define __FADE_LINEAR_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IFadeEffect.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * A simple linear fade effect.
 */
class FadeLinear : public IFadeEffect
{
public:

    /**
     * Constructs the linear fade effect.
     */
    FadeLinear() :
        m_state(FADE_STATE_INIT),
        m_intensity(Color::MIN_BRIGHT)
    {
    }

    /**
     * Destroys the linear fade effect instance.
     */
    ~FadeLinear()
    {
    }

    /**
     * Initializes/reset fade effect. May be necessary in case a fade effect was aborted.
     */
    void init() override;

    /**
     * Achieves a fade in effect. Call this method as long as the effect is not completed.
     *
     * @param[in] gfx   Graphics interface to display
     * @param[in] prev  Graphics interface to previous framebuffer
     * @param[in] next  Graphics interface to next framebuffer
     *
     * @return If the effect is complete, it will return true otherwise false.
     */
    bool fadeIn(IGfx& gfx, IGfx& prev, IGfx& next) override;

    /**
     * Achieves a fade out effect. Call this method as long as the effect is not completed.
     *
     * @param[in] gfx   Graphics interface to display
     * @param[in] prev  Graphics interface to previous framebuffer
     * @param[in] next  Graphics interface to next framebuffer
     *
     * @return If the effect is complete, it will return true otherwise false.
     */
    bool fadeOut(IGfx& gfx, IGfx& prev, IGfx& next) override;

    /**
     * Fading step per fadeIn/fadeOut call.
     * If the fade effect shall take place 1s and the call period is 20ms, it will need a
     * fading step of 5 digits.
     */
    static const uint8_t FADING_STEP    = 5U;

private:

    /** Fading states. */
    enum FadeState
    {
        FADE_STATE_INIT = 0,    /**< Initialize fadeing */
        FADE_STATE_IN,          /**< Fading in is pending */
        FADE_STATE_OUT          /**< Fading out is pending */
    };

    FadeState   m_state;        /**< Current fading state */
    uint8_t     m_intensity;    /**< Current color intensity [0; 255] - 0: min. bright / 255: max. bright */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __FADE_LINEAR_H__ */

/** @} */
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
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef FADE_MOVE_X_H
#define FADE_MOVE_X_H

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
 * A simple fade in/out effect, which moves the old content and out and the
 * new content in. The movement is along the x-axis into the direction of
 * the negative x-coordinates.
 */
class FadeMoveX : public IFadeEffect
{
public:

    /**
     * Constructs the fade effect.
     */
    FadeMoveX() :
        m_state(FADE_STATE_INIT),
        m_xOffset(0)
    {
    }

    /**
     * Destroys the fade effect instance.
     */
    ~FadeMoveX()
    {
    }

    /**
     * Initializes/reset fade effect. May be necessary in case a fade effect was aborted.
     */
    void init() final;

    /**
     * Achieves a fade in effect. Call this method as long as the effect is not completed.
     *
     * @param[in] gfx   Graphics interface to display
     * @param[in] prev  Previous framebuffer
     * @param[in] next  Next framebuffer
     *
     * @return If the effect is complete, it will return true otherwise false.
     */
    bool fadeIn(YAGfx& gfx, YAGfxBitmap& prev, YAGfxBitmap& next) final;

    /**
     * Achieves a fade out effect. Call this method as long as the effect is not completed.
     *
     * @param[in] gfx   Graphics interface to display
     * @param[in] prev  Previous framebuffer
     * @param[in] next  Next framebuffer
     *
     * @return If the effect is complete, it will return true otherwise false.
     */
    bool fadeOut(YAGfx& gfx, YAGfxBitmap& prev, YAGfxBitmap& next) final;

private:

    /** Fading states. */
    enum FadeState
    {
        FADE_STATE_INIT = 0,    /**< Initialize fadeing */
        FADE_STATE_OUT          /**< Fading out is pending */
    };

    FadeState   m_state;        /**< Current fading state */
    int16_t     m_xOffset;      /**< Current x-offset regarding movement */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* FADE_MOVE_X_H */

/** @} */
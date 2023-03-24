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
 * @brief  Basic fade effect interface
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef BASE_IFADEEFFECT_HPP
#define BASE_IFADEEFFECT_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <YAGfx.h>
#include <YAGfxBitmap.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Base fade effect interface, used to fade display content in or out.
 * The effect will fade in/out from one framebuffer to another and draws the
 * result directly to the display.
 */
class IFadeEffect
{
public:

    /**
     * Destroys the fade effect interface.
     */
    virtual ~IFadeEffect()
    {
    }

    /**
     * Initializes/reset fade effect. May be necessary in case a fade effect was aborted.
     */
    virtual void init() = 0;

    /**
     * Achieves a fade in effect. Call this method as long as the effect is not completed.
     *
     * @param[in] gfx   Graphics interface to display
     * @param[in] prev  Previous framebuffer
     * @param[in] next  Next framebuffer
     *
     * @return If the effect is complete, it will return true otherwise false.
     */
    virtual bool fadeIn(YAGfx& gfx, YAGfxBitmap& prev, YAGfxBitmap& next) = 0;

    /**
     * Achieves a fade out effect. Call this method as long as the effect is not completed.
     *
     * @param[in] gfx   Graphics interface to display
     * @param[in] prev  Previous framebuffer
     * @param[in] next  Next framebuffer
     *
     * @return If the effect is complete, it will return true otherwise false.
     */
    virtual bool fadeOut(YAGfx& gfx, YAGfxBitmap& prev, YAGfxBitmap& next) = 0;

protected:

    /**
     * Constructs the fade effect interface.
     */
    IFadeEffect()
    {
    }

private:

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* BASE_IFADEEFFECT_HPP */

/** @} */
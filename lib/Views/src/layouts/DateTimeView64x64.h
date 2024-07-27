/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
 * @brief  View for 64x64 LED matrix with date and time.
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup plugin
 *
 * @{
 */

#ifndef DATE_TIME_VIEW_64X64_H
#define DATE_TIME_VIEW_64X64_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <YAGfx.h>
#include "DateTimeViewGeneric.h"

#include <Fonts.h>
#include <LampWidget.h>
#include <TextWidget.h>
#include <Util.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * View for 64x64 LED matrix with date and time.
 */
class DateTimeView64x64 : public DateTimeViewGeneric
{
public:

    /**
     * Construct the view.
     */
    DateTimeView64x64() : DateTimeViewGeneric()
    {
        /* Disable fade effect in case the user required to show seconds,
         * which will continuously trigger the fading effect.
         */
        m_textWidget.disableFadeEffect();

        /*
         * Move digital clock to lower paet of analog clock.
         * Analog clock is also shifted in X by one as the 
         * mid point is 31.5
         */
        m_textWidget.move(1, 46); 
    }

    /**
     * Destroy the view.
     */
    virtual ~DateTimeView64x64()
    {
    }

    /**
     * Update the underlying canvas.
     * 
     * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
     */
    void update(YAGfx& gfx) override;

    
    
protected:

    /**
     * Center x-coordinate of analog clock 
     */

    static const int16_t ANALOG_CENTER_X     = 32;

    /**
     * Center y-coordinate of analog clock 
     */
    static const int16_t ANALOG_CENTER_Y     = 31;

    /**
     * Anaolog Clock radius  
     */
    static const int16_t ANALOG_RADIUS       = 31;

    DateTimeView64x64(const DateTimeView64x64& other);
    DateTimeView64x64& operator=(const DateTimeView64x64& other);

    /**
     * Draw analog clock backround (the minute tick marks)
     * 
     * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
     */
    void drawAnalogClockBackground(YAGfx& gfx);

    /**
     * Draw analog clock hands for given time.
     * 
     * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
     * @param[in] now current time 
     */

    /**
     * @brief 
     * 
     * @param[in] gfx    Graphic functionality to draw on the underlying canvas.
     * @param[in] minute Minute to point to (0..59).
     * @param[in] radius Length of hand (radius from clock mid point)
     * @param[in] col    Color of the hand.
     */
    void drawAnalogClockHand(YAGfx& gfx, int16_t minute, int16_t radius, const Color& col);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* DATE_TIME_VIEW_64X64_H */

/** @} */

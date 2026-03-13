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
 * @file   IIndicatorView.h
 * @brief  Indicator view interface
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef IINDICATOR_VIEW_H
#define IINDICATOR_VIEW_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <YAGfx.h>
#include <WString.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Interface for a view with indicators in each corner of the display.
 * The indicators can be used to show the state of a certain functionality.
 *
 * If a indicator is set to on, it will be drawn with the corresponding color,
 * otherwise it will not be drawn at all.
 */
class IIndicatorView
{
public:

    /**
     * Indicator state.
     */
    enum State
    {
        STATE_OFF = 0, /**< Indicator is off. */
        STATE_ON,      /**< Indicator is on. */
        STATE_BLINK    /**< Indicator is blinking. */
    };

    /**
     * Destroy the interface.
     */
    virtual ~IIndicatorView()
    {
    }

    /**
     * Initialize view, which will prepare the widgets and the default values.
     *
     * @param[in] width     Display width in pixel.
     * @param[in] height    Display height in pixel.
     */
    virtual void init(uint16_t width, uint16_t height)          = 0;

    /**
     * Update the underlying canvas.
     *
     * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
     */
    virtual void update(YAGfx& gfx)                             = 0;

    /**
     * Set the indicator at given position to on/off state.
     * If the indicator id is invalid, it will do nothing.
     *
     * The indicator id 255 will be used to turn on/off all indicators at once.
     *
     * @param[in] indicatorId   Id of the indicator, which to set.
     * @param[in] state          State to set the indicator to.
     */
    virtual void setIndicator(uint8_t indicatorId, State state) = 0;

    /**
     * Get the indicator state at given position.
     * If the indicator id is invalid, it will return STATE_OFF.
     *
     * @param[in]  indicatorId   Id of the indicator, which to set.
     *
     * @return State of the indicator.
     */
    virtual State getIndicatorState(uint8_t indicatorId) const  = 0;

protected:

    /**
     * Construct the interface.
     */
    IIndicatorView()
    {
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* IINDICATOR_VIEW_H */

/** @} */

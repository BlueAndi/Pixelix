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
 * @file   IndicatorViewGeneric.cpp
 * @brief  Generic view with indicators in each display corner.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IndicatorViewGeneric.h"

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

void IndicatorViewGeneric::update(YAGfx& gfx)
{
    size_t idx;

    for (idx = 0U; idx < MAX_LAMPS; ++idx)
    {
        LampWidget& lampWidget = m_lampWidgets[idx];

        if (STATE_BLINK == m_lampStates[idx])
        {
            if (true == m_blinkTimer.isTimeout())
            {
                lampWidget.toggle();
            }
        }

        /* Update the lamp widget if it is turned on.
         * If the lamp is turned off, it shall not overwrite the display.
         */
        if (true == lampWidget.getOnState())
        {
            lampWidget.update(gfx);
        }
    }

    if (true == m_blinkTimer.isTimeout())
    {
        m_blinkTimer.restart();
    }
}

void IndicatorViewGeneric::setIndicator(uint8_t indicatorId, State state)
{
    /* Check if the indicator id is valid. */
    if (MAX_LAMPS > indicatorId)
    {
        if (STATE_OFF == state)
        {
            m_lampWidgets[indicatorId].setOnState(false);
        }
        else
        {
            m_lampWidgets[indicatorId].setOnState(true);
        }

        m_lampStates[indicatorId] = state;
    }
    /* Special case to turn on/off all lamps? */
    else if (INDICATOR_ID_ALL == indicatorId)
    {
        size_t idx;

        for (idx = 0U; idx < MAX_LAMPS; ++idx)
        {
            m_lampWidgets[idx].setOnState(STATE_OFF == state ? false : true);
            m_lampStates[idx] = state;
        }
    }
    /* Invalid indicator id, do nothing. */
    else
    {
        /* Nothing to do. */
        ;
    }
}

IIndicatorView::State IndicatorViewGeneric::getIndicatorState(uint8_t indicatorId) const
{
    State state = STATE_OFF;

    /* Check if the indicator id is valid. */
    if (MAX_LAMPS > indicatorId)
    {
        state = m_lampStates[indicatorId];
    }
    /* Invalid indicator id, return off state. */
    else
    {
        /* Nothing to do. */
        ;
    }

    return state;
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

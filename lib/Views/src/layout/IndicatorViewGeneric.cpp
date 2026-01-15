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

        /* If the lamp is on, it will be drawn otherwise not. */
        if (true == lampWidget.getOnState())
        {
            lampWidget.update(gfx);
        }
    }
}

void IndicatorViewGeneric::setIndicator(uint8_t indicatorId, bool isOn)
{
    /* Check if the indicator id is valid. */
    if (indicatorId < MAX_LAMPS)
    {
        m_lampWidgets[indicatorId].setOnState(isOn);
    }
    /* Special case to turn on/off all lamps? */
    else if (INDICATOR_ID_ALL == indicatorId)
    {
        size_t idx;

        for (idx = 0U; idx < MAX_LAMPS; ++idx)
        {
            m_lampWidgets[idx].setOnState(isOn);
        }
    }
    /* Invalid indicator id, do nothing. */
    else
    {
        /* Nothing to do. */
        ;
    }
}

bool IndicatorViewGeneric::isIndicatorOn(uint8_t indicatorId) const
{
    bool isOn = false;

    if (indicatorId < MAX_LAMPS)
    {
        isOn = m_lampWidgets[indicatorId].getOnState();
    }

    return isOn;
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

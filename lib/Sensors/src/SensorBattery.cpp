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
 * @brief  Battery state of charge driver
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "SensorBattery.h"

#include <Board.h>

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

uint32_t BatteryChannelSoC::getValue()
{
    return m_driver->getStateOfCharge() + m_offset;
}

void SensorBattery::begin()
{
    if (IoPin::NC != Board::batteryVoltageIn.getPinNo())
    {
        m_isAvailable = true;
    }
}

bool SensorBattery::isAvailable() const
{
    return m_isAvailable;
}

ISensorChannel* SensorBattery::getChannel(uint8_t index)
{
    ISensorChannel* channel = nullptr;

    if (true == m_isAvailable)
    {
        switch(index)
        {
        case CHANNEL_ID_SOC:
            channel = &m_socChannel;
            break;

        default:
            break;
        }
    }

    return channel;
}

float SensorBattery::getStateOfCharge()
{
    uint32_t stateOfCharge  = 0U;
    uint16_t adcRawAvg      = getAdcRawAvg();

    if (ADC_RAW_FULL <= adcRawAvg)
    {
        stateOfCharge = 100U;
    }
    else if (ADC_RAW_EMPTY < adcRawAvg)
    {
        stateOfCharge = adcRawAvg - ADC_RAW_EMPTY;
        stateOfCharge *= 100U;
        stateOfCharge /= ADC_RAW_FULL - ADC_RAW_EMPTY;
    }
    else
    {
        ;
    }

    return stateOfCharge;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

uint16_t SensorBattery::getAdcRawAvg()
{
    const uint16_t ADC_BATTERY_VOLTAGE = Board::batteryVoltageIn.read();

    if (true == m_isInit)
    {
        m_adcRawAvg = ADC_BATTERY_VOLTAGE;
        m_isInit = false;
    }
    else
    {
        /* Simple moving average.
         * 87.5% of original value
         * 12.5% of new value
         */
        m_adcRawAvg -= m_adcRawAvg / 8U;
        m_adcRawAvg += ADC_BATTERY_VOLTAGE / 8U;
    }

    return m_adcRawAvg;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

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
 * @brief  SHT3x sensor
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "SensorSht3X.h"

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

void SensorSht3X::begin()
{
    if (false == m_driver.init())
    {
        m_isAvailable = false;
    }
    else
    {
        m_isAvailable = true;
        
        /* Only supported for SHT3x sensors */
        m_driver.setAccuracy(SHTSensor::SHT_ACCURACY_MEDIUM);
    }
}

void SensorSht3X::process()
{
    if (true == m_isAvailable)
    {
        (void)m_driver.readSample();
    }
}

const char* SensorSht3X::getName() const
{
    /* Model can not be read back and the automatic detection may be enabled,
     * so the given model (via constructor) can not be used.
     */
    return "SHT3x";
}

ISensorChannel* SensorSht3X::getChannel(uint8_t index)
{
    ISensorChannel* channel = nullptr;

    if (true == m_isAvailable)
    {
        switch(index)
        {
        case CHANNEL_ID_TEMPERATURE:
            channel = &m_temperatureChannel;
            break;

        case CHANNEL_ID_HUMIDITY:
            channel = &m_humidityChannel;
            break;
        
        default:
            break;
        }
    }

    return channel;
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

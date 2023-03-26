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
 * @brief  DHTx sensor
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "SensorDhtX.h"

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

void SensorDhtX::begin()
{
    float temperature   = NAN;
    float humidity      = NAN;

    m_driver.begin();
    
    /* Detect whether a sensor is available. */
    temperature = m_driver.readTemperature();
    humidity    = m_driver.readHumidity();

    if (isnan(temperature) || isnan(humidity))
    {
        m_isAvailable = false;
    }
    else
    {
        m_isAvailable = true;
    }
}

const char* SensorDhtX::getName() const
{
    const char* sensorName  = "?";

    switch(m_model)
    {
    case MODEL_DHT11:
        sensorName = "DHT11";
        break;
    
    case MODEL_DHT12:
        sensorName = "DHT12";
        break;
    
    case MODEL_DHT21:
        sensorName = "DHT21";
        break;

    case MODEL_DHT22:
        sensorName = "DHT22";
        break;

    default:
        break;
    }

    return sensorName;
}

ISensorChannel* SensorDhtX::getChannel(uint8_t index)
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

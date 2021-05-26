/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Sensor data provider
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "SensorDataProvider.h"
#include <Sensors.h>

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

void SensorDataProvider::begin()
{
    m_impl->begin();
}

uint8_t SensorDataProvider::getAvailSensors() const
{
    return m_impl->getAvailSensors();
}


ISensor* SensorDataProvider::getSensor(uint8_t index)
{
    return m_impl->getSensor(index);
}

bool SensorDataProvider::find(
    uint8_t& sensorIndex,
    uint8_t& channelIndex,
    ISensorChannel::DataWithUnit dataWithUnit,
    ISensorChannel::DataType dataType,
    uint8_t sensorStartIdx,
    uint8_t channelStartIdx)
{
    uint8_t sensorIdx   = sensorStartIdx;
    uint8_t channelIdx  = channelStartIdx;
    uint8_t sensorCnt   = m_impl->getAvailSensors();
    bool    isFound     = false;

    while((sensorCnt > sensorIdx) && (false == isFound))
    {
        ISensor* sensor = m_impl->getSensor(sensorIdx);
        
        if (nullptr != sensor)
        {
            uint8_t channelCnt = sensor->getNumChannels();

            while((channelCnt > channelIdx) && (false == isFound))
            {
                ISensorChannel* channel = sensor->getChannel(channelIdx);

                if (nullptr != channel)
                {
                    if (channel->getDataWithUnit() == dataWithUnit)
                    {
                        if (ISensorChannel::DATA_TYPE_INVALID == dataType)
                        {
                            isFound = true;
                        }
                    }
                    else
                    {
                        if (channel->getType() == dataType)
                        {
                            isFound = true;
                        }
                    }
                }

                ++channelIdx;
            }
        }

        ++sensorIdx;
    }

    if (sensorCnt <= sensorIdx)
    {
        sensorIdx = INVALID_SENSOR_IDX;
    }

    return sensorIdx;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

SensorDataProvider::SensorDataProvider() :
    m_impl(Sensors::getSensorDataProviderImpl())
{
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

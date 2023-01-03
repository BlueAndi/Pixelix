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
 * @brief  Sensor data provider
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "SensorDataProvider.h"
#include <Sensors.h>
#include <Logging.h>

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
    uint8_t index   = 0U;
    uint8_t cnt     = m_impl->getNumSensors();

    /* Initialize all sensor drivers. */
    m_impl->begin();

    /* For debug purposes, show the sensor driver states. */
    for(index = 0U; index < cnt; ++index)
    {
        ISensor* sensor = m_impl->getSensor(index);

        if (nullptr != sensor)
        {
            bool isAvailable = sensor->isAvailable();

            LOG_INFO("Sensor %s: %s", sensor->getName(), (false == isAvailable) ? "-" : "available" );
        }
    }
}

uint8_t SensorDataProvider::getNumSensors() const
{
    return m_impl->getNumSensors();
}

ISensor* SensorDataProvider::getSensor(uint8_t index)
{
    return m_impl->getSensor(index);
}

bool SensorDataProvider::find(
    uint8_t& sensorIndex,
    uint8_t& channelIndex,
    ISensorChannel::Type type,
    ISensorChannel::DataType dataType,
    uint8_t sensorStartIdx,
    uint8_t channelStartIdx)
{
    uint8_t sensorIdx   = sensorStartIdx;
    uint8_t channelIdx  = channelStartIdx;
    uint8_t sensorCnt   = m_impl->getNumSensors();
    bool    isFound     = false;

    while((sensorCnt > sensorIdx) && (false == isFound))
    {
        ISensor* sensor = m_impl->getSensor(sensorIdx);
        
        /* A sensor driver must be installed and of course, a physical
         * sensor must be available.
         */
        if ((nullptr != sensor) &&
            (true == sensor->isAvailable()))
        {
            uint8_t channelCnt = sensor->getNumChannels();

            /* Walk through all sensor channels and try to find the requested one. */
            channelIdx = 0U;
            while((channelCnt > channelIdx) && (false == isFound))
            {
                ISensorChannel* channel = sensor->getChannel(channelIdx);

                if (nullptr != channel)
                {
                    /* The kind of data and its unit must always match. */
                    if (channel->getType() == type)
                    {
                        /* Shall the value data type be considered? */
                        if (ISensorChannel::DATA_TYPE_INVALID != dataType)
                        {
                            if (channel->getDataType() == dataType)
                            {
                                isFound = true;
                            }
                        }
                        else
                        /* Don't consider the value data type. */
                        {
                            isFound = true;
                        }

                        if (true == isFound)
                        {
                            sensorIndex     = sensorIdx;
                            channelIndex    = channelIdx;
                        }
                    }
                }

                ++channelIdx;
            }
        }

        ++sensorIdx;
    }

    return isFound;
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

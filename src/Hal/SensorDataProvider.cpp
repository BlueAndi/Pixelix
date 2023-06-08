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
#include <JsonFile.h>
#include <FileSystem.h>
#include <SensorChannelType.hpp>

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

/* Initialize file name where to find the sensor calibration values. */
const char* SensorDataProvider::SENSOR_CALIB_FILE_NAME = "/configuration/sensors.json";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void SensorDataProvider::begin()
{
    uint8_t index   = 0U;
    uint8_t cnt     = m_impl->getNumSensors();

    /* Initialize all sensor drivers. */
    m_impl->begin();
    
    /* Load calibration values. If they are not available, save it with the sensor defaults. */
    if (false == load())
    {
        uint8_t                             defaultValues                   = 0U;
        const SensorChannelDefaultValue*    sensorChannelDefaultValueList   = Sensors::getSensorChannelDefaultValues(defaultValues);

        /* Use the default values. */
        for(index = 0U; index < defaultValues; ++index)
        {
            const SensorChannelDefaultValue*    value   = &sensorChannelDefaultValueList[index];
            ISensor*                            sensor  = getSensor(value->sensorId);
            ISensorChannel*                     channel = sensor->getChannel(value->channelId);
            DynamicJsonDocument                 jsonDoc(256U);

            if (DeserializationError::Ok == deserializeJson(jsonDoc, value->jsonStrValue))
            {
                channelOffsetFromJson(*channel, jsonDoc["offset"]);
            }
        }

        (void)save();
    }

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

bool SensorDataProvider::load()
{
    bool                status                  = false;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint8_t             sensorIdx               = 0U;
    const uint8_t       SENSOR_CNT              = m_impl->getNumSensors();

    if (true == jsonFile.load(SENSOR_CALIB_FILE_NAME, jsonDoc))
    {
        while(SENSOR_CNT > sensorIdx)
        {
            ISensor* sensor = m_impl->getSensor(sensorIdx);
            
            if (nullptr != sensor)
            {
                JsonArrayConst jsonArray = jsonDoc[sensor->getName()];

                if (false == jsonArray.isNull())
                {
                    uint8_t         channelIdx  = 0U;
                    const uint8_t   CHANNEL_CNT = sensor->getNumChannels();

                    while(CHANNEL_CNT > channelIdx)
                    {
                        ISensorChannel* channel = sensor->getChannel(channelIdx);

                        if (nullptr != channel)
                        {
                            channelOffsetFromJson(*channel, jsonArray[channelIdx]);
                        }

                        ++channelIdx;
                    }
                }
            }

            ++sensorIdx;
        }

        status = true;
    }

    return status;
}

bool SensorDataProvider::save()
{
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint8_t             sensorIdx               = 0U;
    const uint8_t       SENSOR_CNT              = m_impl->getNumSensors();

    while(SENSOR_CNT > sensorIdx)
    {
        ISensor* sensor = m_impl->getSensor(sensorIdx);

        if (nullptr != sensor)
        {
            JsonArray jsonChannels = jsonDoc.createNestedArray(sensor->getName());

            if (true == sensor->isAvailable())
            {
                uint8_t         channelIdx  = 0U;
                const uint8_t   CHANNEL_CNT = sensor->getNumChannels();

                while(CHANNEL_CNT > channelIdx)
                {
                    ISensorChannel* channel = sensor->getChannel(channelIdx);

                    if (nullptr == channel)
                    {
                        jsonChannels.add("null");
                    }
                    else
                    {
                        channelOffsetToJson(jsonChannels, *channel);
                    }

                    ++channelIdx;
                }
            }
        }

        ++sensorIdx;
    }

    return jsonFile.save(SENSOR_CALIB_FILE_NAME, jsonDoc);
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

void SensorDataProvider::channelOffsetToJson(JsonArray& jsonOffset, const ISensorChannel& channel) const
{
    switch(channel.getDataType())
    {
    case ISensorChannel::DataType::DATA_TYPE_INVALID:
        jsonOffset.add("NaN");
        break;

    case ISensorChannel::DataType::DATA_TYPE_UINT32:
        {
            const SensorChannelUInt32* uint32Channel = reinterpret_cast<const SensorChannelUInt32*>(&channel);

            if (nullptr == uint32Channel)
            {
                jsonOffset.add("NaN");
            }
            else
            {
                jsonOffset.add(uint32Channel->getOffset());
            }
        }
        break;

    case ISensorChannel::DataType::DATA_TYPE_INT32:
        {
            const SensorChannelInt32* int32Channel = reinterpret_cast<const SensorChannelInt32*>(&channel);

            if (nullptr == int32Channel)
            {
                jsonOffset.add("NaN");
            }
            else
            {
                jsonOffset.add(int32Channel->getOffset());
            }

        }
        break;

    case ISensorChannel::DataType::DATA_TYPE_FLOAT32:
        {
            const SensorChannelFloat32* float32Channel = reinterpret_cast<const SensorChannelFloat32*>(&channel);

            if (nullptr == float32Channel)
            {
                jsonOffset.add("NaN");
            }
            else
            {
                jsonOffset.add(float32Channel->getOffset());
            }
        }
        break;

    case ISensorChannel::DataType::DATA_TYPE_BOOL:
        jsonOffset.add("NaN");
        break;

    default:
        jsonOffset.add("NaN");
        break;
    }
}

void SensorDataProvider::channelOffsetFromJson(ISensorChannel& channel, JsonVariantConst jsonOffset) const
{
    switch(channel.getDataType())
    {
    case ISensorChannel::DataType::DATA_TYPE_INVALID:
        break;

    case ISensorChannel::DataType::DATA_TYPE_UINT32:
        {
            SensorChannelUInt32* uint32Channel = reinterpret_cast<SensorChannelUInt32*>(&channel);

            if ((nullptr != uint32Channel) &&
                (true == jsonOffset.is<uint32_t>()))
            {
                uint32Channel->setOffset(jsonOffset.as<uint32_t>());
            }
        }
        break;

    case ISensorChannel::DataType::DATA_TYPE_INT32:
        {
            SensorChannelInt32* int32Channel = reinterpret_cast<SensorChannelInt32*>(&channel);

            if ((nullptr != int32Channel) &&
                (true == jsonOffset.is<int32_t>()))
            {
                int32Channel->setOffset(jsonOffset.as<int32_t>());
            }
        }
        break;

    case ISensorChannel::DataType::DATA_TYPE_FLOAT32:
        {
            SensorChannelFloat32* float32Channel = reinterpret_cast<SensorChannelFloat32*>(&channel);

            if ((nullptr != float32Channel) &&
                (true == jsonOffset.is<float>()))
            {
                float32Channel->setOffset(jsonOffset.as<float>());
            }
        }
        break;

    case ISensorChannel::DataType::DATA_TYPE_BOOL:
        /* Not supported. */
        break;

    default:
        break;
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

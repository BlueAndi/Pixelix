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
#include <Util.h>
#include <TopicHandlerService.h>
#include <SettingsService.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/** The number of sensor topics: temperature, humidity, illuminance and battery */
#define SENSOR_TOPICS_COUNT (4U)

/** Defines the extra configuration for the homeassistant support. */
#define EXTRA_HOMEASSISTANT(_component, _name, _unit, _icon, _deviceClass)  \
    "{"                                                     \
        "\"ha\": {"                                         \
            "\"component\": \"" _component "\","            \
            "\"discovery\": {"                              \
                "\"name\": \"" _name "\","                  \
                "\"unit_of_meas\": \"" _unit "\","          \
                "\"ic\": \"" _icon "\","                    \
                "\"dev_cla\": \"" _deviceClass "\","        \
                "\"val_tpl\": \"{{ value_json.value }}\""   \
            "}"                                             \
        "}"                                                 \
    "}"

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/** This type defines the required data to publish sensor values as topics. */
typedef struct
{
    ISensorChannel::Type    sensorChannelType;  /**< Sensor channel type. */
    const char*             extra;              /**< Extra data as JSON string, e.g. for homeassistant extension. */
    uint32_t                updatePeriod;       /**< Max. sensor data update period in ms regarding publishing. */

} SensorTopic;

/** This type defines the runtime data for a sensor topic, required for publishing. */
typedef struct
{
    String      lastValue;      /**< Last published sensor value. */
    uint32_t    lastTimestamp;  /**< Last timestamp of publishing, used to limit the update period. */

} SensorTopicRunData;

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/* Initialize file name where to find the sensor calibration values. */
const char* SensorDataProvider::SENSOR_CALIB_FILE_NAME = "/configuration/sensors.json";

/** The provided sensor topics. */
static const SensorTopic gSensorTopics[SENSOR_TOPICS_COUNT] =
{
    {
        ISensorChannel::TYPE_TEMPERATURE_DEGREE_CELSIUS,
        EXTRA_HOMEASSISTANT("sensor", "Temperature", "°C", "mdi:thermometer", "temperature"),
        30000U
    },
    {
        ISensorChannel::TYPE_HUMIDITY_PERCENT,
        EXTRA_HOMEASSISTANT("sensor", "Humidity", "%", "mdi:water-percent", "humidity"),
        30000U
    },
    {
        ISensorChannel::TYPE_ILLUMINANCE_LUX,
        EXTRA_HOMEASSISTANT("sensor", "Illuminance", "lx", "mdi:sun-wireless", "illuminance"),
        10000U
    },
    {
        ISensorChannel::TYPE_STATE_OF_CHARGE_PERCENT,
        EXTRA_HOMEASSISTANT("sensor", "Battery", "%", "mdi:battery-90", "battery"),
        10000U
    }
};

/** The runtime sensor topic data. */
static SensorTopicRunData gSensorLastValue[SENSOR_TOPICS_COUNT] =
{
    {   String(), 0U    },
    {   String(), 0U    },
    {   String(), 0U    },
    {   String(), 0U    }
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void SensorDataProvider::begin()
{
    SettingsService& settings = SettingsService::getInstance();

    if (false == settings.open(true))
    {
        m_deviceId = settings.getHostname().getDefault();
    }
    else
    {
        m_deviceId = settings.getHostname().getValue();

        settings.close();
    }

    /* Initialize all sensor drivers. */
    m_impl->begin();
    
    /* Load calibration values. If they are not available, save it with the sensor defaults. */
    if (false == load())
    {
        createCalibrationFile();
    }

    logSensorAvailability();
    registerSensorTopics();
    
    m_timer.start(SENSOR_PROCESS_PERIOD);
    m_isInitialized = true;
}

void SensorDataProvider::end()
{
    m_isInitialized = false;
    m_timer.stop();
    unregisterSensorTopics();
}

void SensorDataProvider::process()
{
    if ((true == m_isInitialized) &&
        (true == m_timer.isTimeout()))
    {
        m_impl->process();
        m_timer.restart();
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
            
            /* Don't reset in front of the channel loop to avoid overwritting
             * the initial channel start index.
             */
            channelIdx = 0U;
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

    if (true == jsonFile.load(SENSOR_CALIB_FILE_NAME, jsonDoc))
    {
        uint8_t         sensorIdx   = 0U;
        const uint8_t   SENSOR_CNT  = m_impl->getNumSensors();

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
    m_impl(Sensors::getSensorDataProviderImpl()),
    m_deviceId(),
    m_timer(),
    m_isInitialized(false)
{
}

void SensorDataProvider::logSensorAvailability()
{
    uint8_t index   = 0U;
    uint8_t cnt     = m_impl->getNumSensors();

    /* For user information, show the sensor driver states. */
    while(cnt > index)
    {
        ISensor* sensor = m_impl->getSensor(index);

        if (nullptr != sensor)
        {
            bool isAvailable = sensor->isAvailable();

            LOG_INFO("Sensor %s: %s", sensor->getName(), (false == isAvailable) ? "-" : "available" );
        }

        ++index;
    }
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

            jsonOffset.add(uint32Channel->getOffset());
        }
        break;

    case ISensorChannel::DataType::DATA_TYPE_INT32:
        {
            const SensorChannelInt32* int32Channel = reinterpret_cast<const SensorChannelInt32*>(&channel);

            jsonOffset.add(int32Channel->getOffset());
        }
        break;

    case ISensorChannel::DataType::DATA_TYPE_FLOAT32:
        {
            const SensorChannelFloat32* float32Channel = reinterpret_cast<const SensorChannelFloat32*>(&channel);

            jsonOffset.add(float32Channel->getOffset());
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

            if (true == jsonOffset.is<uint32_t>())
            {
                uint32Channel->setOffset(jsonOffset.as<uint32_t>());
            }
        }
        break;

    case ISensorChannel::DataType::DATA_TYPE_INT32:
        {
            SensorChannelInt32* int32Channel = reinterpret_cast<SensorChannelInt32*>(&channel);

            if (true == jsonOffset.is<int32_t>())
            {
                int32Channel->setOffset(jsonOffset.as<int32_t>());
            }
        }
        break;

    case ISensorChannel::DataType::DATA_TYPE_FLOAT32:
        {
            SensorChannelFloat32* float32Channel = reinterpret_cast<SensorChannelFloat32*>(&channel);

            if (true == jsonOffset.is<float>())
            {
                float32Channel->setOffset(jsonOffset.as<float>());
            }
        }
        break;

    case ISensorChannel::DataType::DATA_TYPE_BOOL:
        /* Not supported. */
        break;

    default:
        /* Not supported. */
        break;
    }
}

void SensorDataProvider::createCalibrationFile()
{
    uint8_t                             defaultValues                   = 0U;
    const SensorChannelDefaultValue*    sensorChannelDefaultValueList   = Sensors::getSensorChannelDefaultValues(defaultValues);

    /* Use the default values. */
    if (nullptr != sensorChannelDefaultValueList)
    {
        uint8_t index = 0U;

        for(index = 0U; index < defaultValues; ++index)
        {
            const SensorChannelDefaultValue*    value   = &sensorChannelDefaultValueList[index];
            ISensor*                            sensor  = getSensor(value->sensorId);

            if (nullptr == sensor)
            {
                LOG_ERROR("Sensor %u doesn't exists.", value->sensorId);
            }
            else
            {
                ISensorChannel* channel = sensor->getChannel(value->channelId);

                if (nullptr == channel)
                {
                    LOG_ERROR("Sensor %u has no channel %u.", value->sensorId, value->channelId);
                }
                else
                {
                    const size_t        JSON_DOC_SIZE   = 256U;
                    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

                    if (DeserializationError::Ok == deserializeJson(jsonDoc, value->jsonStrValue))
                    {
                        channelOffsetFromJson(*channel, jsonDoc["offset"]);
                    }
                }
            }
        }
    }

    (void)save();
}

void SensorDataProvider::registerSensorTopics()
{
    uint8_t                 index                   = 0U;
    TopicHandlerService&    topicHandlerService     = TopicHandlerService::getInstance();

    for(index = 0U; index < UTIL_ARRAY_NUM(gSensorTopics); ++index)
    {
        const SensorTopic*  sensorTopic             = &gSensorTopics[index];
        SensorTopicRunData* sensorTopicRunData      = &gSensorLastValue[index];
        const size_t        JSON_DOC_SIZE           = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObjectConst     extra;

        if (DeserializationError::Ok != deserializeJson(jsonDoc, sensorTopic->extra))
        {
            LOG_ERROR("Sensor/Channel %u discovery details error.", index);
        }
        else
        {
            uint8_t sensorIndex     = 0U;
            uint8_t channelIndex    = 0U;
            
            extra = jsonDoc.as<JsonObjectConst>();

            /* Try to find a sensor channel which provides the required information. */
            if (true == find(sensorIndex, channelIndex, sensorTopic->sensorChannelType))
            {
                const uint32_t                      VALUE_PRECISION = 2U; /* 2 digits after the . */
                ISensor*                            sensor          = this->getSensor(sensorIndex);
                ISensorChannel*                     sensorChannel   = sensor->getChannel(channelIndex);
                String                              channelName     = "/" + ISensorChannel::channelTypeToName(sensorTopic->sensorChannelType);
                String                              entityId        = "sensors/";
                ITopicHandler::GetTopicFunc         getTopicFunc    =
                    [sensorTopic, sensorChannel, VALUE_PRECISION](const String& topic, JsonObject& jsonValue) -> bool
                    {
                        bool    isSuccessful    = false;
                        String  value           = sensorChannel->getValueAsString(VALUE_PRECISION);

                        /* The callback is dedicated to a topic, therefore the
                         * topic parameter is not used.
                         */
                        UTIL_NOT_USED(topic);

                        /* Floating point channels may provide NaN. */
                        if (value != "NAN")
                        {
                            jsonValue["value"] = value;

                            isSuccessful = true;
                        }

                        return isSuccessful;
                    };
                TopicHandlerService::HasChangedFunc hasChangedFunc  =
                    [sensorTopic, sensorChannel, sensorTopicRunData, VALUE_PRECISION](const String& topic) -> bool
                    {
                        bool            hasChanged      = false;
                        String          value           = sensorChannel->getValueAsString(VALUE_PRECISION);
                        uint32_t        timestamp       = millis();
                        uint32_t        delta           = timestamp - sensorTopicRunData->lastTimestamp;

                        /* The callback is dedicated to a topic, therefore the
                         * topic parameter is not used.
                         */
                        UTIL_NOT_USED(topic);

                        if ((value != "NAN") &&                         /* Floating point channels may provide NaN. */
                            (sensorTopicRunData->lastValue != value) && /* Value changed? */
                            (sensorTopic->updatePeriod <= delta))       /* Update period expired? */
                        {
                            sensorTopicRunData->lastValue       = value;
                            sensorTopicRunData->lastTimestamp   = timestamp;

                            hasChanged = true;
                        }

                        return hasChanged;
                    };

                entityId += index;

                topicHandlerService.registerTopic(m_deviceId, entityId, channelName, extra, getTopicFunc, hasChangedFunc, nullptr, nullptr);
            }
        }
    }
}

void SensorDataProvider::unregisterSensorTopics()
{
    uint8_t                 index                   = 0U;
    TopicHandlerService&    topicHandlerService     = TopicHandlerService::getInstance();

    for(index = 0U; index < UTIL_ARRAY_NUM(gSensorTopics); ++index)
    {
        const SensorTopic*  sensorTopic = &gSensorTopics[index];
        String              channelName = "/" + ISensorChannel::channelTypeToName(sensorTopic->sensorChannelType);
        String              entityId    = "sensors/";

        entityId += index;
        
        topicHandlerService.unregisterTopic(m_deviceId, entityId, channelName);
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

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
 * @brief  Sensor plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "SensorPlugin.h"

#include <Logging.h>
#include <ArduinoJson.h>
#include <SensorDataProvider.h>
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

/* Initialize plugin topic. */
const char* SensorPlugin::TOPIC_CONFIG = "/channel";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void SensorPlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC_CONFIG);
}

bool SensorPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CONFIG))
    {
        getConfiguration(value);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool SensorPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CONFIG))
    {
        const size_t        JSON_DOC_SIZE           = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg                 = jsonDoc.to<JsonObject>();
        JsonVariantConst    jsonSensorIndex         = value["sensorIndex"];
        JsonVariantConst    jsonChannelIndex        = value["channelIndex"];

        /* The received configuration may not contain all single key/value pair.
         * Therefore read first the complete internal configuration and
         * overwrite them with the received ones.
         */
        getConfiguration(jsonCfg);

        /* Note:
         * Check only for the key/value pair availability.
         * The type check will follow in the setConfiguration().
         */

        if (false == jsonSensorIndex.isNull())
        {
            jsonCfg["sensorIndex"] = jsonSensorIndex.as<uint8_t>();
            isSuccessful = true;
        }

        if (false == jsonChannelIndex.isNull())
        {
            jsonCfg["channelIndex"] = jsonChannelIndex.as<uint8_t>();
            isSuccessful = true;
        }

        if (true == isSuccessful)
        {
            JsonObjectConst jsonCfgConst = jsonCfg;

            isSuccessful = setConfiguration(jsonCfgConst);

            if (true == isSuccessful)
            {
                requestStoreToPersistentMemory();
            }
        }
    }
    else
    {
        ;
    }

    return isSuccessful;
}

bool SensorPlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    bool                        hasTopicChanged = m_hasTopicChanged;

    /* Only a single topic, therefore its not necessary to check. */
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void SensorPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.init(width, height);

    PluginWithConfig::start(width, height);

    m_sensorChannel = getChannel(m_sensorIdx, m_channelIdx);

    /* Force update. */
    m_updateTimer.start(0U);
}

void SensorPlugin::stop()
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    PluginWithConfig::stop();
}

void SensorPlugin::process(bool isConnected)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    PluginWithConfig::process(isConnected);
}

void SensorPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (true == m_updateTimer.isTimeout())
    {
        update();
        m_updateTimer.start(UPDATE_PERIOD);
    }

    m_view.update(gfx);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void SensorPlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    bool                        isAvailable     = (nullptr != m_sensorChannel)  ? true : false;

    jsonCfg["sensorIndex"]  = m_sensorIdx;
    jsonCfg["channelIndex"] = m_channelIdx;
    jsonCfg["isAvailable"]  = isAvailable;
}

bool SensorPlugin::setConfiguration(JsonObjectConst& jsonCfg)
{
    bool                status              = false;
    JsonVariantConst    jsonSensorIndex     = jsonCfg["sensorIndex"];
    JsonVariantConst    jsonChannelIndex    = jsonCfg["channelIndex"];

    if (false == jsonSensorIndex.is<uint8_t>())
    {
        LOG_WARNING("Sensor index not found or invalid type.");
    }
    else if (false == jsonChannelIndex.is<uint8_t>())
    {
        LOG_WARNING("Channel index not found or invalid type.");
    }
    else
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        m_sensorIdx     = jsonSensorIndex.as<uint8_t>();
        m_channelIdx    = jsonChannelIndex.as<uint8_t>();
        m_sensorChannel = getChannel(m_sensorIdx, m_channelIdx);

        m_hasTopicChanged = true;

        status = true;
    }

    return status;
}

void SensorPlugin::update()
{
    String          text;
    const uint32_t  PRECISION   = 2U;

    if (nullptr == m_sensorChannel)
    {
        text = "{hc}-";
    }
    else
    {
        /* If text is updated and keeps static shown (no scrolling), right
         * aligned looks better, because the unit stays at the same
         * position and only the number seems to be updated.
         */
        text  = "{hr}";
        text += m_sensorChannel->getValueAsString(PRECISION);
        text += " ";
        text += ISensorChannel::channelTypeToUnit(m_sensorChannel->getType());
    }

    m_view.setFormatText(text);
}

ISensorChannel* SensorPlugin::getChannel(uint8_t sensorIdx, uint8_t channelIdx)
{
    ISensor*        sensor  = SensorDataProvider::getInstance().getSensor(sensorIdx);
    ISensorChannel* channel = nullptr;

    if ((nullptr != sensor) &&
        (true == sensor->isAvailable()))
    {
        channel = sensor->getChannel(channelIdx);
    }

    return channel;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

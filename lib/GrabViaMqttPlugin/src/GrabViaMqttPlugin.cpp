/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Grab information via MQTT API plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "GrabViaMqttPlugin.h"

#include <Logging.h>
#include <ArduinoJson.h>
#include <MqttService.h>

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
const char* GrabViaMqttPlugin::TOPIC_CONFIG = "grabConfig";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void GrabViaMqttPlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC_CONFIG);
}

bool GrabViaMqttPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_CONFIG))
    {
        getConfiguration(value);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool GrabViaMqttPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_CONFIG))
    {
        const size_t        JSON_DOC_SIZE = 1024U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg        = jsonDoc.to<JsonObject>();
        JsonVariantConst    jsonPath       = value["path"];
        JsonVariantConst    jsonFilter     = value["filter"];
        JsonVariantConst    jsonIconFileId = value["iconFileId"];
        JsonVariantConst    jsonFormat     = value["format"];
        JsonVariantConst    jsonMultiplier = value["multiplier"];
        JsonVariantConst    jsonOffset     = value["offset"];

        /* The received configuration may not contain all single key/value pair.
         * Therefore read first the complete internal configuration and
         * overwrite them with the received ones.
         */
        getConfiguration(jsonCfg);

        /* Note:
         * Check only for the key/value pair availability.
         * The type check will follow in the setConfiguration().
         */

        if (false == jsonPath.isNull())
        {
            jsonCfg["path"] = jsonPath.as<String>();
            isSuccessful    = true;
        }

        if (false == jsonFilter.isNull())
        {
            if (true == jsonFilter.is<JsonObjectConst>())
            {
                jsonCfg["filter"] = jsonFilter.as<JsonObjectConst>();
                isSuccessful      = true;
            }
            else if (true == jsonFilter.is<JsonArrayConst>())
            {
                jsonCfg["filter"] = jsonFilter.as<JsonArrayConst>();
                isSuccessful      = true;
            }
            else if (true == jsonFilter.is<String>())
            {
                const size_t         JSON_DOC_FILTER_SIZE = 256U;
                DynamicJsonDocument  jsonDocFilter(JSON_DOC_FILTER_SIZE);
                DeserializationError result = deserializeJson(jsonDocFilter, jsonFilter.as<String>());

                if (DeserializationError::Ok == result)
                {
                    if (true == jsonDocFilter.is<JsonObjectConst>())
                    {
                        jsonCfg["filter"] = jsonDocFilter.as<JsonObjectConst>();
                        isSuccessful      = true;
                    }
                    else if (true == jsonDocFilter.is<JsonArrayConst>())
                    {
                        jsonCfg["filter"] = jsonDocFilter.as<JsonArrayConst>();
                        isSuccessful      = true;
                    }
                    else
                    {
                        ;
                    }
                }
            }
            else
            {
                ;
            }
        }

        if (false == jsonIconFileId.isNull())
        {
            jsonCfg["iconFileId"] = jsonIconFileId.as<FileMgrService::FileId>();
            isSuccessful          = true;
        }

        if (false == jsonFormat.isNull())
        {
            jsonCfg["format"] = jsonFormat.as<String>();
            isSuccessful      = true;
        }

        if (false == jsonMultiplier.isNull())
        {
            jsonCfg["multiplier"] = jsonMultiplier.as<float>();
            isSuccessful          = true;
        }

        if (false == jsonOffset.isNull())
        {
            jsonCfg["offset"] = jsonOffset.as<float>();
            isSuccessful      = true;
        }

        if (true == isSuccessful)
        {
            JsonObjectConst jsonCfgConst = jsonCfg;

            isSuccessful                 = setConfiguration(jsonCfgConst);

            if (true == isSuccessful)
            {
                requestStoreToPersistentMemory();
            }
        }
    }

    return isSuccessful;
}

bool GrabViaMqttPlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive> guard(m_mutex);
    bool                       hasTopicChanged = m_hasTopicChanged;

    /* Only a single topic, therefore its not necessary to check. */
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void GrabViaMqttPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.init(width, height);

    PluginWithConfig::start(width, height);

    if (FileMgrService::FILE_ID_INVALID != m_iconFileId)
    {
        String iconFullPath;

        if (false == FileMgrService::getInstance().getFileFullPathById(iconFullPath, m_iconFileId))
        {
            LOG_WARNING("Unknown file id %u.", m_iconFileId);
            m_view.setupTextOnly();
        }
        else if (false == m_view.loadIcon(iconFullPath))
        {
            LOG_ERROR("Icon not found: %s", iconFullPath.c_str());
            m_view.setupTextOnly();
        }
        else
        {
            m_view.setupBitmapAndText();
        }
    }
    else
    {
        m_view.setupTextOnly();
    }

    subscribe();
}

void GrabViaMqttPlugin::stop()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    unsubscribe();

    PluginWithConfig::stop();
}

void GrabViaMqttPlugin::process(bool isConnected)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    PluginWithConfig::process(isConnected);
}

void GrabViaMqttPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.update(gfx);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void GrabViaMqttPlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["path"]       = m_path;
    jsonCfg["filter"]     = m_filter;
    jsonCfg["iconFileId"] = m_iconFileId;
    jsonCfg["format"]     = m_format;
    jsonCfg["multiplier"] = m_multiplier;
    jsonCfg["offset"]     = m_offset;
}

bool GrabViaMqttPlugin::setConfiguration(const JsonObjectConst& jsonCfg)
{
    bool             status         = false;
    JsonVariantConst jsonPath       = jsonCfg["path"];
    JsonVariantConst jsonFilter     = jsonCfg["filter"];
    JsonVariantConst jsonIconFileId = jsonCfg["iconFileId"];
    JsonVariantConst jsonFormat     = jsonCfg["format"];
    JsonVariantConst jsonMultiplier = jsonCfg["multiplier"];
    JsonVariantConst jsonOffset     = jsonCfg["offset"];

    if (false == jsonPath.is<String>())
    {
        LOG_WARNING("JSON path not found or invalid type.");
    }
    else if ((false == jsonFilter.is<JsonObjectConst>()) &&
             (false == jsonFilter.is<JsonArrayConst>()))
    {
        LOG_WARNING("JSON filter not found or invalid type.");
    }
    else if (false == jsonIconFileId.is<FileMgrService::FileId>())
    {
        LOG_WARNING("JSON icon file id not found or invalid type.");
    }
    else if (false == jsonFormat.is<String>())
    {
        LOG_WARNING("JSON format not found or invalid type.");
    }
    else if (false == jsonMultiplier.is<float>())
    {
        LOG_WARNING("JSON multiplier not found or invalid type.");
    }
    else if (false == jsonOffset.is<float>())
    {
        LOG_WARNING("JSON offset not found or invalid type.");
    }
    else
    {
        bool                       reqInit = false;
        bool                       reqIcon = false;
        MutexGuard<MutexRecursive> guard(m_mutex);

        if (m_path != jsonPath.as<String>())
        {
            unsubscribe();
            reqInit = true;
        }

        if (m_iconFileId != jsonIconFileId.as<FileMgrService::FileId>())
        {
            reqIcon = true;
        }

        m_path       = jsonPath.as<String>();
        m_filter     = jsonFilter;
        m_iconFileId = jsonIconFileId.as<FileMgrService::FileId>();
        m_format     = jsonFormat.as<String>();
        m_multiplier = jsonMultiplier.as<float>();
        m_offset     = jsonOffset.as<float>();

        if (true == reqInit)
        {
            subscribe();
        }

        /* Load icon immediately */
        if (true == reqIcon)
        {
            if (FileMgrService::FILE_ID_INVALID != m_iconFileId)
            {
                String iconFullPath;

                if (false == FileMgrService::getInstance().getFileFullPathById(iconFullPath, m_iconFileId))
                {
                    LOG_WARNING("Unknown file id %u.", m_iconFileId);
                    m_view.setupTextOnly();
                }
                else if (false == m_view.loadIcon(iconFullPath))
                {
                    LOG_ERROR("Icon not found: %s", iconFullPath.c_str());
                    m_view.setupTextOnly();
                }
                else
                {
                    m_view.setupBitmapAndText();
                }
            }
            else
            {
                m_view.setupTextOnly();
            }
        }

        m_hasTopicChanged = true;

        status            = true;
    }

    return status;
}

void GrabViaMqttPlugin::getJsonValueByFilter(JsonVariantConst src, JsonVariantConst filter, JsonArray& values)
{
    /* Source type and filter type must always match.
     * If not, it is a configuration error and the values array will be empty.
     */
    if ((true == src.is<JsonObjectConst>()) &&
        (true == filter.is<JsonObjectConst>()))
    {
        /* The filter leads to the required key/value pair. */
        for (JsonPairConst pair : filter.as<JsonObjectConst>())
        {
            /* If the pair value is a JSON object or array, continue traversing. */
            if ((true == pair.value().is<JsonObjectConst>()) ||
                (true == pair.value().is<JsonArrayConst>()))
            {
                getJsonValueByFilter(src[pair.key()], filter[pair.key()], values);
            }
            /* Capture the value from the source, by using the filter pair key. */
            else
            {
                if (false == values.add(src[pair.key()]))
                {
                    break;
                }
            }
        }
    }
    else if ((true == src.is<JsonArrayConst>()) &&
             (true == filter.is<JsonArrayConst>()))
    {
        JsonArrayConst filterArray = filter.as<JsonArrayConst>();

        /* Walk through the source array and capture every required value. */
        for (JsonVariantConst value : src.as<JsonArrayConst>())
        {
            /* If the pair value is a JSON object or array, continue traversing. */
            if ((true == filterArray[0].is<JsonObjectConst>()) ||
                (true == filterArray[0].is<JsonArrayConst>()))
            {
                getJsonValueByFilter(value, filterArray[0], values);
            }
            /* Capture the value from the source. */
            else
            {
                if (false == values.add(value))
                {
                    break;
                }
            }
        }
    }
    else
    {
        /* Configuration error. */
        ;
    }
}

void GrabViaMqttPlugin::subscribe()
{
    MqttService& mqttService = MqttService::getInstance();

    if (false == m_path.isEmpty())
    {
        (void)mqttService.subscribe(m_path,
            [this](const String& topic, const uint8_t* payload, size_t size) {
                this->mqttTopicCallback(topic, payload, size);
            });
    }
}

void GrabViaMqttPlugin::unsubscribe()
{
    MqttService& mqttService = MqttService::getInstance();

    if (false == m_path.isEmpty())
    {
        mqttService.unsubscribe(m_path);
    }
}

void GrabViaMqttPlugin::mqttTopicCallback(const String& topic, const uint8_t* payload, size_t size)
{
    const size_t         JSON_DOC_SIZE = 1024U;
    DynamicJsonDocument  jsonDoc(JSON_DOC_SIZE);
    DeserializationError error = deserializeJson(jsonDoc, payload, size);

    if (DeserializationError::Ok != error)
    {
        LOG_WARNING("MQTT payload contains invalid JSON.");
    }
    else
    {
        DynamicJsonDocument jsonDocValues(JSON_DOC_SIZE);
        JsonArray           jsonValuesArray = jsonDocValues.to<JsonArray>();
        size_t              index           = 0U;
        String              outputStr;
        size_t              valueCount = 0U;

        getJsonValueByFilter(jsonDoc, m_filter, jsonValuesArray);
        valueCount = jsonValuesArray.size();

        if (true == jsonDocValues.overflowed())
        {
            LOG_ERROR("Less memory for JSON values available.");

            /* The last value may be corrupt, throw it away and show the rest. */
            if (0U < valueCount)
            {
                --valueCount;
            }
        }

        for (index = 0U; index < valueCount; ++index)
        {
            JsonVariantConst jsonValue = jsonValuesArray[index];

            if (0U < index)
            {
                outputStr += m_delimiter;
            }

            /* Is it a number and format string doesn't contain a '%s'? */
            if ((true == jsonValue.is<float>()) &&
                (0 > m_format.indexOf("%s"))) /* Prevent mistake which may cause a LoadProhibited core panic by snprintf. */
            {
                const size_t BUFFER_SIZE = 128U;
                char         buffer[BUFFER_SIZE];
                float        value  = jsonValue.as<float>();

                value              *= m_multiplier;
                value              += m_offset;

                (void)snprintf(buffer, sizeof(buffer), m_format.c_str(), value);

                outputStr += buffer;
            }
            /* Is it a string? */
            else if (true == jsonValue.is<String>())
            {
                const size_t BUFFER_SIZE = 128U;
                char         buffer[BUFFER_SIZE];

                (void)snprintf(buffer, sizeof(buffer), m_format.c_str(), jsonValue.as<String>().c_str());

                outputStr += buffer;
            }
            else
            {
                outputStr += "?";
            }
        }

        if (true == outputStr.isEmpty())
        {
            outputStr = "{hc}-";
        }

        LOG_INFO("Grabbed: %s", outputStr.c_str());

        m_view.setFormatText(outputStr);
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

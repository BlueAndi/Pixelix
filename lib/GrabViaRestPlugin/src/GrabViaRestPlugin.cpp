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
 * @file   GrabViaRestPlugin.cpp
 * @brief  Grab information via REST plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "GrabViaRestPlugin.h"

#include <Logging.h>
#include <ArduinoJson.h>
#include <HttpStatus.h>
#include <Util.h>

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
const char* GrabViaRestPlugin::TOPIC_CONFIG = "grabConfig";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool GrabViaRestPlugin::isEnabled() const
{
    bool isEnabled = false;

    /* The plugin shall only be scheduled if its enabled and text is set. */
    if ((true == m_isEnabled) &&
        (false == m_view.getText().isEmpty()))
    {
        isEnabled = true;
    }

    return isEnabled;
}

void GrabViaRestPlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC_CONFIG);
}

bool GrabViaRestPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_CONFIG))
    {
        getConfiguration(value);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool GrabViaRestPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_CONFIG))
    {
        const size_t        JSON_DOC_SIZE = 1024U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg        = jsonDoc.to<JsonObject>();
        JsonVariantConst    jsonMethod     = value["method"];
        JsonVariantConst    jsonUrl        = value["url"];
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

        if (false == jsonMethod.isNull())
        {
            jsonCfg["method"] = jsonMethod.as<const char*>();
            isSuccessful      = true;
        }

        if (false == jsonUrl.isNull())
        {
            jsonCfg["url"] = jsonUrl.as<const char*>();
            isSuccessful   = true;
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
                DeserializationError result = deserializeJson(jsonDocFilter, jsonFilter.as<const char*>());

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
            jsonCfg["format"] = jsonFormat.as<const char*>();
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

bool GrabViaRestPlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive> guard(m_mutex);
    bool                       hasTopicChanged = m_hasTopicChanged;

    /* Only a single topic, therefore its not necessary to check. */
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void GrabViaRestPlugin::start(uint16_t width, uint16_t height)
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
        }
        else if (false == m_view.loadIcon(iconFullPath))
        {
            LOG_ERROR("Icon not found: %s", iconFullPath.c_str());
        }
        else
        {
            ;
        }
    }
}

void GrabViaRestPlugin::stop()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_requestTimer.stop();

    PluginWithConfig::stop();

    m_isAllowedToSend = false;

    if (RestService::INVALID_REST_ID != m_dynamicRestId)
    {
        RestService::getInstance().abortRequest(m_dynamicRestId);
        m_dynamicRestId = RestService::INVALID_REST_ID;
    }
}

void GrabViaRestPlugin::process(bool isConnected)
{
    MutexGuard<MutexRecursive> guard(m_mutex);
    DynamicJsonDocument        jsonDoc(0U);
    bool                       isValidResponse;

    PluginWithConfig::process(isConnected);

    /* Only if a network connection is established, the required information
     * shall be periodically requested via REST API.
     */
    if (false == m_requestTimer.isTimerRunning())
    {
        if (true == isConnected)
        {
            /* Only one request can be sent at a time. */
            if (true == m_isAllowedToSend)
            {
                if (false == startHttpRequest())
                {
                    /* If a request fails, a '?' will be shown. */
                    m_view.setFormatText("{hc}?");

                    m_requestTimer.start(UPDATE_PERIOD_SHORT);
                }
                else
                {
                    m_requestTimer.start(UPDATE_PERIOD);
                    m_isAllowedToSend = false;
                }
            }
        }
    }
    else
    {
        /* If the connection is lost, stop periodically requesting information
         * via REST API.
         */
        if (false == isConnected)
        {
            m_requestTimer.stop();
        }
        /* Network connection is available and next request may be necessary for
         * information update.
         */
        else if (true == m_requestTimer.isTimeout())
        {
            /* Only one request can be sent at a time. */
            if (true == m_isAllowedToSend)
            {
                if (false == startHttpRequest())
                {
                    /* If a request fails, a '?' will be shown. */
                    m_view.setFormatText("{hc}?");

                    m_requestTimer.start(UPDATE_PERIOD_SHORT);
                }
                else
                {
                    m_requestTimer.start(UPDATE_PERIOD);
                    m_isAllowedToSend = false;
                }
            }
        }
    }

    if (RestService::INVALID_REST_ID != m_dynamicRestId)
    {
        /* Get the response from the REST service. */
        if (true == RestService::getInstance().getResponse(m_dynamicRestId, isValidResponse, jsonDoc))
        {
            if (true == isValidResponse)
            {
                handleWebResponse(jsonDoc);
            }
            else
            {
                LOG_WARNING("Connection error.");

                /* If a request fails, show standard icon and a '?' */
                m_view.setFormatText("{hc}?");

                m_requestTimer.start(UPDATE_PERIOD_SHORT);
            }

            m_dynamicRestId   = RestService::INVALID_REST_ID;
            m_isAllowedToSend = true;
        }
    }
}

void GrabViaRestPlugin::update(YAGfx& gfx)
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

void GrabViaRestPlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["method"]     = m_method;
    jsonCfg["url"]        = m_url;
    jsonCfg["filter"]     = m_filter;
    jsonCfg["iconFileId"] = m_iconFileId;
    jsonCfg["format"]     = m_format;
    jsonCfg["multiplier"] = m_multiplier;
    jsonCfg["offset"]     = m_offset;
}

bool GrabViaRestPlugin::setConfiguration(const JsonObjectConst& jsonCfg)
{
    bool             status         = false;
    JsonVariantConst jsonMethod     = jsonCfg["method"];
    JsonVariantConst jsonUrl        = jsonCfg["url"];
    JsonVariantConst jsonFilter     = jsonCfg["filter"];
    JsonVariantConst jsonIconFileId = jsonCfg["iconFileId"];
    JsonVariantConst jsonFormat     = jsonCfg["format"];
    JsonVariantConst jsonMultiplier = jsonCfg["multiplier"];
    JsonVariantConst jsonOffset     = jsonCfg["offset"];

    if (false == jsonMethod.is<String>())
    {
        LOG_WARNING("JSON method not found or invalid type.");
    }
    else if (false == jsonUrl.is<String>())
    {
        LOG_WARNING("JSON URL not found or invalid type.");
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
        MutexGuard<MutexRecursive> guard(m_mutex);
        FileMgrService::FileId     newIconFileId = jsonIconFileId.as<FileMgrService::FileId>();

        m_method                                 = jsonMethod.as<const char*>();
        m_url                                    = jsonUrl.as<const char*>();
        m_filter                                 = jsonFilter;
        m_format                                 = jsonFormat.as<const char*>();
        m_multiplier                             = jsonMultiplier.as<float>();
        m_offset                                 = jsonOffset.as<float>();

        if (m_iconFileId != newIconFileId)
        {
            m_iconFileId = newIconFileId;

            if (FileMgrService::FILE_ID_INVALID == m_iconFileId)
            {
                m_view.clearIcon();
            }
            else
            {
                String iconFullPath;

                if (false == FileMgrService::getInstance().getFileFullPathById(iconFullPath, m_iconFileId))
                {
                    LOG_WARNING("Unknown file id %u.", m_iconFileId);
                    m_view.clearIcon();
                }
                else
                {
                    if (false == m_view.loadIcon(iconFullPath))
                    {
                        LOG_WARNING("Couldn't load icon: %s", iconFullPath.c_str());
                    }
                }
            }
        }

        /* Force update on display */
        m_requestTimer.start(UPDATE_PERIOD_SHORT);

        m_hasTopicChanged = true;

        status            = true;
    }

    return status;
}

bool GrabViaRestPlugin::startHttpRequest()
{
    bool                            status = false;
    RestService::PreProcessCallback preProcessCallback =
        [this](const char* payload, size_t size, DynamicJsonDocument& doc) {
            return this->preProcessAsyncWebResponse(payload, size, doc);
        };

    if (false == m_url.isEmpty())
    {
        if (true == m_method.equalsIgnoreCase("GET"))
        {
            m_dynamicRestId = RestService::getInstance().get(m_url, preProcessCallback);

            if (RestService::INVALID_REST_ID == m_dynamicRestId)
            {
                LOG_WARNING("GET %s failed.", m_url.c_str());
            }
            else
            {
                status = true;
            }
        }
        else if (true == m_method.equalsIgnoreCase("POST"))
        {
            m_dynamicRestId = RestService::getInstance().post(m_url, preProcessCallback);

            if (RestService::INVALID_REST_ID == m_dynamicRestId)
            {
                LOG_WARNING("POST %s failed.", m_url.c_str());
            }
            else
            {
                status = true;
            }
        }
        else
        {
            LOG_WARNING("Invalid HTTP method %s.", m_method.c_str());
        }
    }

    return status;
}

bool GrabViaRestPlugin::preProcessAsyncWebResponse(const char* payload, size_t payloadSize, DynamicJsonDocument& jsonDoc)
{
    bool                       isSuccessful = false;
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (true == m_filter.overflowed())
    {
        LOG_ERROR("JSON document size exceeded.");
    }
    else
    {
        DeserializationError error = deserializeJson(jsonDoc, payload, payloadSize, DeserializationOption::Filter(m_filter));

        if (DeserializationError::Ok != error.code())
        {
            LOG_WARNING("JSON parse error: %s", error.c_str());
        }
        else
        {
            isSuccessful = true;
        }
    }

    return isSuccessful;
}

void GrabViaRestPlugin::getJsonValueByFilter(JsonVariantConst src, JsonVariantConst filter, JsonArray& values)
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

void GrabViaRestPlugin::handleWebResponse(const DynamicJsonDocument& jsonDoc)
{
    const size_t        JSON_DOC_SIZE = 1024U;
    DynamicJsonDocument jsonDocValues(JSON_DOC_SIZE);
    JsonArray           jsonValuesArray = jsonDocValues.to<JsonArray>();
    size_t              index           = 0U;
    String              outputStr;
    size_t              valueCount = 0U;

    /* Protect against concurrent access. */
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        getJsonValueByFilter(jsonDoc, m_filter, jsonValuesArray);
    }

    valueCount = jsonValuesArray.size();

    if (true == jsonDocValues.overflowed())
    {
        LOG_ERROR("JSON document size exceeded.");

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
            (false == Util::isFormatSpecifierInStr(m_format, 's'))) /* Prevent mistake which may cause a LoadProhibited core panic by snprintf. */
        {
            const size_t BUFFER_SIZE = 128U;
            char         buffer[BUFFER_SIZE];
            float        value = jsonValue.as<float>();

            /* Is it not a number? */
            if (true == std::isnan(value))
            {
                outputStr += "!";
            }
            else
            {
                value *= m_multiplier;
                value += m_offset;

                (void)snprintf(buffer, sizeof(buffer), m_format.c_str(), value);

                outputStr += buffer;
            }
        }
        /* Is it a string and should be converted to a floating point number? */
        else if ((true == jsonValue.is<String>()) &&
                 (true == Util::isFormatSpecifierInStr(m_format, 'f')))
        {
            const size_t BUFFER_SIZE = 128U;
            char         buffer[BUFFER_SIZE];
            float        value = jsonValue.as<String>().toFloat();

            /* Is it not a number? */
            if (true == std::isnan(value))
            {
                outputStr += "!";
            }
            else
            {
                value *= m_multiplier;
                value += m_offset;

                (void)snprintf(buffer, sizeof(buffer), m_format.c_str(), value);

                outputStr += buffer;
            }
        }
        /* Is it a string? */
        else if (true == jsonValue.is<String>())
        {
            const size_t BUFFER_SIZE = 128U;
            char         buffer[BUFFER_SIZE];

            (void)snprintf(buffer, sizeof(buffer), m_format.c_str(), jsonValue.as<const char*>());

            outputStr += buffer;
        }
        else
        {
            outputStr += "?";
        }
    }

    LOG_INFO("Grabbed: %s", outputStr.c_str());

    m_view.setFormatText(outputStr);
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

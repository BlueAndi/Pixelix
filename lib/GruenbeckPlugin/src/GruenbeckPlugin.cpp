/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   GruenbeckPlugin.cpp
 * @brief  Gruenbeck plugin.
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "GruenbeckPlugin.h"

#include <ArduinoJson.h>
#include <Logging.h>
#include <HttpStatus.h>

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
const char* GruenbeckPlugin::TOPIC_CONFIG = "ipAddress";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void GruenbeckPlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC_CONFIG);
}

bool GruenbeckPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_CONFIG))
    {
        getConfiguration(value);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool GruenbeckPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_CONFIG))
    {
        const size_t        JSON_DOC_SIZE = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg = jsonDoc.to<JsonObject>();
        String              ipAddress;
        JsonVariantConst    jsonIpAddress = value["ipAddress"];

        /* The received configuration may not contain all single key/value pair.
         * Therefore read first the complete internal configuration and
         * overwrite them with the received ones.
         */
        getConfiguration(jsonCfg);

        /* Note:
         * Check only for the key/value pair availability.
         * The type check will follow in the setConfiguration().
         */

        if (false == jsonIpAddress.isNull())
        {
            jsonCfg["ipAddress"] = jsonIpAddress.as<const char*>();
            isSuccessful         = true;
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

bool GruenbeckPlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive> guard(m_mutex);
    bool                       hasTopicChanged = m_hasTopicChanged;

    /* Only a single topic, therefore its not necessary to check. */
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void GruenbeckPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.init(width, height);

    PluginWithConfig::start(width, height);
}

void GruenbeckPlugin::stop()
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

void GruenbeckPlugin::process(bool isConnected)
{
    MutexGuard<MutexRecursive> guard(m_mutex);
    DynamicJsonDocument        jsonDoc(0U);
    bool                       isValidResponse;

    PluginWithConfig::process(isConnected);

    /* Only if a network connection is established the required information
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
                    /* If a request fails, show standard icon and a '?' */
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
                    /* If a request fails, show standard icon and a '?' */
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

void GruenbeckPlugin::update(YAGfx& gfx)
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

void GruenbeckPlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["ipAddress"] = m_ipAddress;
}

bool GruenbeckPlugin::setConfiguration(const JsonObjectConst& jsonCfg)
{
    bool             status        = false;
    JsonVariantConst jsonIpAddress = jsonCfg["ipAddress"];

    if (false == jsonIpAddress.is<String>())
    {
        LOG_WARNING("JSON ipAddress not found or invalid type.");
    }
    else
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        m_ipAddress = jsonIpAddress.as<const char*>();

        /* Force update on display */
        m_requestTimer.start(UPDATE_PERIOD_SHORT);

        m_hasTopicChanged = true;

        status            = true;
    }

    return status;
}

bool GruenbeckPlugin::startHttpRequest()
{
    bool                            status = false;
    RestService::PreProcessCallback preProcessCallback =
        [this](const char* payload, size_t size, DynamicJsonDocument& doc) {
            return this->preProcessAsyncWebResponse(payload, size, doc);
        };

    if (false == m_ipAddress.isEmpty())
    {
        String url      = String("http://") + m_ipAddress + "/mux_http?id=42&show=D_Y_10_1~";

        m_dynamicRestId = RestService::getInstance().get(url, preProcessCallback);

        if (RestService::INVALID_REST_ID == m_dynamicRestId)
        {
            LOG_WARNING("GET %s failed.", url.c_str());
        }
        else
        {
            status = true;
        }
    }

    return status;
}

bool GruenbeckPlugin::preProcessAsyncWebResponse(const char* payload, size_t payloadSize, DynamicJsonDocument& jsonDoc)
{
    /* Structure of response-payload for requesting D_Y_10_1
     *
     * <data><code>ok</code><D_Y_10_1>XYZ</D_Y_10_1></data>
     *
     * <data><code>ok</code><D_Y_10_1>  = 31 bytes
     * XYZ                              = 3 byte (relevant data)
     * </D_Y_10_1></data>               = 18 bytes
     */

    /* Start index of relevant data */
    const uint32_t START_INDEX_OF_RELEVANT_DATA = 31U;

    /* Length of relevant data */
    const uint32_t RELEVANT_DATA_LENGTH         = 3U;

    bool           isSuccessful                 = false;
    char           restCapacity[RELEVANT_DATA_LENGTH + 1];

    if ((nullptr != payload) &&
        (payloadSize >= (START_INDEX_OF_RELEVANT_DATA + RELEVANT_DATA_LENGTH)))
    {
        memcpy(restCapacity, &payload[START_INDEX_OF_RELEVANT_DATA], RELEVANT_DATA_LENGTH);
        restCapacity[RELEVANT_DATA_LENGTH] = '\0';
        jsonDoc["restCapacity"]            = restCapacity;
        isSuccessful                       = true;
    }

    return isSuccessful;
}

void GruenbeckPlugin::handleWebResponse(const DynamicJsonDocument& jsonDoc)
{
    JsonVariantConst jsonRestCapacity = jsonDoc["restCapacity"];

    if (false == jsonRestCapacity.is<String>())
    {
        LOG_WARNING("JSON rest capacity mismatch or missing.");
    }
    else
    {
        String restCapacity  = "{hc}";

        restCapacity        += jsonRestCapacity.as<const char*>();
        restCapacity        += "%";

        m_view.setFormatText(restCapacity);
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

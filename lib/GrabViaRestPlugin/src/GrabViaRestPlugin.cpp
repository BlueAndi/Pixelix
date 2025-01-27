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
        const size_t        JSON_DOC_SIZE           = 1024U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg                 = jsonDoc.to<JsonObject>();
        JsonVariantConst    jsonMethod              = value["method"];
        JsonVariantConst    jsonUrl                 = value["url"];
        JsonVariantConst    jsonFilter              = value["filter"];
        JsonVariantConst    jsonIconFileId          = value["iconFileId"];
        JsonVariantConst    jsonFormat              = value["format"];
        JsonVariantConst    jsonMultiplier          = value["multiplier"];
        JsonVariantConst    jsonOffset              = value["offset"];

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
            jsonCfg["method"] = jsonMethod.as<String>();
            isSuccessful = true;
        }

        if (false == jsonUrl.isNull())
        {
            jsonCfg["url"] = jsonUrl.as<String>();
            isSuccessful = true;
        }

        if (false == jsonFilter.isNull())
        {
            if (true == jsonFilter.is<JsonObjectConst>())
            {
                jsonCfg["filter"] = jsonFilter.as<JsonObjectConst>();
                isSuccessful = true;
            }
            else if (true == jsonFilter.is<String>())
            {
                const size_t            JSON_DOC_FILTER_SIZE    = 256U;
                DynamicJsonDocument     jsonDocFilter(JSON_DOC_FILTER_SIZE);
                DeserializationError    result = deserializeJson(jsonDocFilter, jsonFilter.as<String>());

                if (DeserializationError::Ok == result)
                {
                    jsonCfg["filter"] = jsonDocFilter.as<JsonObjectConst>();
                    isSuccessful = true;
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
            isSuccessful = true;
        }

        if (false == jsonFormat.isNull())
        {
            jsonCfg["format"] = jsonFormat.as<String>();
            isSuccessful = true;
        }

        if (false == jsonMultiplier.isNull())
        {
            jsonCfg["multiplier"] = jsonMultiplier.as<float>();
            isSuccessful = true;
        }

        if (false == jsonOffset.isNull())
        {
            jsonCfg["offset"] = jsonOffset.as<float>();
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

    return isSuccessful;
}

bool GrabViaRestPlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    bool                        hasTopicChanged = m_hasTopicChanged;

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

    initHttpClient();
}

void GrabViaRestPlugin::stop()
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_requestTimer.stop();

    PluginWithConfig::stop();
}

void GrabViaRestPlugin::process(bool isConnected)
{
    Msg                         msg;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    PluginWithConfig::process(isConnected);

    /* Only if a network connection is established the required information
     * shall be periodically requested via REST API.
     */
    if (false == m_requestTimer.isTimerRunning())
    {
        if (true == isConnected)
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
            if (false == startHttpRequest())
            {
                /* If a request fails, a '?' will be shown. */
                m_view.setFormatText("{hc}?");

                m_requestTimer.start(UPDATE_PERIOD_SHORT);
            }
            else
            {
                m_requestTimer.start(UPDATE_PERIOD);
            }
        }
    }

    if (true == m_taskProxy.receive(msg))
    {
        switch(msg.type)
        {
        case MSG_TYPE_INVALID:
            /* Should never happen. */
            break;

        case MSG_TYPE_RSP:
            if (nullptr != msg.rsp)
            {
                handleWebResponse(*msg.rsp);
                delete msg.rsp;
                msg.rsp = nullptr;
            }
            break;

        case MSG_TYPE_CONN_CLOSED:
            LOG_INFO("Connection closed.");

            if (true == m_isConnectionError)
            {
                /* If a request fails, show standard icon and a '?' */
                m_view.setFormatText("{hc}?");

                m_requestTimer.start(UPDATE_PERIOD_SHORT);
            }
            m_isConnectionError = false;
            break;

        case MSG_TYPE_CONN_ERROR:
            LOG_WARNING("Connection error.");
            m_isConnectionError = true;
            break;

        default:
            /* Should never happen. */
            break;
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

    jsonCfg["method"]       = m_method;
    jsonCfg["url"]          = m_url;
    jsonCfg["filter"]       = m_filter;
    jsonCfg["iconFileId"]   = m_iconFileId;
    jsonCfg["format"]       = m_format;
    jsonCfg["multiplier"]   = m_multiplier;
    jsonCfg["offset"]       = m_offset;
}

bool GrabViaRestPlugin::setConfiguration(const JsonObjectConst& jsonCfg)
{
    bool                status          = false;
    JsonVariantConst    jsonMethod      = jsonCfg["method"];
    JsonVariantConst    jsonUrl         = jsonCfg["url"];
    JsonVariantConst    jsonFilter      = jsonCfg["filter"];
    JsonVariantConst    jsonIconFileId  = jsonCfg["iconFileId"];
    JsonVariantConst    jsonFormat      = jsonCfg["format"];
    JsonVariantConst    jsonMultiplier  = jsonCfg["multiplier"];
    JsonVariantConst    jsonOffset      = jsonCfg["offset"];

    if (false == jsonMethod.is<String>())
    {
        LOG_WARNING("JSON method not found or invalid type.");
    }
    else if (false == jsonUrl.is<String>())
    {
        LOG_WARNING("JSON URL not found or invalid type.");
    }
    else if (false == jsonFilter.is<JsonObjectConst>())
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
        bool                        reqIcon = false;
        MutexGuard<MutexRecursive>  guard(m_mutex);

        if (m_iconFileId != jsonIconFileId.as<FileMgrService::FileId>())
        {
            reqIcon = true;
        }

        m_method        = jsonMethod.as<String>();
        m_url           = jsonUrl.as<String>();
        m_filter        = jsonFilter.as<JsonObjectConst>();
        m_iconFileId    = jsonIconFileId.as<FileMgrService::FileId>();
        m_format        = jsonFormat.as<String>();
        m_multiplier    = jsonMultiplier.as<float>();
        m_offset        = jsonOffset.as<float>();

        /* Force update on display */
        m_requestTimer.start(UPDATE_PERIOD_SHORT);

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

        status = true;
    }

    return status;
}

bool GrabViaRestPlugin::startHttpRequest()
{
    bool status = false;

    if (false == m_url.isEmpty())
    {
        if (true == m_client.begin(m_url))
        {
            if (true == m_method.equalsIgnoreCase("GET"))
            {
                if (false == m_client.GET())
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
                if (false == m_client.POST())
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
    }

    return status;
}

void GrabViaRestPlugin::initHttpClient()
{
    /* Note: All registered callbacks are running in a different task context!
     *       Therefore it is not allowed to access a member here directly.
     *       The processing must be deferred via task proxy.
     */
    m_client.regOnResponse(
        [this](const HttpResponse& rsp)
        {
            handleAsyncWebResponse(rsp);
        }
    );

    m_client.regOnClosed(
        [this]()
        {
            Msg msg;

            msg.type = MSG_TYPE_CONN_CLOSED;

            (void)this->m_taskProxy.send(msg);
        }
    );

    m_client.regOnError(
        [this]()
        {
            Msg msg;

            msg.type = MSG_TYPE_CONN_ERROR;

            (void)this->m_taskProxy.send(msg);
        }
    );
}

void GrabViaRestPlugin::handleAsyncWebResponse(const HttpResponse& rsp)
{
    if (HttpStatus::STATUS_CODE_OK == rsp.getStatusCode())
    {
        const size_t            JSON_DOC_SIZE   = 512U;
        DynamicJsonDocument*    jsonDoc         = new(std::nothrow) DynamicJsonDocument(JSON_DOC_SIZE);

        if (nullptr != jsonDoc)
        {
            bool                    isSuccessful    = false;
            size_t                  payloadSize     = 0U;
            const void*             vPayload        = rsp.getPayload(payloadSize);
            const char*             payload         = static_cast<const char*>(vPayload);
            
            if (true == m_filter.overflowed())
            {
                LOG_ERROR("Less memory for filter available.");
            }
            else if ((nullptr == payload) ||
                     (0U == payloadSize))
            {
                LOG_ERROR("No payload.");
            }
            else
            {
                DeserializationError error = deserializeJson(*jsonDoc, payload, payloadSize, DeserializationOption::Filter(m_filter));

                if (DeserializationError::Ok != error.code())
                {
                    LOG_WARNING("JSON parse error: %s", error.c_str());
                }
                else
                {
                    Msg msg;

                    msg.type    = MSG_TYPE_RSP;
                    msg.rsp     = jsonDoc;

                    isSuccessful = this->m_taskProxy.send(msg);
                }
            }

            if (false == isSuccessful)
            {
                delete jsonDoc;
                jsonDoc = nullptr;
            }
        }
    }
}

void GrabViaRestPlugin::getJsonValueByFilter(JsonObjectConst src, JsonObjectConst filter, JsonVariantConst& value)
{
    for (JsonPairConst pair : filter)
    {
        if (true == pair.value().is<JsonObjectConst>())
        {
            getJsonValueByFilter(src[pair.key()], filter[pair.key()], value);
        }
        else
        {
            value = src[pair.key()];
        }

        /* Break immediately as its assumed that the filter only contains one
         * single object.
         */
        break;
    }
}

void GrabViaRestPlugin::handleWebResponse(const DynamicJsonDocument& jsonDoc)
{
    JsonVariantConst jsonValue;

    getJsonValueByFilter(jsonDoc.as<JsonObjectConst>(), m_filter.as<JsonObjectConst>(), jsonValue);

    /* Is it a number? */
    if ((true == jsonValue.is<float>()) &&
        (0 > m_format.indexOf("%s"))) /* Prevent mistake which may cause a LoadProhibited core panic by snprintf. */
    {
        const size_t    BUFFER_SIZE = 128U;
        char            buffer[BUFFER_SIZE];
        float           value = jsonValue.as<float>();

        value *= m_multiplier;
        value += m_offset;

        (void)snprintf(buffer, sizeof(buffer), m_format.c_str(), value);

        m_view.setFormatText(buffer);
    }
    /* Is it a string? */
    else if (true == jsonValue.is<String>())
    {
        const size_t    BUFFER_SIZE = 128U;
        char            buffer[BUFFER_SIZE];

        (void)snprintf(buffer, sizeof(buffer), m_format.c_str(), jsonValue.as<String>().c_str());

        m_view.setFormatText(buffer);
    }
    else
    {
        m_view.setFormatText("{hc}-");
    }
}

void GrabViaRestPlugin::clearQueue()
{
    Msg msg;

    while(true == m_taskProxy.receive(msg))
    {
        if (MSG_TYPE_RSP == msg.type)
        {
            delete msg.rsp;
            msg.rsp = nullptr;
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

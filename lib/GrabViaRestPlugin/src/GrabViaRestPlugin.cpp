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
 * @brief  Grab information via MQTT plugin
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
const char* GrabViaRestPlugin::TOPIC_CONFIG = "/grabConfig";

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

    if (0U != topic.equals(TOPIC_CONFIG))
    {
        getConfiguration(value);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool GrabViaRestPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CONFIG))
    {
        const size_t        JSON_DOC_SIZE           = 1024U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg                 = jsonDoc.to<JsonObject>();
        JsonVariantConst    jsonMethod              = value["method"];
        JsonVariantConst    jsonUrl                 = value["url"];
        JsonVariantConst    jsonFilter              = value["filter"];
        JsonVariantConst    jsonIconPath            = value["iconPath"];
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

        if (false == jsonIconPath.isNull())
        {
            jsonCfg["iconPath"] = jsonIconPath.as<String>();
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

    m_layoutLeft.setPosAndSize(0, 0, ICON_WIDTH, ICON_HEIGHT);
    (void)m_layoutLeft.addWidget(m_iconWidget);

    /* The text canvas is left aligned to the icon canvas and it spans over
     * the whole display height.
     */
    m_layoutRight.setPosAndSize(ICON_WIDTH, 0, width - ICON_WIDTH, height);
    (void)m_layoutRight.addWidget(m_textWidgetRight);

    /* If only text is used, it will span over the whole display. */
    m_layoutTextOnly.setPosAndSize(0, 0, width, height);
    (void)m_layoutTextOnly.addWidget(m_textWidgetTextOnly);

    /* Choose font. */
    m_textWidgetRight.setFont(Fonts::getFontByType(m_fontType));
    m_textWidgetTextOnly.setFont(Fonts::getFontByType(m_fontType));

    /* The text widget inside the text canvas is left aligned on x-axis and
     * aligned to the center of y-axis.
     */
    if (height > m_textWidgetRight.getFont().getHeight())
    {
        uint16_t diffY = height - m_textWidgetRight.getFont().getHeight();
        uint16_t offsY = diffY / 2U;

        m_textWidgetRight.move(0, offsY);
        m_textWidgetTextOnly.move(0, offsY);
    }

    /* Try to load configuration. If there is no configuration available, a default configuration
     * will be created.
     */
    if (false == loadConfiguration())
    {
        if (false == saveConfiguration())
        {
            LOG_WARNING("Failed to create initial configuration file %s.", getFullPathToConfiguration().c_str());
        }
    }
    else
    {
        /* Remember current timestamp to detect updates of the configuration in the
         * filesystem without using the plugin API.
         */
        updateTimestampLastUpdate();
    }

    if (false == m_iconPath.isEmpty())
    {
        (void)m_iconWidget.load(FILESYSTEM, m_iconPath);
    }

    m_cfgReloadTimer.start(CFG_RELOAD_PERIOD);

    initHttpClient();
}

void GrabViaRestPlugin::stop()
{
    String                      configurationFilename = getFullPathToConfiguration();
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_cfgReloadTimer.stop();
    m_requestTimer.stop();

    if (false != FILESYSTEM.remove(configurationFilename))
    {
        LOG_INFO("File %s removed", configurationFilename.c_str());
    }
}

void GrabViaRestPlugin::process(bool isConnected)
{
    Msg                         msg;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    /* Configuration in persistent memory updated? */
    if ((true == m_cfgReloadTimer.isTimerRunning()) &&
        (true == m_cfgReloadTimer.isTimeout()))
    {
        if (true == isConfigurationUpdated())
        {
            m_reloadConfigReq = true;
        }

        m_cfgReloadTimer.restart();
    }

    if (true == m_storeConfigReq)
    {
        if (false == saveConfiguration())
        {
            LOG_WARNING("Failed to save configuration: %s", getFullPathToConfiguration().c_str());
        }

        m_storeConfigReq = false;
    }
    else if (true == m_reloadConfigReq)
    {
        LOG_INFO("Reload configuration: %s", getFullPathToConfiguration().c_str());

        if (true == loadConfiguration())
        {
            updateTimestampLastUpdate();
        }

        m_reloadConfigReq = false;
    }
    else
    {
        ;
    }

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
                m_textWidgetRight.setFormatStr("\\calign?");
                m_textWidgetTextOnly.setFormatStr("\\calign?");

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
                m_textWidgetRight.setFormatStr("\\calign?");
                m_textWidgetTextOnly.setFormatStr("\\calign?");

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
                m_textWidgetRight.setFormatStr("\\calign?");
                m_textWidgetTextOnly.setFormatStr("\\calign?");

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

    gfx.fillScreen(ColorDef::BLACK);

    /* If a icon is available, the icon/text layout will be used otherwise the text only layout. */
    if (false == m_iconPath.isEmpty())
    {
        m_layoutLeft.update(gfx);
        m_layoutRight.update(gfx);
    }
    else
    {
        m_layoutTextOnly.update(gfx);
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void GrabViaRestPlugin::requestStoreToPersistentMemory()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_storeConfigReq = true;
}

void GrabViaRestPlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["method"]       = m_method;
    jsonCfg["url"]          = m_url;
    jsonCfg["filter"]       = m_filter;
    jsonCfg["iconPath"]     = m_iconPath;
    jsonCfg["format"]       = m_format;
    jsonCfg["multiplier"]   = m_multiplier;
    jsonCfg["offset"]       = m_offset;
}

bool GrabViaRestPlugin::setConfiguration(JsonObjectConst& jsonCfg)
{
    bool                status          = false;
    JsonVariantConst    jsonMethod      = jsonCfg["method"];
    JsonVariantConst    jsonUrl         = jsonCfg["url"];
    JsonVariantConst    jsonFilter      = jsonCfg["filter"];
    JsonVariantConst    jsonIconPath    = jsonCfg["iconPath"];
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
    else if (false == jsonIconPath.is<String>())
    {
        LOG_WARNING("JSON icon path not found or invalid type.");
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

        if (m_iconPath != jsonIconPath.as<String>())
        {
            reqIcon = true;
        }

        m_method        = jsonMethod.as<String>();
        m_url           = jsonUrl.as<String>();
        m_filter        = jsonFilter.as<JsonObjectConst>();
        m_iconPath      = jsonIconPath.as<String>();
        m_format        = jsonFormat.as<String>();
        m_multiplier    = jsonMultiplier.as<float>();
        m_offset        = jsonOffset.as<float>();

        /* Force update on display */
        m_requestTimer.start(UPDATE_PERIOD_SHORT);

        /* Load icon immediately */
        if (true == reqIcon)
        {
            if (true == m_iconPath.endsWith(".sprite"))
            {
                String textureFileName = m_iconPath;

                textureFileName.replace(".sprite", ".bmp");

                if (false == m_iconWidget.loadSpriteSheet(FILESYSTEM, m_iconPath, textureFileName))
                {
                    LOG_WARNING("Failed to load animation %s / %s.", m_iconPath.c_str(), textureFileName.c_str());
                }
            }
            else if (true == m_iconPath.endsWith(".bmp"))
            {
                if (false == m_iconWidget.load(FILESYSTEM, m_iconPath))
                {
                    LOG_WARNING("Failed to load bitmap %s.", m_iconPath.c_str());
                }
            }
            else
            {
                m_iconWidget.clear(ColorDef::BLACK);
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
            size_t                  payloadSize = 0U;
            const void*             vPayload    = rsp.getPayload(payloadSize);
            const char*             payload     = static_cast<const char*>(vPayload);
            
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

                    if (false == this->m_taskProxy.send(msg))
                    {
                        delete jsonDoc;
                        jsonDoc = nullptr;
                    }
                }
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

        m_textWidgetRight.setFormatStr(buffer);
        m_textWidgetTextOnly.setFormatStr(buffer);
    }
    /* Is it a string? */
    else if (true == jsonValue.is<String>())
    {
        const size_t    BUFFER_SIZE = 128U;
        char            buffer[BUFFER_SIZE];

        (void)snprintf(buffer, sizeof(buffer), m_format.c_str(), jsonValue.as<String>().c_str());

        m_textWidgetRight.setFormatStr(buffer);
        m_textWidgetTextOnly.setFormatStr(buffer);
    }
    else
    {
        m_textWidgetRight.setFormatStr("\\calign-");
        m_textWidgetTextOnly.setFormatStr("\\calign-");
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

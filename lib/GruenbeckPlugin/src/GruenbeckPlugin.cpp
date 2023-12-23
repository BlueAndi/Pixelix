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
 * @brief  Gruenbeck plugin.
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "GruenbeckPlugin.h"
#include "AsyncHttpClient.h"

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

/* Initialize image path. */
const char* GruenbeckPlugin::IMAGE_PATH     = "/plugin/GruenbeckPlugin/gruenbeck.bmp";

/* Initialize plugin topic. */
const char* GruenbeckPlugin::TOPIC_CONFIG   = "/ipAddress";

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

    if (0U != topic.equals(TOPIC_CONFIG))
    {
        getConfiguration(value);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool GruenbeckPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CONFIG))
    {
        const size_t        JSON_DOC_SIZE           = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg                 = jsonDoc.to<JsonObject>();
        String              ipAddress;
        JsonVariantConst    jsonIpAddress           = value["ipAddress"];

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
            jsonCfg["ipAddress"] = jsonIpAddress.as<String>();
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

bool GruenbeckPlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    bool                        hasTopicChanged = m_hasTopicChanged;

    /* Only a single topic, therefore its not necessary to check. */
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void GruenbeckPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_iconCanvas.setPosAndSize(0, 0, ICON_WIDTH, ICON_HEIGHT);
    (void)m_iconCanvas.addWidget(m_bitmapWidget);
    
    /* Load all icons from filesystem now, to prevent filesystem
     * access during active/inactive/update methods.
     */
    (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH);

    /* The text canvas is left aligned to the icon canvas and it spans over
     * the whole display height.
     */
    m_textCanvas.setPosAndSize(ICON_WIDTH, 0, width - ICON_WIDTH, height);
    (void)m_textCanvas.addWidget(m_textWidget);

    /* Choose font. */
    m_textWidget.setFont(Fonts::getFontByType(m_fontType));
    
    /* The text widget inside the text canvas is left aligned on x-axis and
     * aligned to the center of y-axis.
     */
    if (height > m_textWidget.getFont().getHeight())
    {
        uint16_t diffY = height - m_textWidget.getFont().getHeight();
        uint16_t offsY = diffY / 2U;

        m_textWidget.move(0, offsY);
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

    m_cfgReloadTimer.start(CFG_RELOAD_PERIOD);

    initHttpClient();
}

void GruenbeckPlugin::stop()
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

void GruenbeckPlugin::process(bool isConnected)
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
                /* If a request fails, show standard icon and a '?' */
                m_textWidget.setFormatStr("\\calign?");

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
                /* If a request fails, show standard icon and a '?' */
                m_textWidget.setFormatStr("\\calign?");

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
                /* If a request fails, show a '?' */
                m_textWidget.setFormatStr("\\calign?");

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

void GruenbeckPlugin::active(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    gfx.fillScreen(ColorDef::BLACK);
    m_iconCanvas.update(gfx);
    m_textCanvas.update(gfx);
}

void GruenbeckPlugin::inactive()
{
    /* Nothing to do */
}

void GruenbeckPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (false != m_httpResponseReceived)
    {
        m_textWidget.setFormatStr("\\calign" + m_relevantResponsePart + "%");
        
        gfx.fillScreen(ColorDef::BLACK);
        m_iconCanvas.update(gfx);
        m_textCanvas.update(gfx);

        m_relevantResponsePart = "";

        m_httpResponseReceived = false;
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void GruenbeckPlugin::requestStoreToPersistentMemory()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_storeConfigReq = true;
}

void GruenbeckPlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["ipAddress"] = m_ipAddress;
}

bool GruenbeckPlugin::setConfiguration(JsonObjectConst& jsonCfg)
{
    bool                status          = false;
    JsonVariantConst    jsonIpAddress   = jsonCfg["ipAddress"];

    if (false == jsonIpAddress.is<String>())
    {
        LOG_WARNING("JSON ipAddress not found or invalid type.");
    }
    else
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        m_ipAddress = jsonIpAddress.as<String>();

        /* Force update on display */
        m_requestTimer.start(UPDATE_PERIOD_SHORT);

        m_hasTopicChanged = true;

        status = true;
    }

    return status;
}

bool GruenbeckPlugin::startHttpRequest()
{
    bool status  = false;

    if (false == m_ipAddress.isEmpty())
    {
        String url = String("http://") + m_ipAddress + "/mux_http";

        if (true == m_client.begin(url))
        {
            m_client.addPar("id","42");
            m_client.addPar("show","D_Y_10_1~");

            if (false == m_client.POST())
            {
                LOG_WARNING("POST %s failed.", url.c_str());
            }
            else
            {
                status = true;
            }
        }
    }

    return status;
}

void GruenbeckPlugin::initHttpClient()
{
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

void GruenbeckPlugin::handleAsyncWebResponse(const HttpResponse& rsp)
{
    if (HttpStatus::STATUS_CODE_OK == rsp.getStatusCode())
    {
        const size_t            JSON_DOC_SIZE   = 256U;
        DynamicJsonDocument*    jsonDoc         = new(std::nothrow) DynamicJsonDocument(JSON_DOC_SIZE);

        if (nullptr != jsonDoc)
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
            const uint32_t  START_INDEX_OF_RELEVANT_DATA    = 31U;

            /* Length of relevant data */
            const uint32_t  RELEVANT_DATA_LENGTH            = 3U;

            size_t          payloadSize                     = 0U;
            const void*     vPayload                        = rsp.getPayload(payloadSize);
            const char*     payload                         = static_cast<const char*>(vPayload);
            char            restCapacity[RELEVANT_DATA_LENGTH + 1];
            Msg             msg;

            if ((nullptr != payload) &&
                (payloadSize >= (START_INDEX_OF_RELEVANT_DATA + RELEVANT_DATA_LENGTH)))
            {
                memcpy(restCapacity, &payload[START_INDEX_OF_RELEVANT_DATA], RELEVANT_DATA_LENGTH);
                restCapacity[RELEVANT_DATA_LENGTH] = '\0';
            }
            else
            {
                restCapacity[0] = '?';
                restCapacity[1] = '\0';
            }

            (*jsonDoc)["restCapacity"] = restCapacity;

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

void GruenbeckPlugin::handleWebResponse(const DynamicJsonDocument& jsonDoc)
{
    JsonVariantConst jsonRestCapacity = jsonDoc["restCapacity"];

    if (false == jsonRestCapacity.is<String>())
    {
        LOG_WARNING("JSON rest capacity mismatch or missing.");
    }
    else
    {
        m_relevantResponsePart = jsonRestCapacity.as<String>();
        m_httpResponseReceived = true;
    }
}

void GruenbeckPlugin::clearQueue()
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

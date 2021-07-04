/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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
#include "RestApi.h"
#include "AsyncHttpClient.h"
#include "FileSystem.h"

#include <ArduinoJson.h>
#include <Logging.h>
#include <JsonFile.h>

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
const char* GruenbeckPlugin::IMAGE_PATH = "/images/gruenbeck.bmp";

/* Initialize plugin topic. */
const char* GruenbeckPlugin::TOPIC      = "/ipAddress";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void GruenbeckPlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC);
}

bool GruenbeckPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC))
    {
        String  ipAddress   = getIPAddress();

        value["ipAddress"] = ipAddress;

        isSuccessful = true;
    }

    return isSuccessful;
}

bool GruenbeckPlugin::setTopic(const String& topic, const JsonObject& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC))
    {
        String  ipAddress;

        if (false == value["set"].isNull())
        {
            ipAddress = value["set"].as<String>();
            isSuccessful = true;
        }

        if (true == isSuccessful)
        {
            setIPAddress(ipAddress);
        }
    }

    return isSuccessful;
}

void GruenbeckPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (nullptr == m_iconCanvas)
    {
        m_iconCanvas = new Canvas(ICON_WIDTH, ICON_HEIGHT, 0, 0);

        if (nullptr != m_iconCanvas)
        {
            (void)m_iconCanvas->addWidget(m_bitmapWidget);

            /* Load  icon from filesystem. */
            (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH);
        }
    }

    if (nullptr == m_textCanvas)
    {
        m_textCanvas = new Canvas(width - ICON_WIDTH, height, ICON_WIDTH, 0);

        if (nullptr != m_textCanvas)
        {
            (void)m_textCanvas->addWidget(m_textWidget);
        }
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

    initHttpClient();
    if (false == startHttpRequest())
    {
        /* If a request fails, show a '?' */
        m_textWidget.setFormatStr("\\calign?");

        m_requestTimer.start(UPDATE_PERIOD_SHORT);
    }
    else
    {
        m_requestTimer.start(UPDATE_PERIOD);
    }

    return;
}

void GruenbeckPlugin::stop()
{
    String                      configurationFilename = getFullPathToConfiguration();
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_requestTimer.stop();

    if (false != FILESYSTEM.remove(configurationFilename))
    {
        LOG_INFO("File %s removed", configurationFilename.c_str());
    }

    if (nullptr != m_iconCanvas)
    {
        delete m_iconCanvas;
        m_iconCanvas = nullptr;
    }

    if (nullptr != m_textCanvas)
    {
        delete m_textCanvas;
        m_textCanvas = nullptr;
    }

    return;
}

void GruenbeckPlugin::process()
{
    Msg                         msg;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if ((true == m_requestTimer.isTimerRunning()) &&
        (true == m_requestTimer.isTimeout()))
    {
        if (false == startHttpRequest())
        {
            /* If a request fails, show a '?' */
            m_textWidget.setFormatStr("\\calign?");

            m_requestTimer.start(UPDATE_PERIOD_SHORT);
        }
        else
        {
            m_requestTimer.start(UPDATE_PERIOD);
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

    return;
}

void GruenbeckPlugin::active(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    gfx.fillScreen(ColorDef::BLACK);

    if (nullptr != m_iconCanvas)
    {
        m_iconCanvas->update(gfx);
    }

    if (nullptr != m_textCanvas)
    {
        m_textCanvas->update(gfx);
    }

    return;
}

void GruenbeckPlugin::inactive()
{
    /* Nothing to do */
    return;
}

void GruenbeckPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (false != m_httpResponseReceived)
    {
        m_textWidget.setFormatStr("\\calign" + m_relevantResponsePart + "%");
        gfx.fillScreen(ColorDef::BLACK);

        if (nullptr != m_iconCanvas)
        {
            m_iconCanvas->update(gfx);
        }

        if (nullptr != m_textCanvas)
        {
            m_textCanvas->update(gfx);
        }

        m_relevantResponsePart = "";

        m_httpResponseReceived = false;
    }

    return;
}

String GruenbeckPlugin::getIPAddress() const
{
    String                      ipAddress;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    ipAddress = m_ipAddress;

    return ipAddress;
}

void GruenbeckPlugin::setIPAddress(const String& ipAddress)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_ipAddress = ipAddress;
    (void)saveConfiguration();

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool GruenbeckPlugin::startHttpRequest()
{
    bool status  = false;

    if (0 < m_ipAddress.length())
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
            const size_t            JSON_DOC_SIZE   = 256U;
            DynamicJsonDocument*    jsonDoc         = new DynamicJsonDocument(JSON_DOC_SIZE);

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
                const char*     payload                         = reinterpret_cast<const char*>(rsp.getPayload(payloadSize));
                char            restCapacity[RELEVANT_DATA_LENGTH + 1];
                Msg             msg;

                if (payloadSize >= (START_INDEX_OF_RELEVANT_DATA + RELEVANT_DATA_LENGTH))
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

void GruenbeckPlugin::handleWebResponse(DynamicJsonDocument& jsonDoc)
{
    if (false == jsonDoc["restCapacity"].is<String>())
    {
        LOG_WARNING("JSON rest capacity missmatch or missing.");
    }
    else
    {
        m_relevantResponsePart = jsonDoc["restCapacity"].as<String>();
        m_httpResponseReceived = true;
    }
}

bool GruenbeckPlugin::saveConfiguration() const
{
    bool                status                  = true;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    String              configurationFilename   = getFullPathToConfiguration();

    jsonDoc["gruenbeckIP"] = m_ipAddress;
    
    if (false == jsonFile.save(configurationFilename, jsonDoc))
    {
        LOG_WARNING("Failed to save file %s.", configurationFilename.c_str());
        status = false;
    }
    else
    {
        LOG_INFO("File %s saved.", configurationFilename.c_str());
    }

    return status;
}

bool GruenbeckPlugin::loadConfiguration()
{
    bool                status                  = true;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    String              configurationFilename   = getFullPathToConfiguration();

    if (false == jsonFile.load(configurationFilename, jsonDoc))
    {
        LOG_WARNING("Failed to load file %s.", configurationFilename.c_str());
        status = false;
    }
    else
    {
        m_ipAddress = jsonDoc["gruenbeckIP"].as<String>();
    }

    return status;
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

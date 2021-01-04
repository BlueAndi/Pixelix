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

#include <ArduinoJson.h>
#include <Logging.h>
#include <SPIFFS.h>
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
const char* GruenbeckPlugin::IMAGE_PATH     = "/images/gruenbeck.bmp";

/* Initialize configuration path. */
const char* GruenbeckPlugin::CONFIG_PATH    = "/configuration";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void GruenbeckPlugin::registerWebInterface(AsyncWebServer& srv, const String& baseUri)
{
    m_url = baseUri + "/ipAddress";

    m_callbackWebHandler = &srv.on( m_url.c_str(),
                                    [this](AsyncWebServerRequest *request)
                                    {
                                        this->webReqHandler(request);
                                    });

    LOG_INFO("[%s] Register: %s", getName(), m_url.c_str());

    return;
}

void GruenbeckPlugin::unregisterWebInterface(AsyncWebServer& srv)
{
    LOG_INFO("[%s] Unregister: %s", getName(), m_url.c_str());

    if (false == srv.removeHandler(m_callbackWebHandler))
    {
        LOG_WARNING("Couldn't remove %s handler.", getName());
    }

    m_callbackWebHandler = nullptr;

    return;
}

void GruenbeckPlugin::active(IGfx& gfx)
{
    lock();

    gfx.fillScreen(ColorDef::BLACK);

    if (nullptr == m_iconCanvas)
    {
        m_iconCanvas = new Canvas(ICON_WIDTH, ICON_HEIGHT, 0, 0);

        if (nullptr != m_iconCanvas)
        {
            (void)m_iconCanvas->addWidget(m_bitmapWidget);

            /* Load  icon from filesystem. */
            (void)m_bitmapWidget.load(IMAGE_PATH);

            m_iconCanvas->update(gfx);
        }
    }
    else
    {
        m_iconCanvas->update(gfx);
    }

    if (nullptr == m_textCanvas)
    {
        m_textCanvas = new Canvas(gfx.getWidth() - ICON_WIDTH, gfx.getHeight(), ICON_WIDTH, 0);

        if (nullptr != m_textCanvas)
        {
            (void)m_textCanvas->addWidget(m_textWidget);

            m_textCanvas->update(gfx);
        }
    }
    else
    {
        m_textCanvas->update(gfx);
    }

    unlock();

    return;
}

void GruenbeckPlugin::inactive()
{
    /* Nothing to do */
    return;
}

void GruenbeckPlugin::update(IGfx& gfx)
{
    lock();

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

    unlock();

    return;
}

void GruenbeckPlugin::start()
{
    lock();

    m_configurationFilename = String(CONFIG_PATH) + "/" + getUID() + ".json";

    /* Try to load configuration. If there is no configuration available, a default configuration
     * will be created.
     */
    createConfigDirectory();
    if (false == loadConfiguration())
    {
        if (false == saveConfiguration())
        {
            LOG_WARNING("Failed to create initial configuration file %s.", m_configurationFilename.c_str());
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

    unlock();

    return;
}

void GruenbeckPlugin::stop()
{
    lock();

    m_requestTimer.stop();

    if (false != SPIFFS.remove(m_configurationFilename))
    {
        LOG_INFO("File %s removed", m_configurationFilename.c_str());
    }

    unlock();

    return;
}

void GruenbeckPlugin::process()
{
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

    return;
}

String GruenbeckPlugin::getIPAddress() const
{
    String ipAddress;

    lock();
    ipAddress = m_ipAddress;
    unlock();

    return ipAddress;
}

void GruenbeckPlugin::setIPAddress(const String& ipAddress)
{
    lock();
    m_ipAddress = ipAddress;
    (void)saveConfiguration();
    unlock();

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void GruenbeckPlugin::webReqHandler(AsyncWebServerRequest *request)
{
    String              content;
    const size_t        JSON_DOC_SIZE   = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;
    const size_t        MAX_USAGE       = 80U;
    size_t              usageInPercent  = 0U;

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET == request->method())
    {
        String      ipAddress   = getIPAddress();
        JsonObject  dataObj     = jsonDoc.createNestedObject("data");

        dataObj["ipAddress"] = ipAddress;

        /* Prepare response */
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);
        httpStatusCode      = HttpStatus::STATUS_CODE_OK;
    }
    else if (HTTP_POST == request->method())
    {
        /* Argument missing? */
        if (false == request->hasArg("set"))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Argument is missing.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else
        {
            String ipAddress = request->arg("set");

            setIPAddress(ipAddress);

            /* Prepare response */
            (void)jsonDoc.createNestedObject("data");
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);
            httpStatusCode      = HttpStatus::STATUS_CODE_OK;
        }
    }
    else
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }

    usageInPercent = (100U * jsonDoc.memoryUsage()) / jsonDoc.capacity();
    if (MAX_USAGE < usageInPercent)
    {
        LOG_WARNING("JSON document uses %u%% of capacity.", usageInPercent);
    }

    (void)serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

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
    m_client.regOnResponse([this](const HttpResponse& rsp){
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

        lock();
        m_relevantResponsePart = restCapacity;
        m_httpResponseReceived = true;
        unlock();
    });

    m_client.regOnClosed([this]() {
        LOG_INFO("Connection closed.");

        lock();
        if (true == m_isConnectionError)
        {
            /* If a request fails, show a '?' */
            m_textWidget.setFormatStr("\\calign?");

            m_requestTimer.start(UPDATE_PERIOD_SHORT);
        }
        m_isConnectionError = false;
        unlock();
    });

    m_client.regOnError([this]() {
        LOG_WARNING("Connection error happened.");

        lock();
        m_isConnectionError = true;
        unlock();
    });
}

bool GruenbeckPlugin::saveConfiguration()
{
    bool                status                  = true;
    JsonFile            jsonFile(SPIFFS);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    jsonDoc["gruenbeckIP"] = m_ipAddress;
    
    if (false == jsonFile.save(m_configurationFilename, jsonDoc))
    {
        LOG_WARNING("Failed to save file %s.", m_configurationFilename.c_str());
        status = false;
    }
    else
    {
        LOG_INFO("File %s saved.", m_configurationFilename.c_str());
    }

    return status;
}

bool GruenbeckPlugin::loadConfiguration()
{
    bool                status                  = true;
    JsonFile            jsonFile(SPIFFS);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (false == jsonFile.load(m_configurationFilename, jsonDoc))
    {
        LOG_WARNING("Failed to load file %s.", m_configurationFilename.c_str());
        status = false;
    }
    else
    {
        m_ipAddress = jsonDoc["gruenbeckIP"].as<String>();
    }

    return status;
}

void GruenbeckPlugin::createConfigDirectory()
{
    if (false == SPIFFS.exists(CONFIG_PATH))
    {
        if (false == SPIFFS.mkdir(CONFIG_PATH))
        {
            LOG_WARNING("Couldn't create directory: %s", CONFIG_PATH);
        }
    }
}

void GruenbeckPlugin::lock() const
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreTakeRecursive(m_xMutex, portMAX_DELAY);
    }

    return;
}

void GruenbeckPlugin::unlock() const
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreGiveRecursive(m_xMutex);
    }

    return;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

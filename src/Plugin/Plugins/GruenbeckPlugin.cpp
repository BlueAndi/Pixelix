/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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
#include <SPIFFS.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/* Structure of response-payload for requesting D_Y_10_1
 * 
 * <data><code>ok</code><D_Y_10_1>XYZ</D_Y_10_1></data> 
 * 
 * <data><code>ok</code><D_Y_10_1>  = 31 bytes
 * XYZ                              = 3 byte (relevant data)
 * </D_Y_10_1></data>               = 18 bytes
 */
 
/* Startindex of relevant data. */
#define START_INDEX_OF_RELEVANT_DATA (31U)

/* Endindex of relevant data. */
#define END_INDEX_OF_RELEVANT_DATA (34u)

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

void GruenbeckPlugin::active(IGfx& gfx)
{
    if (nullptr == m_iconCanvas)
    {
        m_iconCanvas = new Canvas(ICON_WIDTH, ICON_HEIGHT, 0, 0);

        if (nullptr != m_iconCanvas)
        {
            (void)m_iconCanvas->addWidget(m_bitmapWidget);

            /* Load  icon from filesystem. */
            (void)m_bitmapWidget.load(IMAGE_PATH);
            gfx.fillScreen(ColorDef::BLACK);

            m_iconCanvas->update(gfx);
        }
    }

    if (nullptr == m_textCanvas)
    {
        m_textCanvas = new Canvas(gfx.getWidth() - ICON_WIDTH, gfx.getHeight(), ICON_WIDTH, 0);

        if (nullptr != m_textCanvas)
        {
            (void)m_textCanvas->addWidget(m_textWidget);

            /* Move the text widget one line lower for better look. */
            m_textWidget.move(0, 1);

            m_textWidget.setFormatStr("\\calign?");
         
            m_textCanvas->update(gfx);
        }
    }

    requestNewData();
    m_requestDataTimer.start(GruenbeckPlugin::UPDATE_PERIOD);
        
    return;
}

void GruenbeckPlugin::inactive()
{
    m_requestDataTimer.stop();
    return;
}

void GruenbeckPlugin::update(IGfx& gfx)
{
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

void GruenbeckPlugin::start()
{
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

    registerResponseCallback();

    return;
}

void GruenbeckPlugin::stop() 
{
    if (false != SPIFFS.remove(m_configurationFilename))
    {
        LOG_INFO("File %s removed", m_configurationFilename.c_str());
    }

    return;
}

void GruenbeckPlugin::process()
{
    if ((true == m_requestDataTimer.isTimerRunning()) &&
        (true == m_requestDataTimer.isTimeout()))
    {
        requestNewData();
        m_requestDataTimer.restart();
    }

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void GruenbeckPlugin::requestNewData()
{
    String url = String("http://") + m_ipAddress + "/mux_http";

    if (true == m_client.begin(url))
    {
        m_client.addPar("id","42");
        m_client.addPar("show","D_Y_10_1~");
        (void)m_client.POST();
    }
}

void GruenbeckPlugin::registerResponseCallback()
{
    m_client.regOnResponse([this](const HttpResponse& rsp){
        size_t      payloadSize     = 0U;
        const char* payload         = reinterpret_cast<const char*>(rsp.getPayload(payloadSize));
        size_t      payloadIndex    = 0U;
        String      payloadString;
       
        while(payloadSize > payloadIndex)
        {
            payloadString += payload[payloadIndex];
            ++payloadIndex;
        }
        m_relevantResponsePart = payloadString.substring(START_INDEX_OF_RELEVANT_DATA, END_INDEX_OF_RELEVANT_DATA);
        m_httpResponseReceived = true;
    });
}

bool GruenbeckPlugin::saveConfiguration()
{
    bool    status  = true;
    File    fd      = SPIFFS.open(m_configurationFilename, "w");

    if (false == fd)
    {
        LOG_WARNING("Failed to create file %s.", m_configurationFilename.c_str());
        status = false;
    }
    else
    {
        const size_t        JSON_DOC_SIZE           = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

        jsonDoc["gruenbeckIP"] = m_ipAddress;

        (void)serializeJson(jsonDoc, fd);
        fd.close();

        LOG_INFO("File %s saved.", m_configurationFilename.c_str());
    }

    return status;
}

bool GruenbeckPlugin::loadConfiguration()
{
    bool    status  = true;
    File    fd      = SPIFFS.open(m_configurationFilename, "r");

    if (false == fd)
    {
        LOG_WARNING("Failed to load file %s.", m_configurationFilename.c_str());
        status = false;
    }
    else
    {
        const size_t            JSON_DOC_SIZE           = 512U;
        DynamicJsonDocument     jsonDoc(JSON_DOC_SIZE);
        DeserializationError    error                   = deserializeJson(jsonDoc, fd.readString());

        if (DeserializationError::Ok != error)
        {
            LOG_WARNING("Failed to load file %s.", m_configurationFilename.c_str());
            status = false;   
        }
        else
        {
            JsonObject obj = jsonDoc.as<JsonObject>();

            m_ipAddress = obj["gruenbeckIP"].as<String>();
        }        

        fd.close();
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

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

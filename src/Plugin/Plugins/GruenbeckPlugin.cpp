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
/** Structure of response-payload for requesting D_Y_10_1
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
const char* GruenbeckPlugin::CONFIG_PATH    = "/configuration/";

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
            m_iconCanvas->addWidget(m_bitmapWidget);

            /* Load  icon from filesystem. */
            (void)m_bitmapWidget.load(IMAGE_PATH);
            gfx.fillScreen(ColorDef::convert888To565(ColorDef::BLACK));

            m_iconCanvas->update(gfx);
        }
    }

    if (nullptr == m_textCanvas)
    {
        m_textCanvas = new Canvas(gfx.width() - ICON_WIDTH, gfx.height(), ICON_WIDTH, 0);

        if (nullptr != m_textCanvas)
        {
            m_textCanvas->addWidget(m_textWidget);

            /* Move the text widget one line lower for better look. */
            m_textWidget.move(0, 1);

            setText("\\calign?");
         
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
        setText("\\calign" + m_relevantResponsePart + "%");
        gfx.fillScreen(ColorDef::convert888To565(ColorDef::BLACK));

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

void GruenbeckPlugin::setText(const String& formatText)
{
    m_textWidget.setFormatStr(formatText);

    return;
}

void GruenbeckPlugin::start()
{
    String configPath = CONFIG_PATH;

    m_configurationFilename = configPath + getUID() + ".json";

    if (false != loadOrGenerateConfigFile())
    {
        LOG_WARNING("Error on loading/generating: %s", m_configurationFilename);
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
    m_client.begin(m_url);
    m_client.addPar("id","42");
    m_client.addPar("show","D_Y_10_1~");
    m_client.addHeader("Content-Type", "application/x-www-form-urlencoded");
    m_client.POST();
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


bool GruenbeckPlugin::loadOrGenerateConfigFile()
{
    bool                status          = true;
    const size_t        JSON_DOC_SIZE   = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    const size_t        MAX_USAGE       = 80U;
    size_t              usageInPercent  = (100U * jsonDoc.memoryUsage()) / jsonDoc.capacity();

    /* Check if the plugin has already created it's configuration file in the filesystem.*/
    if (false == SPIFFS.exists(m_configurationFilename))
    {
        LOG_WARNING("File %s doesn't exists.", m_configurationFilename.c_str());

        /* If not  we are on the very first instalation of the plugin
           First we create the directory. */
        if (false == SPIFFS.mkdir(m_configurationFilename))
        {
            LOG_WARNING("Couldn't create directory: %s", m_configurationFilename);
            status = false;
        }
        else
        {
            /* And afterwards the plugin(UID)specific configuration file with default configuration values. */
            String defaultIPAddress = "192.168.0.16";

            m_fd = SPIFFS.open(m_configurationFilename, "w");
            jsonDoc["gruenbeck_ip"] = defaultIPAddress;
            serializeJson(jsonDoc, m_fd);
            m_fd.close();
            m_url = "http://" + defaultIPAddress + "/mux_http";
            LOG_INFO("File %s created", m_configurationFilename.c_str());
            status = false;
        }
    }
    else
    {
        m_fd = SPIFFS.open(m_configurationFilename, "r");

        if (false == m_fd)
        {
            LOG_WARNING("Failed to open file %s.", m_configurationFilename.c_str());
            status = false;
        }
        else
        {
            JsonObject obj;
            String ipAddress;
            String file_content = m_fd.readString();
            
            deserializeJson(jsonDoc, file_content);

            obj = jsonDoc.as<JsonObject>();
            ipAddress = obj["gruenbeck_ip"].as<String>();

            m_url = "http://" + ipAddress + "/mux_http";
            m_fd.close();
        }
    }

    if (MAX_USAGE < usageInPercent)
    {
        LOG_WARNING("JSON document uses %u%% of capacity.", usageInPercent);
    }

    return status;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

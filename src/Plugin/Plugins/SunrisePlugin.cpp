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
 * @brief  Sunrise plugin.
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "AsyncHttpClient.h"
#include "ClockDrv.h"
#include "Settings.h"
#include "SunrisePlugin.h"

#include "time.h"
#include <ArduinoJson.h>
#include <Logging.h>
#include <SPIFFS.h>

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
const char* SunrisePlugin::IMAGE_PATH     = "/images/sunrise.bmp";

/* Initialize configuration path. */
const char* SunrisePlugin::CONFIG_PATH    = "/configuration/";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void SunrisePlugin::active(IGfx& gfx)
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

            setText("\\calign?");
         
            m_textCanvas->update(gfx);
        }
    }

    requestNewData();

    return;
}

void SunrisePlugin::inactive()
{
    return;
}

void SunrisePlugin::update(IGfx& gfx)
{
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

void SunrisePlugin::setText(const String& formatText)
{
    m_textWidget.setFormatStr(formatText);

    return;
}

void SunrisePlugin::start()
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

void SunrisePlugin::stop() 
{
    if (false != SPIFFS.remove(m_configurationFilename))
    {
        LOG_INFO("File %s removed", m_configurationFilename.c_str());
    }

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void SunrisePlugin::requestNewData()
{
    if (true == m_client.begin(m_url))
    {
        (void)m_client.GET();
    }
}

void SunrisePlugin::registerResponseCallback()
{

    m_client.regOnResponse([this](const HttpResponse& rsp){
    size_t              payloadSize     = 0U;
    const char*         payload         = reinterpret_cast<const char*>(rsp.getPayload(payloadSize));
    size_t              payloadIndex    = 0U;
    const size_t        JSON_DOC_SIZE   = 768U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    const size_t        MAX_USAGE       = 80U;
    size_t              usageInPercent  = (100U * jsonDoc.memoryUsage()) / jsonDoc.capacity();
    String sunrise;
    String sunset;
    JsonObject results;
    JsonObject obj;


    while(payloadSize > payloadIndex)
    {
        m_response += payload[payloadIndex];
        ++payloadIndex;
    }

    m_httpResponseReceived = true;

    deserializeJson(jsonDoc, m_response);
    obj = jsonDoc.as<JsonObject>();
    results = obj["results"];
    sunrise = results["sunrise"].as<String>();
    sunset = results["sunset"].as<String>();

    sunrise = addCurrentTimezoneValues(sunrise);
    sunset = addCurrentTimezoneValues(sunset);

    m_relevantResponsePart = sunrise + " / " + sunset;

    setText(m_relevantResponsePart);

    if (MAX_USAGE < usageInPercent)
    {
        LOG_WARNING("JSON document uses %u%% of capacity.", usageInPercent);
    }
    });
}

String SunrisePlugin::addCurrentTimezoneValues(String dateTimeString)
{
    tm timeInfo;
    char timeBuffer [17];
    int16_t gmtOffset = 0;
    int16_t isDaylightSaving = 0;
    const char* formattedTimeString = ClockDrv::getInstance().getTimeFormat() ? "%H:%M":"%I:%M %p";
    bool isPM = dateTimeString.endsWith("PM");

    /* Get the GMT offset and daylight saving enabled/disabled from persistent memory. */
    if (false == Settings::getInstance().open(true))
    {
        LOG_WARNING("Use default values.");
        gmtOffset           = Settings::getInstance().getGmtOffset().getDefault();
        isDaylightSaving    = Settings::getInstance().getDaylightSavingAdjustment().getDefault();
    }
    else
    {
        gmtOffset           = Settings::getInstance().getGmtOffset().getValue();
        isDaylightSaving    = Settings::getInstance().getDaylightSavingAdjustment().getValue();
        Settings::getInstance().close();
    }

    strptime(dateTimeString.c_str(), "%Y-%m-%dT%H:%M:%S" ,&timeInfo); 
    timeInfo.tm_hour += gmtOffset /3600;
    timeInfo.tm_hour += isDaylightSaving;
    timeInfo.tm_hour += isPM * 12;

    strftime(timeBuffer, sizeof(timeBuffer), formattedTimeString, &timeInfo);

    return timeBuffer;
}

bool SunrisePlugin::loadOrGenerateConfigFile()
{
    bool status = true;
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
            String defaultLongitude = "2.295";
            String defaultLatitude = "48.858";

            m_fd = SPIFFS.open(m_configurationFilename, "w");
            jsonDoc["longitude"] = defaultLongitude;
            jsonDoc["latitude"] = defaultLatitude;

            serializeJson(jsonDoc, m_fd);
            m_fd.close();
            m_url = "http://api.sunrise-sunset.org/json?lat="+ defaultLatitude + "&lng=" + defaultLongitude + "&formatted=0";

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
            String longitude;
            String latitude;
            JsonObject obj;
            String file_content = m_fd.readString();

            deserializeJson(jsonDoc, file_content);

            obj = jsonDoc.as<JsonObject>();
            
            longitude = obj["longitude"].as<String>();
            latitude = obj["latitude"].as<String>();
            
            m_url = "http://api.sunrise-sunset.org/json?lat="+ latitude + "&lng=" + longitude + "&formatted=0";

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

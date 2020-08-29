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
#include "RestApi.h"

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
const char* SunrisePlugin::CONFIG_PATH    = "/configuration";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void SunrisePlugin::registerWebInterface(AsyncWebServer& srv, const String& baseUri)
{
    m_url = baseUri + "/location";

    m_callbackWebHandler = &srv.on( m_url.c_str(),
                                    [this](AsyncWebServerRequest *request)
                                    {
                                        this->webReqHandler(request);
                                    });

    LOG_INFO("[%s] Register: %s", getName(), m_url.c_str());

    return;
}

void SunrisePlugin::unregisterWebInterface(AsyncWebServer& srv)
{
    LOG_INFO("[%s] Unregister: %s", getName(), m_url.c_str());

    if (false == srv.removeHandler(m_callbackWebHandler))
    {
        LOG_WARNING("Couldn't remove %s handler.", getName());
    }

    m_callbackWebHandler = nullptr;

    return;
}

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

            m_textWidget.setFormatStr("\\calign?");

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

void SunrisePlugin::start()
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

void SunrisePlugin::stop()
{
    if (false != SPIFFS.remove(m_configurationFilename))
    {
        LOG_INFO("File %s removed", m_configurationFilename.c_str());
    }

    return;
}

void SunrisePlugin::setLocation(const String& longitude, const String& latitude)
{
    m_longitude = longitude;
    m_latitude  = latitude;

    /* Always stores the configuration, otherwise it will be overwritten during
     * plugin activation.
     */
    (void)saveConfiguration();

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void SunrisePlugin::webReqHandler(AsyncWebServerRequest *request)
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

    if (HTTP_POST != request->method())
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        /* Location missing? */
        if ((false == request->hasArg("longitude")) ||
            (false == request->hasArg("latitude")))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Argument is missing.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else
        {
            setLocation(request->arg("longitude"), request->arg("latitude"));

            /* Prepare response */
            (void)jsonDoc.createNestedObject("data");
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);
            httpStatusCode      = HttpStatus::STATUS_CODE_OK;
        }
    }

    usageInPercent = (100U * jsonDoc.memoryUsage()) / jsonDoc.capacity();
    if (MAX_USAGE < usageInPercent)
    {
        LOG_WARNING("JSON document uses %u%% of capacity.", usageInPercent);
    }

    serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

void SunrisePlugin::requestNewData()
{
    String url = String("http://api.sunrise-sunset.org/json?lat=") + m_latitude + "&lng=" + m_longitude + "&formatted=0";

    if (true == m_client.begin(url))
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
        String              payloadStr;
        const size_t        JSON_DOC_SIZE   = 768U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        const size_t        MAX_USAGE       = 80U;
        size_t              usageInPercent  = 0U;
        String              sunrise;
        String              sunset;
        JsonObject          results;
        JsonObject          obj;

        while(payloadSize > payloadIndex)
        {
            payloadStr += payload[payloadIndex];
            ++payloadIndex;
        }

        m_httpResponseReceived = true;

        deserializeJson(jsonDoc, payloadStr);
        obj = jsonDoc.as<JsonObject>();
        results = obj["results"];
        sunrise = results["sunrise"].as<String>();
        sunset = results["sunset"].as<String>();

        sunrise = addCurrentTimezoneValues(sunrise);
        sunset = addCurrentTimezoneValues(sunset);

        m_relevantResponsePart = sunrise + " / " + sunset;

        m_textWidget.setFormatStr(m_relevantResponsePart);

        usageInPercent = (100U * jsonDoc.memoryUsage()) / jsonDoc.capacity();
        if (MAX_USAGE < usageInPercent)
        {
            LOG_WARNING("JSON document uses %u%% of capacity.", usageInPercent);
        }
    });
}

String SunrisePlugin::addCurrentTimezoneValues(String dateTimeString)
{
    tm          timeInfo;
    char        timeBuffer[17]      = { 0 };
    int16_t     gmtOffset           = 0;
    int16_t     isDaylightSaving    = 0;
    const char* formattedTimeString = ClockDrv::getInstance().getTimeFormat() ? "%H:%M":"%I:%M %p";
    bool        isPM                = dateTimeString.endsWith("PM");

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

bool SunrisePlugin::saveConfiguration()
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

        jsonDoc["longitude"]    = m_longitude;
        jsonDoc["latitude"]     = m_latitude;

        (void)serializeJson(jsonDoc, fd);
        fd.close();

        LOG_INFO("File %s saved.", m_configurationFilename.c_str());
    }

    return status;
}

bool SunrisePlugin::loadConfiguration()
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

            m_longitude = obj["longitude"].as<String>();
            m_latitude  = obj["latitude"].as<String>();
        }

        fd.close();
    }

    return status;
}

void SunrisePlugin::createConfigDirectory()
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

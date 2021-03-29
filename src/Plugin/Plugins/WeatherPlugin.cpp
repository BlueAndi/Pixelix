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
 * @brief  Weather Plugin based on sunrise plugin
 * @author Flavio Curti <fcu-github@no-way.org>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "AsyncHttpClient.h"
#include "ClockDrv.h"
#include "Settings.h"
#include "WeatherPlugin.h"
#include "RestApi.h"
#include "time.h"
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

/* Workaround: strptime is available in libc, but the prototype is not available
 * in time.h
 * Therefore we define the prototype here. Should be removed, if time.h is
 * hopefully updated in the next IDF release.
 */
char *_EXFUN(strptime,  (const char *__restrict,
                        const char *__restrict,
                        struct tm *__restrict));

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/* Initialize image path. */
const char* WeatherPlugin::IMAGE_PATH     = "/images/Weather.bmp";

/* Initialize configuration path. */
const char* WeatherPlugin::CONFIG_PATH    = "/configuration";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void WeatherPlugin::registerWebInterface(AsyncWebServer& srv, const String& baseUri)
{
    m_url = baseUri + "/weather";

    m_callbackWebHandler = &srv.on( m_url.c_str(),
                                    [this](AsyncWebServerRequest *request)
                                    {
                                        this->webReqHandler(request);
                                    });

    LOG_INFO("[%s] Register: %s", getName(), m_url.c_str());

    return;
}

void WeatherPlugin::unregisterWebInterface(AsyncWebServer& srv)
{
    LOG_INFO("[%s] Unregister: %s", getName(), m_url.c_str());

    if (false == srv.removeHandler(m_callbackWebHandler))
    {
        LOG_WARNING("Couldn't remove %s handler.", getName());
    }

    m_callbackWebHandler = nullptr;

    return;
}

void WeatherPlugin::active(IGfx& gfx)
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
            (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH);

            m_iconCanvas->update(gfx);
        }
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

    unlock();

    return;
}

void WeatherPlugin::inactive()
{
    return;
}

void WeatherPlugin::update(IGfx& gfx)
{
    lock();

    gfx.fillScreen(ColorDef::BLACK);

    if (nullptr != m_iconCanvas)
    {
        m_iconCanvas->update(gfx);
    }

    if (nullptr != m_textCanvas)
    {
        m_textCanvas->update(gfx);
    }

    unlock();

    return;
}

void WeatherPlugin::start()
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
        m_requestTimer.start(UPDATE_PERIOD_SHORT);
    }
    else
    {
        m_requestTimer.start(UPDATE_PERIOD);
    }

    unlock();

    return;
}

void WeatherPlugin::stop()
{
    lock();

    m_requestTimer.stop();

    if (false != FILESYSTEM.remove(m_configurationFilename))
    {
        LOG_INFO("File %s removed", m_configurationFilename.c_str());
    }

    unlock();

    return;
}

void WeatherPlugin::process()
{
    lock();

    if ((true == m_requestTimer.isTimerRunning()) &&
        (true == m_requestTimer.isTimeout()))
    {
        if (false == startHttpRequest())
        {
            m_requestTimer.start(UPDATE_PERIOD_SHORT);
        }
        else
        {
            m_requestTimer.start(UPDATE_PERIOD);
        }
    }

    unlock();

    return;
}

void WeatherPlugin::getLocation(String& location) const
{
    lock();
    location = m_location;
    unlock();
    return;
}

void WeatherPlugin::getApikey(String& apikey) const
{
    lock();
    apikey = m_apikey;
    unlock();
    return;
}

void WeatherPlugin::getDays(String& days) const
{
    lock();
    days = m_days;
    unlock();
    return;
}

void WeatherPlugin::setLocation(const String& location)
{
    lock();
    if ((location != m_location))
    {
        m_location = location;
        (void)saveConfiguration();
    }
    unlock();
    return;
}

void WeatherPlugin::setApikey(const String& apikey)
{
    lock();
    if ((apikey != m_apikey))
    {
        m_apikey = apikey;
        (void)saveConfiguration();
    }
    unlock();
    return;
}

void WeatherPlugin::setDays(const String& days)
{
    lock();
    if ((days != m_days))
    {
        m_days = days;
        (void)saveConfiguration();
    }
    unlock();
    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void WeatherPlugin::webReqHandler(AsyncWebServerRequest *request)
{
    String              content;
    const size_t        JSON_DOC_SIZE   = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET == request->method())
    {
        JsonObject  dataObj     = jsonDoc.createNestedObject("data");
        String      location;
        String      days;
        String      apikey;        

        //get everything;
        getLocation(location);
        getDays(days);
        getApikey(apikey);
        
        dataObj["location"] = location;
        dataObj["days"]     = days;
        dataObj["apikey"]   = apikey;

        /* Prepare response */
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);
        httpStatusCode      = HttpStatus::STATUS_CODE_OK;
    }
    else if (HTTP_POST == request->method())
    {
        /* Location missing? */
        if ((false == request->hasArg("location")) ||
            (false == request->hasArg("days")) ||
            (false == request->hasArg("apikey"))
            )
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Argument is missing.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else
        {
            setLocation(request->arg("location"));
            setApikey(request->arg("apikey"));
            setDays(request->arg("days"));

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

    if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document has less memory available.");
    }
    else
    {
        LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
    }

    (void)serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

bool WeatherPlugin::startHttpRequest()
{
    bool    status  = false;
    String  url     = String("http://api.Weather-sunset.org/json?lat=") + m_location + "&formatted=0";

    if (true == m_client.begin(url))
    {
        if (false == m_client.GET())
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

void WeatherPlugin::initHttpClient()
{
    m_client.regOnResponse([this](const HttpResponse& rsp){
        size_t                          payloadSize             = 0U;
        const char*                     payload                 = reinterpret_cast<const char*>(rsp.getPayload(payloadSize));
        const size_t                    JSON_DOC_SIZE           = 512U;
        DynamicJsonDocument             jsonDoc(JSON_DOC_SIZE);
        const size_t                    FILTER_SIZE             = 128U;
        StaticJsonDocument<FILTER_SIZE> filter;
        DeserializationError            error;

        m_httpResponseReceived = true;

        filter["results"]["Weather"]    = true;
        filter["results"]["sunset"]     = true;

        if (true == filter.overflowed())
        {
            LOG_ERROR("Less memory for filter available.");
        }

        error = deserializeJson(jsonDoc, payload, payloadSize, DeserializationOption::Filter(filter));

        if (DeserializationError::Ok != error.code())
        {
            LOG_ERROR("Invalid JSON message received: %s", error.c_str());
        }
        else
        {
            /*
            String      Weather;
            String      sunset;
            JsonObject  results;
            
            results = jsonDoc["results"];
            Weather = results["Weather"].as<String>();
            sunset  = results["sunset"].as<String>();

            Weather = addCurrentTimezoneValues(sunrise);
            sunset  = addCurrentTimezoneValues(sunset);

            lock();

            m_relevantResponsePart = sunrise + " / " + sunset;
            m_textWidget.setFormatStr(m_relevantResponsePart);

            unlock();
            */
            if (true == jsonDoc.overflowed())
            {
                LOG_ERROR("JSON document has less memory available.");
            }
            else
            {
                LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
            }
        }
    });
}


bool WeatherPlugin::saveConfiguration()
{
    bool                status                  = true;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    jsonDoc["location"]     = m_location;
    jsonDoc["apikey"]       = m_apikey;
    jsonDoc["days"]         = m_days;
    
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

bool WeatherPlugin::loadConfiguration()
{
    bool                status                  = true;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (false == jsonFile.load(m_configurationFilename, jsonDoc))
    {
        LOG_WARNING("Failed to load file %s.", m_configurationFilename.c_str());
        status = false;
    }
    else
    {
        m_location = jsonDoc["location"].as<String>();
        m_apikey  = jsonDoc["apikey"].as<String>();
        m_days  = jsonDoc["days"].as<String>();
    }

    return status;
}

void WeatherPlugin::createConfigDirectory()
{
    if (false == FILESYSTEM.exists(CONFIG_PATH))
    {
        if (false == FILESYSTEM.mkdir(CONFIG_PATH))
        {
            LOG_WARNING("Couldn't create directory: %s", CONFIG_PATH);
        }
    }
}

void WeatherPlugin::lock() const
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreTakeRecursive(m_xMutex, portMAX_DELAY);
    }

    return;
}

void WeatherPlugin::unlock() const
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

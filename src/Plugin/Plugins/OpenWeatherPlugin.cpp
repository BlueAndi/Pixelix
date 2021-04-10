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
 * @brief  OpenWeather plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "OpenWeatherPlugin.h"
#include "RestApi.h"
#include "FileSystem.h"

#include <Logging.h>
#include <ArduinoJson.h>
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

/* Initialize image path for standard icon. */
const char* OpenWeatherPlugin::IMAGE_PATH_STD_ICON      = "/images/openWeather.bmp";

/* Initialize image path for the weather condition icons. */
const char* OpenWeatherPlugin::IMAGE_PATH               = "/images/";

/* Initialize OpenWeather base URI */
const char* OpenWeatherPlugin::OPEN_WEATHER_BASE_URI    = "http://api.openweathermap.org";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void OpenWeatherPlugin::start()
{
    lock();

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
        /* If a request fails, show standard icon and a '?' */
        (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);
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

void OpenWeatherPlugin::stop()
{
    String configurationFilename = getFullPathToConfiguration();

    lock();

    m_requestTimer.stop();

    if (false != FILESYSTEM.remove(configurationFilename))
    {
        LOG_INFO("File %s removed", configurationFilename.c_str());
    }

    unlock();

    return;
}

void OpenWeatherPlugin::process()
{
    lock();

    if ((true == m_requestTimer.isTimerRunning()) &&
        (true == m_requestTimer.isTimeout()))
    {
        if (false == startHttpRequest())
        {
            /* If a request fails, show standard icon and a '?' */
            (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);
            m_textWidget.setFormatStr("\\calign?");

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

void OpenWeatherPlugin::active(IGfx& gfx)
{
    lock();

    gfx.fillScreen(ColorDef::BLACK);

    if (nullptr == m_iconCanvas)
    {
        m_iconCanvas = new Canvas(ICON_WIDTH, ICON_HEIGHT, 0, 0);

        if (nullptr != m_iconCanvas)
        {
            (void)m_iconCanvas->addWidget(m_bitmapWidget);

            /* Load icon from filesystem. */
            (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);

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

void OpenWeatherPlugin::inactive()
{
    /* Nothing to do. */
    return;
}

void OpenWeatherPlugin::registerWebInterface(AsyncWebServer& srv, const String& baseUri)
{
    m_urlWeather = baseUri + "/weather";

    m_callbackWebHandlerWeather = &srv.on( m_urlWeather.c_str(),
                                    [this](AsyncWebServerRequest *request)
                                    {
                                        this->webReqHandler(request);
                                    });

    LOG_INFO("[%s] Register: %s", getName(), m_urlWeather.c_str());

    return;
}

void OpenWeatherPlugin::unregisterWebInterface(AsyncWebServer& srv)
{
    LOG_INFO("[%s] Unregister: %s", getName(), m_urlWeather.c_str());

    if (false == srv.removeHandler(m_callbackWebHandlerWeather))
    {
        LOG_WARNING("Couldn't remove %s handler.", getName());
    }

    m_callbackWebHandlerWeather = nullptr;

    return;
}

void OpenWeatherPlugin::update(IGfx& gfx)
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

String OpenWeatherPlugin::getApiKey() const
{
    String apiKey;

    lock();
    apiKey = m_apiKey;
    unlock();

    return apiKey;
}

void OpenWeatherPlugin::setApiKey(const String& apiKey)
{
    lock();
    m_apiKey = apiKey;
    (void)saveConfiguration();
    unlock();

    return;
}

String OpenWeatherPlugin::getCityId() const
{
    String cityId;

    lock();
    cityId = m_cityId;
    unlock();

    return cityId;
}

void OpenWeatherPlugin::setCityId(const String& cityId)
{
    lock();
    m_cityId = cityId;
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

void OpenWeatherPlugin::webReqHandler(AsyncWebServerRequest *request)
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
        JsonObject  dataObj = jsonDoc.createNestedObject("data");

        dataObj["apiKey"] = getApiKey();
        dataObj["cityId"] = getCityId();

        /* Prepare response */
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);
        httpStatusCode      = HttpStatus::STATUS_CODE_OK;
    }
    else if (HTTP_POST == request->method())
    {
        /* One argument must be available at least. */
        if ((false == request->hasArg("apiKey")) &&
            (false == request->hasArg("cityId")))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Argument is missing.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else
        {
            if (true == request->hasArg("apiKey"))
            {
                String apiKey = request->arg("apiKey");
                setApiKey(apiKey);
            }

            if (true == request->hasArg("cityId"))
            {
                String cityId = request->arg("cityId");
                setCityId(cityId);
            }

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

bool OpenWeatherPlugin::startHttpRequest()
{
    bool status = false;

    if ((0 < m_apiKey.length()) &&
        (0 < m_cityId.length()))
    {
        String url = OPEN_WEATHER_BASE_URI;

        /* Get current weather information: https://openweathermap.org/current#cityid */
        url += "/data/2.5/weather?id=";
        url += m_cityId;
        url += "&units=metric&APPID=";
        url += m_apiKey;

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
    }

    return status;
}

void OpenWeatherPlugin::initHttpClient()
{
    m_client.regOnResponse([this](const HttpResponse& rsp){
        size_t                          payloadSize             = 0U;
        const char*                     payload                 = reinterpret_cast<const char*>(rsp.getPayload(payloadSize));
        const size_t                    JSON_DOC_SIZE           = 512U;
        DynamicJsonDocument             jsonDoc(JSON_DOC_SIZE);
        const size_t                    FILTER_SIZE             = 128U;
        StaticJsonDocument<FILTER_SIZE> filter;
        DeserializationError            error;

        /* See https://openweathermap.org/current#current_JSON */
        filter["main"]["temp"]          = true; /* Temperature */
        filter["weather"][0]["icon"]    = true; /* Weather icon id, see https://openweathermap.org/weather-conditions */
        
        if (true == filter.overflowed())
        {
            LOG_ERROR("Less memory for filter available.");
        }

        error = deserializeJson(jsonDoc, payload, payloadSize, DeserializationOption::Filter(filter));

        if (DeserializationError::Ok != error.code())
        {
            LOG_WARNING("JSON parse error: %s", error.c_str());
        }
        else
        {
            if (false == jsonDoc["main"]["temp"].is<float>())
            {
                LOG_WARNING("JSON temperature type missmatch or missing.");
            }
            else if (false == jsonDoc["weather"][0]["icon"].is<String>())
            {
                LOG_WARNING("JSON weather icon id type missmatch or missing.");
            }
            else
            {
                float   temperature             = jsonDoc["main"]["temp"].as<float>();
                String  weatherIconId           = jsonDoc["weather"][0]["icon"].as<String>();
                char    tempReducedPrecison[5]  = { 0 };
                String  temperatureStrResult    = "\\calign";
                String  weatherConditionIcon;

                /* Reduce temperature precision */
                (void)snprintf(tempReducedPrecison, sizeof(tempReducedPrecison), "%.1f", temperature);
                temperatureStrResult += tempReducedPrecison;

                /* Add unit °C */
                temperatureStrResult += "\x8E";
                temperatureStrResult += "C";

                /* Handle icon depended on weather icon id.
                 * See https://openweathermap.org/weather-conditions
                 * 
                 * First check whether there is a specific icon available.
                 * If not, check for a generic weather icon.
                 * If this is not available too, use the standard OpenWeather icon.
                 */
                weatherConditionIcon = IMAGE_PATH + weatherIconId + ".bmp";
                if (false == FILESYSTEM.exists(weatherConditionIcon))
                {
                    weatherConditionIcon  = IMAGE_PATH + weatherIconId.substring(0U, weatherIconId.length() - 1U);
                    weatherConditionIcon += ".bmp";
                }

                lock();

                if (false == m_bitmapWidget.load(FILESYSTEM, weatherConditionIcon))
                {
                    (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);
                }

                m_textWidget.setFormatStr(temperatureStrResult);

                unlock();
            }
        }
    });

    m_client.regOnClosed([this]() {
        LOG_INFO("Connection closed.");

        lock();
        if (true == m_isConnectionError)
        {
            /* If a request fails, show standard icon and a '?' */
            (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);
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

bool OpenWeatherPlugin::saveConfiguration()
{
    bool                status                  = true;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    String              configurationFilename   = getFullPathToConfiguration();

    jsonDoc["apiKey"] = m_apiKey;
    jsonDoc["cityId"] = m_cityId;
    
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

bool OpenWeatherPlugin::loadConfiguration()
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
    else if (false == jsonDoc["apiKey"].is<String>())
    {
        LOG_WARNING("API key not found or invalid type.");
        status = false;
    }
    else if (false == jsonDoc["cityId"].is<String>())
    {
        LOG_WARNING("City id not found or invalid type.");
        status = false;
    }
    else
    {
        m_apiKey = jsonDoc["apiKey"].as<String>();
        m_cityId = jsonDoc["cityId"].as<String>();
    }

    return status;
}

void OpenWeatherPlugin::lock() const
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreTakeRecursive(m_xMutex, portMAX_DELAY);
    }

    return;
}

void OpenWeatherPlugin::unlock() const
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

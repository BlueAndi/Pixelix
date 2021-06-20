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

/** Divider to convert ms in s */
#define MS_TO_SEC_DIVIDER                       (1000U)

/**
 * Toggle counter value to switch between general weather data and additional information.
 * if DURATION_INFINITE was set for the plugin.
 */
#define MAX_COUNTER_VALUE_FOR_DURATION_INFINITE (15U)

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

/* Initialize image path for uvi icon. */
const char* OpenWeatherPlugin::IMAGE_PATH_UVI_ICON      = "/images/uvi.bmp";

/* Initialize image path for humidity icon. */
const char* OpenWeatherPlugin::IMAGE_PATH_HUMIDITY_ICON = "/images/hum.bmp";

/* Initialize image path for uvi icon. */
const char* OpenWeatherPlugin::IMAGE_PATH_WIND_ICON     = "/images/wind.bmp";

/* Initialize image path for the weather condition icons. */
const char* OpenWeatherPlugin::IMAGE_PATH               = "/images/";

/* Initialize OpenWeather base URI */
const char* OpenWeatherPlugin::OPEN_WEATHER_BASE_URI    = "http://api.openweathermap.org";

/* Initialize plugin topic. */
const char* OpenWeatherPlugin::TOPIC                    = "/weather";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void OpenWeatherPlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC);
}

bool OpenWeatherPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC))
    {
        value["apiKey"]   = getApiKey();
        value["lat"]      = getLatitude();
        value["lon"]      = getLongitude();
        value["other"]    = static_cast<int>(getAdditionalInformation());
        value["units"]    = getUnits();

        isSuccessful = true;
    }

    return isSuccessful;
}

bool OpenWeatherPlugin::setTopic(const String& topic, const JsonObject& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC))
    {
        if (false == value["apiKey"].isNull())
        {
            setApiKey(value["apiKey"].as<String>());
            isSuccessful = true;
        }

        if (false == value["lat"].isNull())
        {
            setLatitude(value["lat"].as<String>());
            isSuccessful = true;
        }
        
        if (false == value["lon"].isNull())
        {
            setLongitude(value["lon"].as<String>());
            isSuccessful = true;
        }

        if (false == value["other"].isNull())
        {
            OtherWeatherInformation other = static_cast<OtherWeatherInformation>(value["other"].as<uint8_t>());

            setAdditionalInformation(other);
            isSuccessful = true;
        }

        if (false == value["units"].isNull())
        {
            setUnits(value["units"].as<String>());
            isSuccessful = true;
        }
    }

    return isSuccessful;
}

void OpenWeatherPlugin::setSlot(const ISlotPlugin* slotInterf)
{
    m_slotInterf = slotInterf;
    return;
}

void OpenWeatherPlugin::start(uint16_t width, uint16_t height)
{
    lock();

    if (nullptr == m_iconCanvas)
    {
        m_iconCanvas = new Canvas(ICON_WIDTH, ICON_HEIGHT, 0, 0);

        if (nullptr != m_iconCanvas)
        {
            (void)m_iconCanvas->addWidget(m_bitmapWidget);

            /* Load icon from filesystem. */
            (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);
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

    unlock();

    return;
}

void OpenWeatherPlugin::process()
{
    Msg msg;

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

    if ((true == m_updateContentTimer.isTimerRunning()) &&
        (true == m_updateContentTimer.isTimeout()))
    {
        updateDisplay(false);
        m_updateContentTimer.restart();
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
                (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);
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
    
    unlock();

    return;
}

void OpenWeatherPlugin::active(YAGfx& gfx)
{
    lock();

    /* Load configuration, because it may be changed by web request
     * or direct editing.
     */
    (void)loadConfiguration();

    /* Force immediate weather update on activation */
    updateDisplay(true);

    /* Force drawing on display in the update() method for the very first time
     * after activation.
     */
    m_isUpdateAvailable = true;
    m_durationCounter = 0U;
    m_updateContentTimer.start(DURATION_TICK_PERIOD);

    unlock();

    return;
}

void OpenWeatherPlugin::inactive()
{
    lock();

    m_updateContentTimer.stop();

    unlock();

    return;
}

void OpenWeatherPlugin::update(YAGfx& gfx)
{
    lock();

    if (false != m_isUpdateAvailable)
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

        m_isUpdateAvailable = false;
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

    if (apiKey != m_apiKey)
    {
        m_apiKey = apiKey;

        (void)saveConfiguration();
    }

    unlock();

    return;
}

String OpenWeatherPlugin::getLatitude() const
{
    String latitude;

    lock();
    latitude = m_latitude;
    unlock();

    return latitude;
}

void OpenWeatherPlugin::setLatitude(const String& latitude)
{
    lock();

    if (latitude != m_latitude)
    {
        m_latitude = latitude;

        (void)saveConfiguration();
    }

    unlock();

    return;
}

String OpenWeatherPlugin::getLongitude() const
{
    String longitude;

    lock();
    longitude = m_longitude;
    unlock();

    return longitude;
}

void OpenWeatherPlugin::setLongitude(const String& longitude)
{
    lock();

    if (longitude != m_longitude)
    {
        m_longitude = longitude;

        (void)saveConfiguration();
    }

    unlock();

    return;
}

OpenWeatherPlugin::OtherWeatherInformation OpenWeatherPlugin::getAdditionalInformation() const
{
    OtherWeatherInformation additionalInformation;

    lock();
    additionalInformation = m_additionalInformation;
    unlock();

    return additionalInformation;
}

void OpenWeatherPlugin::setAdditionalInformation(const OtherWeatherInformation& additionalInformation)
{
    lock();

    if (additionalInformation != m_additionalInformation)
    {
        m_additionalInformation = additionalInformation;

        (void)saveConfiguration();
    }

    unlock();

    return;
}

String OpenWeatherPlugin::getUnits() const
{
    String units;

    lock();
    units = m_units;
    unlock();

    return units;
}

void OpenWeatherPlugin::setUnits(const String& units)
{
    lock();

    if (units != m_units)
    {
        m_units = units;

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

String OpenWeatherPlugin::uvIndexToColor(float uvIndex)
{
    String color;

    if ((0.0f <= uvIndex) && (3.0f > uvIndex))
    {
        color = "\\#c0ffa0"; 
    }
    else if ((3.0f <= uvIndex) && (6.0f > uvIndex))
    {
        color = "\\#f8f140";
    }
    else if ((6.0f <= uvIndex) && (8.0f > uvIndex))
    {
        color = "\\#f77820";
    }
    else if ((8.0f <= uvIndex) && (11.0f > uvIndex))
    {
        color = "\\#d80020";
    }
    else 
    {
        color = "\\#a80081";
    }

    return color;
}

void OpenWeatherPlugin::updateDisplay(bool force)
{
    bool        showGeneralWeatherInformation = ((0U == m_durationCounter) ? true : false);
    bool        showAdditionalInformation = false;
    uint32_t    duration = (nullptr == m_slotInterf) ? 0U : m_slotInterf->getDuration();
    String      icon;
    String      text;

    /* If infinite duration was set switch every 15s between time and date. */
    if (0U == duration)
    {
        showAdditionalInformation = ((MAX_COUNTER_VALUE_FOR_DURATION_INFINITE == m_durationCounter) ? true : false);
    }
    else
    {
        showAdditionalInformation = ((duration / (2U * MS_TO_SEC_DIVIDER) == m_durationCounter) ? true : false);
    }

    m_durationCounter++;

    if ((false != showGeneralWeatherInformation) || (true == force))
    {
        icon = m_currentWeatherIcon;
        text = m_currentTemp;

        if (false == m_bitmapWidget.load(FILESYSTEM, icon))
        {
            (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);
        }

        m_textWidget.setFormatStr(text);

        m_isUpdateAvailable = true;

    }

    if (false != showAdditionalInformation)
    {
        switch (m_additionalInformation)
        {
        case UVI:
            text = m_currentUvIndex;
            icon = IMAGE_PATH_UVI_ICON;
            break;

        case HUMIDITY:
            text = m_currentHumidity;
            icon = IMAGE_PATH_HUMIDITY_ICON;
            break;

        case WIND:
            text = m_currentWindspeed;
            icon = IMAGE_PATH_WIND_ICON;
            break;

        case OFF:
            text = m_currentTemp;
            icon = m_currentWeatherIcon;
            break;

        default:
            break;
        }

        if (false == m_bitmapWidget.load(FILESYSTEM, icon))
        {
            (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);
        }

        m_textWidget.setFormatStr(text);
        m_isUpdateAvailable = true;
    }

    /* If infinite duration was switch every 15s between general and additional information. */
    if (0U == duration)
    {
        if ((2U * MAX_COUNTER_VALUE_FOR_DURATION_INFINITE) == m_durationCounter)
        {
            m_durationCounter = 0U;
        }
    }
    else
    {
        if ((duration / MS_TO_SEC_DIVIDER) == m_durationCounter)
        {
            m_durationCounter = 0U;
        }
    }
}

bool OpenWeatherPlugin::startHttpRequest()
{
    bool status = false;

    if ((0 < m_latitude.length()) &&
        (0 < m_longitude.length()) &&
        (0 < m_units.length()) &&
        (0 < m_apiKey.length()))
    {
        String url = OPEN_WEATHER_BASE_URI;

        /* Get current weather information: https://openweathermap.org/api/one-call-api */
        url += "/data/2.5/onecall?lat=";
        url += m_latitude;
        url += "&lon=";
        url += m_longitude;
        url += "&units=";
        url += m_units;
        url += "&appid=";
        url += m_apiKey;
        url += "&exclude=minutely,hourly,daily,alerts";

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
    /* Note: All registered callbacks are running in a different task context!
     *       Therefore it is not allowed to access a member here directly.
     *       The processing must be deferred via task proxy.
     */
    m_client.regOnResponse(
        [this](const HttpResponse& rsp)
        {
            const size_t            JSON_DOC_SIZE   = 256U;
            DynamicJsonDocument*    jsonDoc         = new DynamicJsonDocument(JSON_DOC_SIZE);

            if (nullptr != jsonDoc)
            {
                size_t                          payloadSize             = 0U;
                const char*                     payload                 = reinterpret_cast<const char*>(rsp.getPayload(payloadSize));
                const size_t                    FILTER_SIZE             = 128U;
                StaticJsonDocument<FILTER_SIZE> filter;
                JsonObject                      filterCurrent           = filter.createNestedObject("current");
                DeserializationError            error;

                /* See https://openweathermap.org/api/one-call-api for an example of API response. */
                filterCurrent["temp"]                  = true;
                filterCurrent["uvi"]                   = true;
                filterCurrent["humidity"]              = true;
                filterCurrent["wind_speed"]            = true;
                filterCurrent["weather"][0]["icon"]    = true;
                
                if (true == filter.overflowed())
                {
                    LOG_ERROR("Less memory for filter available.");
                }

                error = deserializeJson(*jsonDoc, payload, payloadSize, DeserializationOption::Filter(filter));

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

void OpenWeatherPlugin::handleWebResponse(DynamicJsonDocument& jsonDoc)
{
    JsonObject current = jsonDoc["current"];

    if (false == current["temp"].is<float>())
    {
        LOG_WARNING("JSON temperature type missmatch or missing.");
    }
    else if (false == current["uvi"].is<float>())
    {
        LOG_WARNING("JSON uvi type missmatch or missing.");
    }
    else if (false == current["humidity"].is<int>())
    {
        LOG_WARNING("JSON wind_speed type missmatch or missing.");
    }
    else if (false == current["wind_speed"].is<float>())
    {
        LOG_WARNING("JSON uvi type missmatch or missing.");
    }
    else if (false == current["weather"][0]["icon"].is<String>())
    {
        LOG_WARNING("JSON weather icon id type missmatch or missing.");
    }
    else
    {
        float   temperature             = current["temp"].as<float>();
        String  weatherIconId           = current["weather"][0]["icon"].as<String>();
        float   uvIndex                 = current["uvi"].as<float>();
        int     humidity                = current["humidity"].as<int>();
        float   windSpeed               = current["wind_speed"].as<float>();
        char    tempReducedPrecison[6]  = { 0 };
        char    windReducedPrecison[5]  = { 0 };
        String  weatherConditionIcon;

        /* Generate UV-Index string and adapt color of string accordingly. */
        m_currentUvIndex = "\\calign";
        m_currentUvIndex += uvIndexToColor(uvIndex);
        m_currentUvIndex += uvIndex;

        const char* reducePrecision = (temperature < -9.9f) ? "%.0f" : "%.1f";

        /* Generate temperature string with reduced precision and add unit °C/°F. */
        (void)snprintf(tempReducedPrecison, sizeof(tempReducedPrecison), reducePrecision, temperature);

        m_currentTemp  = "\\calign";
        m_currentTemp += tempReducedPrecison;
        m_currentTemp += "\x8E";
        m_currentTemp += (m_units == "metric")?"C":"F";

        /* Generate humidity string */
        m_currentHumidity = "\\calign";
        m_currentHumidity += humidity;
        m_currentHumidity += "%";

        /* Generate windapeed string and add unit.*/
        (void)snprintf(windReducedPrecison, sizeof(windReducedPrecison), "%.1f", windSpeed);
        m_currentWindspeed = "\\calign";
        m_currentWindspeed += windReducedPrecison;
        m_currentWindspeed += "m/s";

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

        m_currentWeatherIcon = weatherConditionIcon;

        updateDisplay(false);
    }
}


bool OpenWeatherPlugin::saveConfiguration() const
{
    bool                status                  = true;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    String              configurationFilename   = getFullPathToConfiguration();

    jsonDoc["apiKey"]   = m_apiKey;
    jsonDoc["lat"]      = m_latitude;
    jsonDoc["lon"]      = m_longitude;
    jsonDoc["other"]    = static_cast<int>(m_additionalInformation);
    jsonDoc["units"]    = m_units;
    
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
    else if (false == jsonDoc["lat"].is<String>())
    {
        LOG_WARNING("Latitude not found or invalid type.");
        status = false;
    }
    else if (false == jsonDoc["lon"].is<String>())
    {
        LOG_WARNING("Longitude not found or invalid type.");
        status = false;
    }
     else if (false == jsonDoc["other"].is<int>())
    {
        LOG_WARNING("other not found or invalid type.");
        status = false;
    }
    else if (false == jsonDoc["units"].is<String>())
    {
        LOG_WARNING("Units not found or invalid type.");
        status = false;
    }
    else
    {
        m_apiKey                = jsonDoc["apiKey"].as<String>();
        m_latitude              = jsonDoc["lat"].as<String>();
        m_longitude             = jsonDoc["lon"].as<String>();
        m_additionalInformation = static_cast<OtherWeatherInformation>(jsonDoc["other"].as<int>());
        m_units                 = jsonDoc["units"].as<String>();
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

void OpenWeatherPlugin::clearQueue()
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

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
const char* OpenWeatherPlugin::OPEN_WEATHER_BASE_URI    = "https://api.openweathermap.org";

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
        JsonVariant jsonApiKey  = value["apiKey"];
        JsonVariant jsonLat     = value["lat"];
        JsonVariant jsonLon     = value["lon"];
        JsonVariant jsonOther   = value["other"];
        JsonVariant jsonUnits   = value["units"];

        if (false == jsonApiKey.isNull())
        {
            setApiKey(jsonApiKey.as<String>());
            isSuccessful = true;
        }

        if (false == jsonLat.isNull())
        {
            setLatitude(jsonLat.as<String>());
            isSuccessful = true;
        }
        
        if (false == jsonLon.isNull())
        {
            setLongitude(jsonLon.as<String>());
            isSuccessful = true;
        }

        if (false == jsonOther.isNull())
        {
            OtherWeatherInformation other = static_cast<OtherWeatherInformation>(jsonOther.as<uint8_t>());

            setAdditionalInformation(other);
            isSuccessful = true;
        }

        if (false == jsonUnits.isNull())
        {
            setUnits(jsonUnits.as<String>());
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
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_iconCanvas.setPosAndSize(0, 0, ICON_WIDTH, ICON_HEIGHT);
    (void)m_iconCanvas.addWidget(m_bitmapWidget);

    (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);

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

    initHttpClient();

    return;
}

void OpenWeatherPlugin::stop()
{
    String                      configurationFilename = getFullPathToConfiguration();
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_requestTimer.stop();

    if (false != FILESYSTEM.remove(configurationFilename))
    {
        LOG_INFO("File %s removed", configurationFilename.c_str());
    }

    return;
}

void OpenWeatherPlugin::process(bool isConnected)
{
    Msg                         msg;
    MutexGuard<MutexRecursive>  guard(m_mutex);

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
                (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);
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
                (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);
                m_textWidget.setFormatStr("\\calign?");

                m_requestTimer.start(UPDATE_PERIOD_SHORT);
            }
            else
            {
                m_requestTimer.start(UPDATE_PERIOD);
            }
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

    return;
}

void OpenWeatherPlugin::active(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

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

    return;
}

void OpenWeatherPlugin::inactive()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_updateContentTimer.stop();

    return;
}

void OpenWeatherPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (false != m_isUpdateAvailable)
    {
        gfx.fillScreen(ColorDef::BLACK);
        m_iconCanvas.update(gfx);
        m_textCanvas.update(gfx);

        m_isUpdateAvailable = false;
    }

    return;
}

String OpenWeatherPlugin::getApiKey() const
{
    String                      apiKey;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    apiKey = m_apiKey;

    return apiKey;
}

void OpenWeatherPlugin::setApiKey(const String& apiKey)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (apiKey != m_apiKey)
    {
        m_apiKey = apiKey;

        (void)saveConfiguration();
    }

    return;
}

String OpenWeatherPlugin::getLatitude() const
{
    String                      latitude;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    latitude = m_latitude;

    return latitude;
}

void OpenWeatherPlugin::setLatitude(const String& latitude)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (latitude != m_latitude)
    {
        m_latitude = latitude;

        (void)saveConfiguration();
    }

    return;
}

String OpenWeatherPlugin::getLongitude() const
{
    String                      longitude;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    longitude = m_longitude;

    return longitude;
}

void OpenWeatherPlugin::setLongitude(const String& longitude)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (longitude != m_longitude)
    {
        m_longitude = longitude;

        (void)saveConfiguration();
    }

    return;
}

OpenWeatherPlugin::OtherWeatherInformation OpenWeatherPlugin::getAdditionalInformation() const
{
    OtherWeatherInformation     additionalInformation;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    additionalInformation = m_additionalInformation;

    return additionalInformation;
}

void OpenWeatherPlugin::setAdditionalInformation(const OtherWeatherInformation& additionalInformation)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (additionalInformation != m_additionalInformation)
    {
        m_additionalInformation = additionalInformation;

        (void)saveConfiguration();
    }

    return;
}

String OpenWeatherPlugin::getUnits() const
{
    String                      units;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    units = m_units;

    return units;
}

void OpenWeatherPlugin::setUnits(const String& units)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (units != m_units)
    {
        m_units = units;

        (void)saveConfiguration();
    }

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

    if ((false == m_latitude.isEmpty()) &&
        (false == m_longitude.isEmpty()) &&
        (false == m_units.isEmpty()) &&
        (false == m_apiKey.isEmpty()))
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
            DynamicJsonDocument*    jsonDoc         = new(std::nothrow) DynamicJsonDocument(JSON_DOC_SIZE);

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
    JsonVariant jsonCurrent     = jsonDoc["current"];
    JsonVariant jsonTemperature = jsonCurrent["temp"];
    JsonVariant jsonUvi         = jsonCurrent["uvi"];
    JsonVariant jsonHumidity    = jsonCurrent["humidity"];
    JsonVariant jsonWindSpeed   = jsonCurrent["wind_speed"];
    JsonVariant jsonIcon        = jsonCurrent["weather"][0]["icon"];

    if (false == jsonTemperature.is<float>())
    {
        LOG_WARNING("JSON temp type mismatch or missing.");
    }
    else if (false == jsonUvi.is<float>())
    {
        LOG_WARNING("JSON uvi type mismatch or missing.");
    }
    else if (false == jsonHumidity.is<int>())
    {
        LOG_WARNING("JSON humidity type mismatch or missing.");
    }
    else if (false == jsonWindSpeed.is<float>())
    {
        LOG_WARNING("JSON wind_speed type mismatch or missing.");
    }
    else if (false == jsonIcon.is<String>())
    {
        LOG_WARNING("JSON weather icon id type mismatch or missing.");
    }
    else
    {
        float   temperature             = jsonTemperature.as<float>();
        String  weatherIconId           = jsonIcon.as<String>();
        float   uvIndex                 = jsonUvi.as<float>();
        int     humidity                = jsonHumidity.as<int>();
        float   windSpeed               = jsonWindSpeed.as<float>();
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
    else
    {
        JsonVariant jsonApiKey  = jsonDoc["apiKey"];
        JsonVariant jsonLat     = jsonDoc["lat"];
        JsonVariant jsonLon     = jsonDoc["lon"];
        JsonVariant jsonOther   = jsonDoc["other"];
        JsonVariant jsonUnits   = jsonDoc["units"];

        if (false == jsonApiKey.is<String>())
        {
            LOG_WARNING("API key not found or invalid type.");
            status = false;
        }
        else if (false == jsonLat.is<String>())
        {
            LOG_WARNING("Latitude not found or invalid type.");
            status = false;
        }
        else if (false == jsonLon.is<String>())
        {
            LOG_WARNING("Longitude not found or invalid type.");
            status = false;
        }
        else if (false == jsonOther.is<int>())
        {
            LOG_WARNING("other not found or invalid type.");
            status = false;
        }
        else if (false == jsonUnits.is<String>())
        {
            LOG_WARNING("Units not found or invalid type.");
            status = false;
        }
        else
        {
            m_apiKey                = jsonApiKey.as<String>();
            m_latitude              = jsonLat.as<String>();
            m_longitude             = jsonLon.as<String>();
            m_additionalInformation = static_cast<OtherWeatherInformation>(jsonOther.as<int>());
            m_units                 = jsonUnits.as<String>();
        }
    }

    return status;
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

/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Open-Meteo plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "OpenMeteoPlugin.h"

#include <Logging.h>
#include <ArduinoJson.h>
#include <Util.h>
#include <math.h>
#include <HttpStatus.h>

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

/* Initialize Open-Meteo base URI.
 * Use http:// instead of https:// for less required heap memory for SSL connection.
 */
const char* OpenMeteoPlugin::OPEN_METEO_BASE_URI = "http://api.open-meteo.com";

/* Initialize plugin topic. */
const char* OpenMeteoPlugin::TOPIC_CONFIG        = "omweather";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void OpenMeteoPlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC_CONFIG);
}

bool OpenMeteoPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_CONFIG))
    {
        getConfiguration(value);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool OpenMeteoPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_CONFIG))
    {
        const size_t        JSON_DOC_SIZE = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg             = jsonDoc.to<JsonObject>();
        JsonVariantConst    jsonUpdatePeriod    = value["updatePeriod"];
        JsonVariantConst    jsonLatitude        = value["latitude"];
        JsonVariantConst    jsonLongitude       = value["longitude"];
        JsonVariantConst    jsonTemperatureUnit = value["temperatureUnit"];
        JsonVariantConst    jsonWindSpeedUnit   = value["windSpeedUnit"];
        JsonVariantConst    jsonWeatherInfo     = value["weatherInfo"];

        /* The received configuration may not contain all single key/value pair.
         * Therefore read first the complete internal configuration and
         * overwrite them with the received ones.
         */
        getConfiguration(jsonCfg);

        /* Note:
         * Check only for the key/value pair availability.
         * The type check will follow in the setConfiguration().
         */

        if (false == jsonUpdatePeriod.isNull())
        {
            jsonCfg["updatePeriod"] = jsonUpdatePeriod.as<uint32_t>();
            isSuccessful            = true;
        }

        if (false == jsonLatitude.isNull())
        {
            jsonCfg["latitude"] = jsonLatitude.as<String>();
            isSuccessful        = true;
        }

        if (false == jsonLongitude.isNull())
        {
            jsonCfg["longitude"] = jsonLongitude.as<String>();
            isSuccessful         = true;
        }

        if (false == jsonTemperatureUnit.isNull())
        {
            jsonCfg["temperatureUnit"] = jsonTemperatureUnit.as<String>();
            isSuccessful               = true;
        }

        if (false == jsonWindSpeedUnit.isNull())
        {
            jsonCfg["windSpeedUnit"] = jsonWindSpeedUnit.as<String>();
            isSuccessful             = true;
        }

        if (false == jsonWeatherInfo.isNull())
        {
            jsonCfg["weatherInfo"] = jsonWeatherInfo.as<uint32_t>();
            isSuccessful           = true;
        }

        if (true == isSuccessful)
        {
            JsonObjectConst jsonCfgConst = jsonCfg;

            isSuccessful                 = setConfiguration(jsonCfgConst);

            if (true == isSuccessful)
            {
                requestStoreToPersistentMemory();
            }
        }
    }

    return isSuccessful;
}

bool OpenMeteoPlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive> guard(m_mutex);
    bool                       hasTopicChanged = m_hasTopicChanged;

    /* Only a single topic, therefore its not necessary to check. */
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void OpenMeteoPlugin::setSlot(const ISlotPlugin* slotInterf)
{
    m_slotInterf = slotInterf;
}

void OpenMeteoPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.init(width, height);
    setViewUnits();

    PluginWithConfig::start(width, height);

    initHttpClient();
}

void OpenMeteoPlugin::stop()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_requestTimer.stop();

    PluginWithConfig::stop();
}

void OpenMeteoPlugin::active(YAGfx& gfx)
{
    UTIL_NOT_USED(gfx);
    m_view.restartWeatherInfo();
}

void OpenMeteoPlugin::inactive()
{
    /* Nothing to do. */
}

void OpenMeteoPlugin::process(bool isConnected)
{
    Msg                        msg;
    MutexGuard<MutexRecursive> guard(m_mutex);
    bool                       isRestRequestRequired = false;

    PluginWithConfig::process(isConnected);

    /* Only if a network connection is established the required information
     * shall be periodically requested via REST API.
     */
    if (false == m_requestTimer.isTimerRunning())
    {
        if (true == isConnected)
        {
            isRestRequestRequired = true;
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
            isRestRequestRequired = true;
        }
    }

    /* Request of new weather information via REST API required? */
    if (true == isRestRequestRequired)
    {
        if (false == startHttpRequest())
        {
            m_requestTimer.start(UPDATE_PERIOD_SHORT);
        }
        else
        {
            m_requestTimer.start(m_updatePeriod);
        }
    }

    if (nullptr != m_slotInterf)
    {
        m_view.setViewDuration(m_slotInterf->getDuration());
    }

    if (true == m_taskProxy.receive(msg))
    {
        switch (msg.type)
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
}

void OpenMeteoPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.update(gfx);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void OpenMeteoPlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["updatePeriod"]    = m_updatePeriod / (60U * 1000U); /* Conversion from ms to minutes. */
    jsonCfg["latitude"]        = m_latitude;
    jsonCfg["longitude"]       = m_longitude;
    jsonCfg["temperatureUnit"] = m_temperatureUnit;
    jsonCfg["windSpeedUnit"]   = m_windUnit;
    jsonCfg["weatherInfo"]     = m_view.getWeatherInfo();
}

bool OpenMeteoPlugin::setConfiguration(const JsonObjectConst& jsonCfg)
{
    bool             status                    = false;
    JsonVariantConst jsonUpdatePeriod          = jsonCfg["updatePeriod"];
    JsonVariantConst jsonLatitude              = jsonCfg["latitude"];
    JsonVariantConst jsonLongitude             = jsonCfg["longitude"];
    JsonVariantConst jsonTemperatureUnit       = jsonCfg["temperatureUnit"];
    JsonVariantConst jsonWindSpeedUnit         = jsonCfg["windSpeedUnit"];
    JsonVariantConst jsonWeatherInfo           = jsonCfg["weatherInfo"];

    const uint32_t   UPDATE_PERIOD_LOWER_LIMIT = 1U;   /* minutes */
    const uint32_t   UPDATE_PERIOD_UPPER_LIMIT = 120U; /* minutes */

    if (false == jsonUpdatePeriod.is<uint32_t>())
    {
        LOG_WARNING("Update period not found or invalid type.");
    }
    else if (false == jsonLatitude.is<String>())
    {
        LOG_WARNING("Latitude not found or invalid type.");
    }
    else if (false == jsonLongitude.is<String>())
    {
        LOG_WARNING("Longitude not found or invalid type.");
    }
    else if (false == jsonTemperatureUnit.is<String>())
    {
        LOG_WARNING("Temperature unit not found or invalid type.");
    }
    else if (false == jsonWindSpeedUnit.is<String>())
    {
        LOG_WARNING("Wind unit not found or invalid type.");
    }
    else if (false == jsonWeatherInfo.is<uint32_t>())
    {
        LOG_WARNING("Weather info not found or invalid type.");
    }
    else
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        m_updatePeriod = jsonUpdatePeriod.as<uint32_t>();

        if ((UPDATE_PERIOD_LOWER_LIMIT > m_updatePeriod) ||
            (UPDATE_PERIOD_UPPER_LIMIT < m_updatePeriod))
        {
            m_updatePeriod = UPDATE_PERIOD;
        }
        else
        {
            m_updatePeriod = SIMPLE_TIMER_MINUTES(m_updatePeriod);
        }

        m_latitude        = jsonLatitude.as<String>();
        m_longitude       = jsonLongitude.as<String>();
        m_temperatureUnit = jsonTemperatureUnit.as<String>();
        m_windUnit        = jsonWindSpeedUnit.as<String>();

        m_view.setWeatherInfo(jsonWeatherInfo.as<uint32_t>());
        setViewUnits();

        /* Force update on display */
        m_requestTimer.start(UPDATE_PERIOD_SHORT);

        m_hasTopicChanged = true;

        status            = true;
    }

    return status;
}

bool OpenMeteoPlugin::startHttpRequest()
{
    bool status = false;

    if ((false == m_latitude.isEmpty()) &&
        (false == m_longitude.isEmpty()) &&
        (false == m_temperatureUnit.isEmpty()) &&
        (false == m_windUnit.isEmpty()))
    {
        String url  = OPEN_METEO_BASE_URI;

        /* Documentation:
         * https://open-meteo.com/en/docs#current=temperature_2m,relative_humidity_2m,is_day,weather_code,wind_speed_10m&hourly=&daily=weather_code,temperature_2m_max,temperature_2m_min,uv_index_max
         */
        url        += "/v1/forecast?latitude=";
        url        += m_latitude;
        url        += "&longitude=";
        url        += m_longitude;
        url        += "&current=temperature_2m,relative_humidity_2m,is_day,weather_code,wind_speed_10m,uv_index";
        url        += "&daily=weather_code,temperature_2m_max,temperature_2m_min";
        url        += "&timezone=auto";
        url        += "&temperature_unit=";
        url        += m_temperatureUnit;
        url        += "&wind_speed_unit=";
        url        += m_windUnit;

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

void OpenMeteoPlugin::initHttpClient()
{
    /* Note: All registered callbacks are running in a different task context!
     *       Therefore it is not allowed to access a member here directly.
     *       The processing must be deferred via task proxy.
     */
    m_client.regOnResponse(
        [this](const HttpResponse& rsp) {
            handleAsyncWebResponse(rsp);
        });

    m_client.regOnClosed(
        [this]() {
            Msg msg;

            msg.type = MSG_TYPE_CONN_CLOSED;

            (void)this->m_taskProxy.send(msg);
        });

    m_client.regOnError(
        [this]() {
            Msg msg;

            msg.type = MSG_TYPE_CONN_ERROR;

            (void)this->m_taskProxy.send(msg);
        });
}

void OpenMeteoPlugin::handleAsyncWebResponse(const HttpResponse& rsp)
{
    if (HttpStatus::STATUS_CODE_OK == rsp.getStatusCode())
    {
        bool                 isSuccessful  = false;
        const size_t         JSON_DOC_SIZE = 2048U;
        DynamicJsonDocument* jsonDoc       = new (std::nothrow) DynamicJsonDocument(JSON_DOC_SIZE);

        if (nullptr != jsonDoc)
        {
            size_t              payloadSize = 0U;
            const void*         vPayload    = rsp.getPayload(payloadSize);
            const char*         payload     = static_cast<const char*>(vPayload);
            const size_t        FILTER_SIZE = 640U;
            DynamicJsonDocument jsonFilterDoc(FILTER_SIZE);

            /* Example:
                {
                    "latitude": 52.52,
                    "longitude": 13.419998,
                    "generationtime_ms": 0.1684427261352539,
                    "utc_offset_seconds": 0,
                    "timezone": "GMT",
                    "timezone_abbreviation": "GMT",
                    "elevation": 38.0,
                    "current_units": {
                        "time": "iso8601",
                        "interval": "seconds",
                        "temperature_2m": "°C",
                        "relative_humidity_2m": "%",
                        "is_day": "",
                        "weather_code": "wmo code",
                        "wind_speed_10m": "m/s",
                        "uv_index": ""
                    },
                    "current": {
                        "time": "2025-02-01T17:15",
                        "interval": 900,
                        "temperature_2m": 3.1,
                        "relative_humidity_2m": 87,
                        "is_day": 0,
                        "weather_code": 2,
                        "wind_speed_10m": 1.36,
                        "uv_index": 0.00
                    },
                    "daily_units": {
                        "time": "iso8601",
                        "weather_code": "wmo code",
                        "temperature_2m_max": "°C",
                        "temperature_2m_min": "°C"
                    },
                    "daily": {
                        "time": [
                            "2025-02-01",
                            "2025-02-02",
                            "2025-02-03",
                            "2025-02-04",
                            "2025-02-05",
                            "2025-02-06",
                            "2025-02-07"
                        ],
                        "weather_code": [
                            45,
                            45,
                            45,
                            3,
                            3,
                            3,
                            3
                        ],
                        "temperature_2m_max": [
                            4.6,
                            1.8,
                            2.3,
                            3.5,
                            2.4,
                            5.4,
                            2.4
                        ],
                        "temperature_2m_min": [
                            0.5,
                            -1.0,
                            -2.7,
                            -1.4,
                            -1.6,
                            0.6,
                            -0.5
                        ]
                    }
                }

            */

            jsonFilterDoc["current"]["temperature_2m"]       = true;
            jsonFilterDoc["current"]["relative_humidity_2m"] = true;
            jsonFilterDoc["current"]["is_day"]               = true;
            jsonFilterDoc["current"]["weather_code"]         = true;
            jsonFilterDoc["current"]["wind_speed_10m"]       = true;
            jsonFilterDoc["current"]["uv_index"]             = true;

            jsonFilterDoc["daily"]["weather_code"]           = true;
            jsonFilterDoc["daily"]["temperature_2m_max"]     = true;
            jsonFilterDoc["daily"]["temperature_2m_min"]     = true;

            if (true == jsonFilterDoc.overflowed())
            {
                LOG_ERROR("Less memory for filter available.");
            }
            else if ((nullptr == payload) ||
                     (0U == payloadSize))
            {
                LOG_ERROR("No payload.");
            }
            else
            {
                DeserializationError error = deserializeJson(*jsonDoc, payload, payloadSize, DeserializationOption::Filter(jsonFilterDoc));

                if (DeserializationError::Ok != error.code())
                {
                    LOG_WARNING("JSON parse error: %s", error.c_str());
                }
                else
                {
                    Msg msg;

                    msg.type     = MSG_TYPE_RSP;
                    msg.rsp      = jsonDoc;

                    isSuccessful = this->m_taskProxy.send(msg);
                }
            }

            if (false == isSuccessful)
            {
                delete jsonDoc;
                jsonDoc = nullptr;
            }
        }

        /* If something went wrong, send a response with empty payload to
         * trigger state change in weather request status and not stuck.
         */
        if (false == isSuccessful)
        {
            Msg msg;

            msg.type = MSG_TYPE_RSP;
            msg.rsp  = nullptr;

            if (false == this->m_taskProxy.send(msg))
            {
                LOG_FATAL("Internal error.");
            }
        }
    }
}

void OpenMeteoPlugin::setViewUnits()
{
    /* Temperature unit */
    if (true == m_temperatureUnit.equals("fahrenheit"))
    {
        m_view.setTemperatureUnit("°F");
    }
    else if (true == m_temperatureUnit.equals("celsius"))
    {
        m_view.setTemperatureUnit("°C");
    }
    else
    {
        m_view.setTemperatureUnit("?");
    }

    /* Wind speed unit */
    if (true == m_windUnit.equals("mph"))
    {
        m_view.setWindSpeedUnit("mph");
    }
    else if (true == m_windUnit.equals("kmh"))
    {
        m_view.setWindSpeedUnit("km/h");
    }
    else if (true == m_windUnit.equals("ms"))
    {
        m_view.setWindSpeedUnit("m/s");
    }
    else if (true == m_windUnit.equals("kn"))
    {
        m_view.setWindSpeedUnit("kn");
    }
    else
    {
        m_view.setWindSpeedUnit("?");
    }
}

String OpenMeteoPlugin::getIconIdFromWeatherCode(uint8_t weatherCode, bool isDay)
{
    String         iconId;
    const uint16_t WEATHER_CODE_MIN_CLEAR_SKY        = 0U;
    const uint16_t WEATHER_CODE_MAX_CLEAR_SKY        = 19U;
    const uint16_t WEATHER_CODE_MIN_FEW_CLOUDS       = 20U;
    const uint16_t WEATHER_CODE_MAX_FEW_CLOUDS       = 29U;
    const uint16_t WEATHER_CODE_MIN_SCATTERED_CLOUDS = 30U;
    const uint16_t WEATHER_CODE_MAX_SCATTERED_CLOUDS = 39U;
    const uint16_t WEATHER_CODE_MIN_BROKEN_CLOUDS    = 40U;
    const uint16_t WEATHER_CODE_MAX_BROKEN_CLOUDS    = 44U;
    const uint16_t WEATHER_CODE_MIN_MIST             = 45U;
    const uint16_t WEATHER_CODE_MAX_MIST             = 49U;
    const uint16_t WEATHER_CODE_MIN_RAIN             = 50U;
    const uint16_t WEATHER_CODE_MAX_RAIN             = 69U;
    const uint16_t WEATHER_CODE_MIN_SNOW             = 70U;
    const uint16_t WEATHER_CODE_MAX_SNOW             = 79U;
    const uint16_t WEATHER_CODE_MIN_SHOWER_RAIN      = 80U;
    const uint16_t WEATHER_CODE_MAX_SHOWER_RAIN      = 89U;
    const uint16_t WEATHER_CODE_MIN_THUNDERSTORM     = 90U;
    const uint16_t WEATHER_CODE_MAX_THUNDERSTORM     = 99U;

    /* Weather codes:
     * https://www.nodc.noaa.gov/archive/arc0021/0002199/1.1/data/0-data/HTML/WMO-CODE/WMO4677.HTM
     *
     * | Weather          | OpenWeather | Open-Meteo |
     * | ---------------- | ----------- | ---------- |
     * | Clear sky        | 01d, 01n    | 00 - 19    |
     * | Few clouds       | 02d, 02n    | 20 - 29    |
     * | Scattered clouds | 03d, 03n    | 30 - 39    |
     * | Broken clouds    | 04d, 04n    | 40 - 44    |
     * | Mist             | 50d, 50n    | 45 - 49    |
     * | Rain             | 10d, 10n    | 50 - 69    |
     * | Snow             | 13d, 13n    | 70 - 79    |
     * | Shower rain      | 09d, 09n    | 80 - 89    |
     * | Thunderstorm     | 11d, 11n    | 90 - 99    |
     */

    /* Clear sky? */
    if ((WEATHER_CODE_MIN_CLEAR_SKY <= weatherCode) &&
        (WEATHER_CODE_MAX_CLEAR_SKY >= weatherCode))
    {
        iconId = "01";
    }
    /* Few clouds? */
    else if ((WEATHER_CODE_MIN_FEW_CLOUDS <= weatherCode) &&
             (WEATHER_CODE_MAX_FEW_CLOUDS >= weatherCode))
    {
        iconId = "02";
    }
    /* Scattered clouds? */
    else if ((WEATHER_CODE_MIN_SCATTERED_CLOUDS <= weatherCode) &&
             (WEATHER_CODE_MAX_SCATTERED_CLOUDS >= weatherCode))
    {
        iconId = "03";
    }
    /* Broken clouds? */
    else if ((WEATHER_CODE_MIN_BROKEN_CLOUDS <= weatherCode) &&
             (WEATHER_CODE_MAX_BROKEN_CLOUDS >= weatherCode))
    {
        iconId = "04";
    }
    /* Mist? */
    else if ((WEATHER_CODE_MIN_MIST <= weatherCode) &&
             (WEATHER_CODE_MAX_MIST >= weatherCode))
    {
        iconId = "50";
    }
    /* Rain? */
    else if ((WEATHER_CODE_MIN_RAIN <= weatherCode) &&
             (WEATHER_CODE_MAX_RAIN >= weatherCode))
    {
        iconId = "10";
    }
    /* Snow? */
    else if ((WEATHER_CODE_MIN_SNOW <= weatherCode) &&
             (WEATHER_CODE_MAX_SNOW >= weatherCode))
    {
        iconId = "13";
    }
    /* Shower rain? */
    else if ((WEATHER_CODE_MIN_SHOWER_RAIN <= weatherCode) &&
             (WEATHER_CODE_MAX_SHOWER_RAIN >= weatherCode))
    {
        iconId = "09";
    }
    /* Thunderstorm? */
    else if ((WEATHER_CODE_MIN_THUNDERSTORM <= weatherCode) &&
             (WEATHER_CODE_MAX_THUNDERSTORM >= weatherCode))
    {
        iconId = "11";
    }
    else
    {
        iconId = "01";
    }

    if (false == isDay)
    {
        iconId += "n";
    }
    else
    {
        iconId += "d";
    }

    return iconId;
}

void OpenMeteoPlugin::handleWebResponse(const DynamicJsonDocument& jsonDoc)
{


    if (true == jsonDoc.containsKey("current"))
    {
        _OpenMeteoPlugin::View::WeatherInfoCurrent currentWeatherInfo;
        uint8_t                                    weatherCode = jsonDoc["current"]["weather_code"].as<uint8_t>();
        bool                                       isDay       = jsonDoc["current"]["is_day"].as<bool>();

        currentWeatherInfo.humidity                            = jsonDoc["current"]["relative_humidity_2m"].as<uint8_t>();
        currentWeatherInfo.iconId                              = getIconIdFromWeatherCode(weatherCode, isDay);
        currentWeatherInfo.temperature                         = jsonDoc["current"]["temperature_2m"].as<float>();
        currentWeatherInfo.uvIndex                             = jsonDoc["current"]["uv_index"].as<float>();
        currentWeatherInfo.windSpeed                           = jsonDoc["current"]["wind_speed_10m"].as<float>();

        LOG_INFO("Icon id: %s", currentWeatherInfo.iconId.c_str());
        LOG_INFO("Temperature: %0.2f", currentWeatherInfo.temperature);
        LOG_INFO("Humidity: %u", currentWeatherInfo.humidity);
        LOG_INFO("UV-Index: %0.2f", currentWeatherInfo.uvIndex);
        LOG_INFO("Wind speed: %0.2f", currentWeatherInfo.windSpeed);

        m_view.setWeatherInfoCurrent(currentWeatherInfo);
    }

    if (true == _OpenMeteoPlugin::View::isWeatherForecastSupported())
    {
        if (true == jsonDoc.containsKey("daily"))
        {
            uint8_t                                     day;
            _OpenMeteoPlugin::View::WeatherInfoForecast weatherInfo;

            for (day = 0U; day < _OpenMeteoPlugin::View::FORECAST_DAYS; ++day)
            {
                weatherInfo.iconId         = getIconIdFromWeatherCode(jsonDoc["daily"]["weather_code"][day].as<uint16_t>(), true);
                weatherInfo.temperatureMin = jsonDoc["daily"]["temperature_2m_min"][day].as<float>();
                weatherInfo.temperatureMax = jsonDoc["daily"]["temperature_2m_max"][day].as<float>();

                LOG_INFO("Day: %u", day);
                LOG_INFO("Icon id: %s", weatherInfo.iconId.c_str());
                LOG_INFO("Temperature min.: %0.2f", weatherInfo.temperatureMin);
                LOG_INFO("Temperature max.: %0.2f", weatherInfo.temperatureMax);

                m_view.setWeatherInfoForecast(day, weatherInfo);
            }
        }
    }
}

void OpenMeteoPlugin::clearQueue()
{
    Msg msg;

    while (true == m_taskProxy.receive(msg))
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

/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
#include "OpenWeatherCurrent.h"
#include "OpenWeatherForecast.h"
#include "OpenWeatherOneCallCurrent.h"
#include "OpenWeatherOneCallForecast.h"

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

/* Initialize OpenWeather base URI.
 * Use http:// instead of https:// for less required heap memory for SSL connection.
 */
const char* OpenWeatherPlugin::OPEN_WEATHER_BASE_URI    = "http://api.openweathermap.org";

/* Initialize plugin topic. */
const char* OpenWeatherPlugin::TOPIC_CONFIG             = "/weather";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void OpenWeatherPlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC_CONFIG);
}

bool OpenWeatherPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_CONFIG))
    {
        getConfiguration(value);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool OpenWeatherPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_CONFIG))
    {
        const size_t        JSON_DOC_SIZE           = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg                 = jsonDoc.to<JsonObject>();
        JsonVariantConst    jsonSourceId            = value["sourceId"];
        JsonVariantConst    jsonUpdatePeriod        = value["updatePeriod"];
        JsonVariantConst    jsonApiKey              = value["apiKey"];
        JsonVariantConst    jsonLatitude            = value["latitude"];
        JsonVariantConst    jsonLongitude           = value["longitude"];
        JsonVariantConst    jsonWeatherInfo         = value["weatherInfo"];
        JsonVariantConst    jsonUnits               = value["units"];

        /* The received configuration may not contain all single key/value pair.
         * Therefore read first the complete internal configuration and
         * overwrite them with the received ones.
         */
        getConfiguration(jsonCfg);

        /* Note:
         * Check only for the key/value pair availability.
         * The type check will follow in the setConfiguration().
         */

        if (false == jsonSourceId.isNull())
        {
            jsonCfg["sourceId"] = jsonSourceId.as<uint32_t>();
            isSuccessful = true;
        }

        if (false == jsonUpdatePeriod.isNull())
        {
            jsonCfg["updatePeriod"] = jsonUpdatePeriod.as<uint32_t>();
            isSuccessful = true;
        }

        if (false == jsonApiKey.isNull())
        {
            jsonCfg["apiKey"] = jsonApiKey.as<String>();
            isSuccessful = true;
        }

        if (false == jsonLatitude.isNull())
        {
            jsonCfg["latitude"] = jsonLatitude.as<String>();
            isSuccessful = true;
        }
        
        if (false == jsonLongitude.isNull())
        {
            jsonCfg["longitude"] = jsonLongitude.as<String>();
            isSuccessful = true;
        }

        if (false == jsonWeatherInfo.isNull())
        {
            jsonCfg["weatherInfo"] = jsonWeatherInfo.as<uint32_t>();
            isSuccessful = true;
        }

        if (false == jsonUnits.isNull())
        {
            jsonCfg["units"] = jsonUnits.as<String>();
            isSuccessful = true;
        }

        if (true == isSuccessful)
        {
            JsonObjectConst jsonCfgConst = jsonCfg;

            isSuccessful = setConfiguration(jsonCfgConst);

            if (true == isSuccessful)
            {
                requestStoreToPersistentMemory();
            }
        }
    }

    return isSuccessful;
}

bool OpenWeatherPlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    bool                        hasTopicChanged = m_hasTopicChanged;

    /* Only a single topic, therefore its not necessary to check. */
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void OpenWeatherPlugin::setSlot(const ISlotPlugin* slotInterf)
{
    m_slotInterf = slotInterf;
}

void OpenWeatherPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.init(width, height);

    PluginWithConfig::start(width, height);

    initHttpClient();
}

void OpenWeatherPlugin::stop()
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_requestTimer.stop();

    PluginWithConfig::stop();
}

void OpenWeatherPlugin::active(YAGfx& gfx)
{
    UTIL_NOT_USED(gfx);
    m_view.restartWeatherInfo();
}

void OpenWeatherPlugin::inactive()
{
    /* Nothing to do. */
}

void OpenWeatherPlugin::process(bool isConnected)
{
    Msg                         msg;
    MutexGuard<MutexRecursive>  guard(m_mutex);
    bool                        isRestRequestRequired   = false;

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
        const IOpenWeatherGeneric* source = getWeatherSourceByStatus();

        /* A request without API key makes no sense. */
        if ((nullptr != source) &&
            (false == source->getApiKey().isEmpty()))
        {
            if (false == startHttpRequest(source))
            {
                LOG_WARNING("Failed to request weather info.");
                
                m_requestTimer.start(UPDATE_PERIOD_SHORT);
            }
            else
            {
                weatherRequestStarted();
                m_requestTimer.start(m_updatePeriod);
            }
        }
    }

    if (nullptr != m_slotInterf)
    {
        m_view.setViewDuration(m_slotInterf->getDuration()); 
    }

    if (true == m_taskProxy.receive(msg))
    {
        switch(msg.type)
        {
        case MSG_TYPE_INVALID:
            /* Should never happen. */
            break;

        case MSG_TYPE_RSP:
            /* Any internal error happened? */
            if (nullptr == msg.rsp)
            {
                /* Reset weather request status to avoid to be stucked. */
                m_weatherReqStatus = WEATHER_REQUEST_STATUS_IDLE;
            }
            /* Successful received a response. */
            else
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

void OpenWeatherPlugin::update(YAGfx& gfx)
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

void OpenWeatherPlugin::createOpenWeatherCurrentSource(OpenWeatherSource id)
{
    destroyOpenWeatherCurrentSource();

    switch(id)
    {
    case OPENWEATHER_SOURCE_CURRENT_FORECAST:
        m_sourceCurrent = new(std::nothrow) OpenWeatherCurrent();
        break;

    case OPENWEATHER_SOURCE_ONE_CALL_30:
        m_sourceCurrent = new(std::nothrow) OpenWeatherOneCallCurrent("3.0");
        break;

    default:
        break;
    }
}

void OpenWeatherPlugin::createOpenWeatherForecastSource(OpenWeatherSource id)
{
    destroyOpenWeatherForecastSource();

    switch(id)
    {
    case OPENWEATHER_SOURCE_CURRENT_FORECAST:
        m_sourceForecast = new(std::nothrow) OpenWeatherForecast();
        break;

    case OPENWEATHER_SOURCE_ONE_CALL_30:
        m_sourceForecast = new(std::nothrow) OpenWeatherOneCallForecast("3.0");
        break;

    default:
        break;
    }
}

void OpenWeatherPlugin::destroyOpenWeatherCurrentSource()
{
    if (nullptr != m_sourceCurrent)
    {
        delete m_sourceCurrent;
        m_sourceCurrent = nullptr;
    }
}

void OpenWeatherPlugin::destroyOpenWeatherForecastSource()
{
    if (nullptr != m_sourceForecast)
    {
        delete m_sourceForecast;
        m_sourceForecast = nullptr;
    }
}

void OpenWeatherPlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    /* Get configuration data from current weather source.
     * The forecast weather source has the same configuration.
     */
    if (nullptr == m_sourceCurrent)
    {
        LOG_ERROR("No OpenWeather source available.");
    }
    else
    {
        jsonCfg["sourceId"]     = static_cast<uint32_t>(m_sourceId);
        jsonCfg["updatePeriod"] = m_updatePeriod / (60U * 1000U); /* Conversion from ms to minutes. */
        jsonCfg["apiKey"]       = m_sourceCurrent->getApiKey();
        jsonCfg["latitude"]     = m_sourceCurrent->getLatitude();
        jsonCfg["longitude"]    = m_sourceCurrent->getLongitude();
        jsonCfg["units"]        = m_sourceCurrent->getUnits();
        jsonCfg["weatherInfo"]  = m_view.getWeatherInfo();
    }
}

bool OpenWeatherPlugin::setConfiguration(const JsonObjectConst& jsonCfg)
{
    bool                status              = false;
    JsonVariantConst    jsonSourceId        = jsonCfg["sourceId"];
    JsonVariantConst    jsonUpdatePeriod    = jsonCfg["updatePeriod"];
    JsonVariantConst    jsonApiKey          = jsonCfg["apiKey"];
    JsonVariantConst    jsonLatitude        = jsonCfg["latitude"];
    JsonVariantConst    jsonLongitude       = jsonCfg["longitude"];
    JsonVariantConst    jsonWeatherInfo     = jsonCfg["weatherInfo"];
    JsonVariantConst    jsonUnits           = jsonCfg["units"];

    const uint32_t      UPDATE_PERIOD_LOWER_LIMIT   = 1U;   /* minutes */
    const uint32_t      UPDATE_PERIOD_UPPER_LIMIT   = 120U; /* minutes */

    if (false == jsonSourceId.is<uint32_t>())
    {
        LOG_WARNING("Source id not found or invalid type.");
    }
    else if (false == jsonUpdatePeriod.is<uint32_t>())
    {
        LOG_WARNING("Update period not found or invalid type.");
    }
    else if (false == jsonApiKey.is<String>())
    {
        LOG_WARNING("API key not found or invalid type.");
    }
    else if (false == jsonLatitude.is<String>())
    {
        LOG_WARNING("Latitude not found or invalid type.");
    }
    else if (false == jsonLongitude.is<String>())
    {
        LOG_WARNING("Longitude not found or invalid type.");
    }
    else if (false == jsonWeatherInfo.is<uint32_t>())
    {
        LOG_WARNING("Weather info not found or invalid type.");
    }
    else if (false == jsonUnits.is<String>())
    {
        LOG_WARNING("Units not found or invalid type.");
    }
    else
    {
        MutexGuard<MutexRecursive>  guard(m_mutex);
        OpenWeatherSource           sourceId        = static_cast<OpenWeatherSource>(jsonSourceId.as<uint32_t>());

        if (m_sourceId != sourceId)
        {
            m_sourceId = sourceId;
            createOpenWeatherCurrentSource(m_sourceId);

            if (true == _OpenWeatherPlugin::View::isWeatherForecastSupported())
            {
                createOpenWeatherForecastSource(m_sourceId);
            }
        }

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

        if (nullptr == m_sourceCurrent)
        {
            LOG_ERROR("No OpenWeather current source available.");
        }
        else
        {
            m_sourceCurrent->setApiKey(jsonApiKey.as<String>());
            m_sourceCurrent->setLatitude(jsonLatitude.as<String>());
            m_sourceCurrent->setLongitude(jsonLongitude.as<String>());
            m_sourceCurrent->setUnits(jsonUnits.as<String>());
        }

        if (true == _OpenWeatherPlugin::View::isWeatherForecastSupported())
        {
            if (nullptr == m_sourceForecast)
            {
                LOG_ERROR("No OpenWeather forecast source available.");
            }
            else
            {
                m_sourceForecast->setApiKey(jsonApiKey.as<String>());
                m_sourceForecast->setLatitude(jsonLatitude.as<String>());
                m_sourceForecast->setLongitude(jsonLongitude.as<String>());
                m_sourceForecast->setUnits(jsonUnits.as<String>());
            }
        }

        m_view.setWeatherInfo(jsonWeatherInfo.as<uint32_t>());

        /* Force update on display */
        m_requestTimer.start(UPDATE_PERIOD_SHORT);

        m_hasTopicChanged = true;

        status = true;
    }

    return status;
}

bool OpenWeatherPlugin::startHttpRequest(const IOpenWeatherGeneric* source)
{
    bool status = false;

    if ((nullptr != source) &&
        (false == source->getApiKey().isEmpty()) &&
        (false == source->getLatitude().isEmpty()) &&
        (false == source->getLongitude().isEmpty()) &&
        (false == source->getUnits().isEmpty()))
    {
        String url = OPEN_WEATHER_BASE_URI;

        source->getUrl(url);

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
            handleAsyncWebResponse(rsp);
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

void OpenWeatherPlugin::handleAsyncWebResponse(const HttpResponse& rsp)
{
    if (HttpStatus::STATUS_CODE_OK == rsp.getStatusCode())
    {
        bool                        isSuccessful    = false;
        const IOpenWeatherGeneric*  source          = getWeatherSourceByStatus();

        if (nullptr != source)
        {
            const size_t            JSON_DOC_SIZE   = 2048U;
            DynamicJsonDocument*    jsonDoc         = new(std::nothrow) DynamicJsonDocument(JSON_DOC_SIZE);

            if (nullptr != jsonDoc)
            {
                size_t                          payloadSize     = 0U;
                const void*                     vPayload        = rsp.getPayload(payloadSize);
                const char*                     payload         = static_cast<const char*>(vPayload);
                const size_t                    FILTER_SIZE     = 640U;
                StaticJsonDocument<FILTER_SIZE> jsonFilterDoc;

                source->getFilter(jsonFilterDoc);

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

                        msg.type    = MSG_TYPE_RSP;
                        msg.rsp     = jsonDoc;

                        isSuccessful = this->m_taskProxy.send(msg);
                    }
                }

                if (false == isSuccessful)
                {
                    delete jsonDoc;
                    jsonDoc = nullptr;
                }
            }
        }

        /* If something went wrong, send a response with empty payload to
         * trigger state change in weather request status and not stuck.
         */
        if (false == isSuccessful)
        {
            Msg msg;

            msg.type    = MSG_TYPE_RSP;
            msg.rsp     = nullptr;

            if (false == this->m_taskProxy.send(msg))
            {
                LOG_FATAL("Internal error.");
            }
        }
    }
}

void OpenWeatherPlugin::handleWebResponse(const DynamicJsonDocument& jsonDoc)
{
    IOpenWeatherGeneric* source = getWeatherSourceByStatus();

    if (nullptr != source)
    {
        /* Any error? */
        if (true == jsonDoc.isNull())
        {
            /* Reset weather request status to avoid to stuck. */
            m_weatherReqStatus = WEATHER_REQUEST_STATUS_IDLE;
        }
        /* Response received */
        else
        {
            source->parse(jsonDoc);

            if (WEATHER_REQUEST_STATUS_CURRENT_PENDING == m_weatherReqStatus)
            {
                /* Handle current weather information. */
                if (nullptr != m_sourceCurrent)
                {
                    _OpenWeatherPlugin::View::WeatherInfoCurrent weatherInfo;

                    weatherInfo.humidity    = m_sourceCurrent->getHumidity();
                    weatherInfo.iconId      = m_sourceCurrent->getWeatherIconId();
                    weatherInfo.temperature = m_sourceCurrent->getTemperature();
                    weatherInfo.uvIndex     = m_sourceCurrent->getUvIndex();
                    weatherInfo.windSpeed   = m_sourceCurrent->getWindSpeed();

                    m_view.setWeatherInfoCurrent(weatherInfo);

                    LOG_INFO("Icon id: %s", weatherInfo.iconId.c_str());
                    LOG_INFO("Temperature: %0.2f", weatherInfo.temperature);
                    LOG_INFO("Humidity: %u", weatherInfo.humidity);
                    LOG_INFO("UV-Index: %0.2f", weatherInfo.uvIndex);
                    LOG_INFO("Wind speed: %0.2f", weatherInfo.windSpeed);
                }

                if (true == _OpenWeatherPlugin::View::isWeatherForecastSupported())
                {
                    m_weatherReqStatus = WEATHER_REQUEST_STATUS_FORECAST_REQ;
                    m_requestTimer.stop(); /* Force immediate request in process(). */
                }
                else
                {
                    m_weatherReqStatus = WEATHER_REQUEST_STATUS_IDLE;
                }
            }
            else if (true == _OpenWeatherPlugin::View::isWeatherForecastSupported())
            {
                if (WEATHER_REQUEST_STATUS_FORECAST_PENDING == m_weatherReqStatus)
                {
                    /* Handle forecast weather information. */
                    if (nullptr != m_sourceForecast)
                    {
                        uint8_t                                         day;
                        _OpenWeatherPlugin::View::WeatherInfoForecast   weatherInfo;

                        for(day = 0U; day < _OpenWeatherPlugin::View::FORECAST_DAYS; ++day)
                        {
                            weatherInfo.iconId          = m_sourceForecast->getWeatherIconId(day);
                            weatherInfo.temperatureMax  = m_sourceForecast->getTemperatureMax(day);
                            weatherInfo.temperatureMin  = m_sourceForecast->getTemperatureMin(day);

                            m_view.setWeatherInfoForecast(day, weatherInfo);

                            LOG_INFO("Day: %u", day);
                            LOG_INFO("Icon id: %s", weatherInfo.iconId.c_str());
                            LOG_INFO("Temperature min.: %0.2f", weatherInfo.temperatureMin);
                            LOG_INFO("Temperature max.: %0.2f", weatherInfo.temperatureMax);
                        }
                    }

                    m_weatherReqStatus = WEATHER_REQUEST_STATUS_IDLE;
                }
                else
                {
                    /* Should never happen. */
                    m_weatherReqStatus = WEATHER_REQUEST_STATUS_IDLE;
                }
            }
            else
            {
                /* Should never happen. */
                m_weatherReqStatus = WEATHER_REQUEST_STATUS_IDLE;
            }
        }
    }
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

IOpenWeatherGeneric* OpenWeatherPlugin::getWeatherSourceByStatus()
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    IOpenWeatherGeneric*        source = nullptr;

    switch(m_weatherReqStatus)
    {
    case WEATHER_REQUEST_STATUS_IDLE:
        source = static_cast<IOpenWeatherGeneric*>(m_sourceCurrent);
        break;

    case WEATHER_REQUEST_STATUS_CURRENT_PENDING:
        source = static_cast<IOpenWeatherGeneric*>(m_sourceCurrent);
        break;
        
    case WEATHER_REQUEST_STATUS_FORECAST_REQ:
        source = static_cast<IOpenWeatherGeneric*>(m_sourceForecast);
        break;
        
    case WEATHER_REQUEST_STATUS_FORECAST_PENDING:
        source = static_cast<IOpenWeatherGeneric*>(m_sourceForecast);
        break;
        
    default:
        break;
    }

    return source;
}

void OpenWeatherPlugin::weatherRequestStarted()
{
    if (WEATHER_REQUEST_STATUS_IDLE == m_weatherReqStatus)
    {
        m_weatherReqStatus = WEATHER_REQUEST_STATUS_CURRENT_PENDING;
    }
    else if (WEATHER_REQUEST_STATUS_FORECAST_REQ == m_weatherReqStatus)
    {
        m_weatherReqStatus = WEATHER_REQUEST_STATUS_FORECAST_PENDING;
    }
    else
    {
        /* Should never happen. */
        ;
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

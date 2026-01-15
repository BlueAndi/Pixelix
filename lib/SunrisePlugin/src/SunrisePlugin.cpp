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
 * @file   SunrisePlugin.cpp
 * @brief  Sunrise plugin.
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "SunrisePlugin.h"

#include <time.h>
#include <ClockDrv.h>
#include <ArduinoJson.h>
#include <Logging.h>
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

/* Initialize plugin topic. */
const char* SunrisePlugin::TOPIC_CONFIG        = "location";

/* Initialize time format. */
const char* SunrisePlugin::TIME_FORMAT_DEFAULT = "%I:%M %p";

/* Initialize sunset and sunrise times API base URI.
 * Use http:// instead of https:// for less required heap memory for SSL connection.
 */
const char* SunrisePlugin::BASE_URI            = "http://api.sunrise-sunset.org";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void SunrisePlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC_CONFIG);
}

bool SunrisePlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_CONFIG))
    {
        getConfiguration(value);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool SunrisePlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_CONFIG))
    {
        const size_t        JSON_DOC_SIZE = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg        = jsonDoc.to<JsonObject>();
        JsonVariantConst    jsonLongitude  = value["longitude"];
        JsonVariantConst    jsonLatitude   = value["latitude"];
        JsonVariantConst    jsonTimeFormat = value["timeFormat"];

        /* The received configuration may not contain all single key/value pair.
         * Therefore read first the complete internal configuration and
         * overwrite them with the received ones.
         */
        getConfiguration(jsonCfg);

        /* Note:
         * Check only for the key/value pair availability.
         * The type check will follow in the setConfiguration().
         */

        if (false == jsonLongitude.isNull())
        {
            jsonCfg["longitude"] = jsonLongitude.as<const char*>();
            isSuccessful         = true;
        }

        if (false == jsonLatitude.isNull())
        {
            jsonCfg["latitude"] = jsonLatitude.as<const char*>();
            isSuccessful        = true;
        }

        if (false == jsonTimeFormat.isNull())
        {
            jsonCfg["timeFormat"] = jsonTimeFormat.as<const char*>();
            isSuccessful          = true;
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

bool SunrisePlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive> guard(m_mutex);
    bool                       hasTopicChanged = m_hasTopicChanged;

    /* Only a single topic, therefore its not necessary to check. */
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void SunrisePlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.init(width, height);

    PluginWithConfig::start(width, height);
}

void SunrisePlugin::stop()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_requestTimer.stop();

    PluginWithConfig::stop();

    m_isAllowedToSend = false;

    if (RestService::INVALID_REST_ID != m_dynamicRestId)
    {
        RestService::getInstance().abortRequest(m_dynamicRestId);
        m_dynamicRestId = RestService::INVALID_REST_ID;
    }
}

void SunrisePlugin::process(bool isConnected)
{
    MutexGuard<MutexRecursive> guard(m_mutex);
    DynamicJsonDocument        jsonDoc(0U);
    bool                       isValidResponse;

    PluginWithConfig::process(isConnected);

    /* Only if a network connection is established the required information
     * shall be periodically requested via REST API.
     */
    if (false == m_requestTimer.isTimerRunning())
    {
        if (true == isConnected)
        {
            /* Only one request can be sent at a time. */
            if (true == m_isAllowedToSend)
            {
                if (false == startHttpRequest())
                {
                    m_requestTimer.start(UPDATE_PERIOD_SHORT);
                }
                else
                {
                    m_requestTimer.start(UPDATE_PERIOD);
                    m_isAllowedToSend = false;
                }
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
            /* Only one request can be sent at a time. */
            if (true == m_isAllowedToSend)
            {
                if (false == startHttpRequest())
                {
                    m_requestTimer.start(UPDATE_PERIOD_SHORT);
                }
                else
                {
                    m_requestTimer.start(UPDATE_PERIOD);
                    m_isAllowedToSend = false;
                }
            }
        }
    }

    if (RestService::INVALID_REST_ID != m_dynamicRestId)
    {
        /* Get the response from the REST service. */
        if (true == RestService::getInstance().getResponse(m_dynamicRestId, isValidResponse, jsonDoc))
        {
            if (true == isValidResponse)
            {
                handleWebResponse(jsonDoc);
            }
            else
            {
                LOG_WARNING("Connection error.");
                m_requestTimer.start(UPDATE_PERIOD_SHORT);
            }

            m_dynamicRestId   = RestService::INVALID_REST_ID;
            m_isAllowedToSend = true;
        }
    }
}

void SunrisePlugin::update(YAGfx& gfx)
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

void SunrisePlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["longitude"]  = m_longitude;
    jsonCfg["latitude"]   = m_latitude;
    jsonCfg["timeFormat"] = m_timeFormat;
}

bool SunrisePlugin::setConfiguration(const JsonObjectConst& jsonCfg)
{
    bool             status         = false;
    JsonVariantConst jsonLon        = jsonCfg["longitude"];
    JsonVariantConst jsonLat        = jsonCfg["latitude"];
    JsonVariantConst jsonTimeFormat = jsonCfg["timeFormat"];

    if (false == jsonLon.is<String>())
    {
        LOG_WARNING("longitude not found or invalid type.");
    }
    else if (false == jsonLat.is<String>())
    {
        LOG_WARNING("latitude not found or invalid type.");
    }
    else if (false == jsonTimeFormat.is<String>())
    {
        LOG_WARNING("JSON time format not found or invalid type.");
    }
    else
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        m_longitude  = jsonLon.as<const char*>();
        m_latitude   = jsonLat.as<const char*>();
        m_timeFormat = jsonTimeFormat.as<const char*>();

        /* Force update on display */
        m_requestTimer.start(UPDATE_PERIOD_SHORT);

        m_hasTopicChanged = true;

        status            = true;
    }

    return status;
}

bool SunrisePlugin::startHttpRequest()
{
    bool                            status = false;
    RestService::PreProcessCallback preProcessCallback =
        [this](const char* payload, size_t size, DynamicJsonDocument& doc) {
            return this->preProcessAsyncWebResponse(payload, size, doc);
        };

    if ((false == m_latitude.isEmpty()) &&
        (false == m_longitude.isEmpty()))
    {
        String url      = String(BASE_URI) + "/json?lat=" + m_latitude + "&lng=" + m_longitude + "&formatted=0";

        m_dynamicRestId = RestService::getInstance().get(url, preProcessCallback);

        if (RestService::INVALID_REST_ID == m_dynamicRestId)
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

bool SunrisePlugin::preProcessAsyncWebResponse(const char* payload, size_t payloadSize, DynamicJsonDocument& jsonDoc)
{
    bool                            isSuccessful = false;
    const size_t                    FILTER_SIZE  = 128U;
    StaticJsonDocument<FILTER_SIZE> jsonFilterDoc;

    /* Example:
     * {
     *   "results":
     *   {
     *     "sunrise":"2015-05-21T05:05:35+00:00",
     *     "sunset":"2015-05-21T19:22:59+00:00",
     *     "solar_noon":"2015-05-21T12:14:17+00:00",
     *     "day_length":51444,
     *     "civil_twilight_begin":"2015-05-21T04:36:17+00:00",
     *     "civil_twilight_end":"2015-05-21T19:52:17+00:00",
     *     "nautical_twilight_begin":"2015-05-21T04:00:13+00:00",
     *     "nautical_twilight_end":"2015-05-21T20:28:21+00:00",
     *     "astronomical_twilight_begin":"2015-05-21T03:20:49+00:00",
     *     "astronomical_twilight_end":"2015-05-21T21:07:45+00:00"
     *   },
     *    "status":"OK"
     * }
     */

    jsonFilterDoc["results"]["sunrise"] = true;
    jsonFilterDoc["results"]["sunset"]  = true;

    if (true == jsonFilterDoc.overflowed())
    {
        LOG_ERROR("JSON document size exceeded.");
    }
    else
    {
        DeserializationError error = deserializeJson(jsonDoc, payload, payloadSize, DeserializationOption::Filter(jsonFilterDoc));

        if (DeserializationError::Ok != error.code())
        {
            LOG_ERROR("Invalid JSON message received: %s", error.c_str());
        }
        else
        {
            isSuccessful = true;
        }
    }

    return isSuccessful;
}

void SunrisePlugin::handleWebResponse(const DynamicJsonDocument& jsonDoc)
{
    JsonVariantConst jsonResults = jsonDoc["results"];
    JsonVariantConst jsonSunrise = jsonResults["sunrise"];
    JsonVariantConst jsonSunset  = jsonResults["sunset"];

    if ((false == jsonSunrise.is<String>()))
    {
        LOG_WARNING("JSON sunrise type mismatch or missing.");
    }
    else if ((false == jsonSunset.is<String>()))
    {
        LOG_WARNING("JSON sunset type mismatch or missing.");
    }
    else
    {
        const char* sunrise     = jsonSunrise.as<const char*>();
        const char* sunset      = jsonSunset.as<const char*>();

        m_relevantResponsePart  = addCurrentTimeZoneValues(sunrise);
        m_relevantResponsePart += " / ";
        m_relevantResponsePart += addCurrentTimeZoneValues(sunset);

        m_view.setFormatText(m_relevantResponsePart);
    }
}

String SunrisePlugin::addCurrentTimeZoneValues(const char* dateTimeString) const
{
    char timeBuffer[17] = { 0 };

    if (nullptr != dateTimeString)
    {
        tm        utcTimeInfo;
        const tm* lcTimeInfo = nullptr;
        time_t    tLocal;

        /* Example: "2015-05-21T05:05:35+00:00" */

        /* Convert date/time string to GMT time information */
        (void)strptime(dateTimeString, "%Y-%m-%dT%H:%M:%S", &utcTimeInfo);

        /* Convert to local time */
        utcTimeInfo.tm_isdst  = 0; /* Not daylight saving time. */
        tLocal                = mktime(&utcTimeInfo);
        tLocal               += ClockDrv::getInstance().getCurrentTimeZoneOffset();
        lcTimeInfo            = localtime(&tLocal);

        /* Convert time information to user friendly string. */
        (void)strftime(timeBuffer, sizeof(timeBuffer), m_timeFormat.c_str(), lcTimeInfo);
    }

    return timeBuffer;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

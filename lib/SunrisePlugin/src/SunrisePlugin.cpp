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
 * @brief  Sunrise plugin.
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "AsyncHttpClient.h"
#include "ClockDrv.h"
#include "SunrisePlugin.h"
#include "time.h"

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

/* Initialize image path. */
const char* SunrisePlugin::IMAGE_PATH           = "/plugins/SunrisePlugin/sunrise.bmp";

/* Initialize plugin topic. */
const char* SunrisePlugin::TOPIC_CONFIG         = "/location";

/* Initialize time format. */
const char* SunrisePlugin::TIME_FORMAT_DEFAULT  = "%I:%M %p";

/* Initialize sunset and sunrise times API base URI.
 * Use http:// instead of https:// for less required heap memory for SSL connection.
 */
const char* SunrisePlugin::BASE_URI             = "http://api.sunrise-sunset.org";

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

    if (0U != topic.equals(TOPIC_CONFIG))
    {
        getConfiguration(value);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool SunrisePlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CONFIG))
    {
        const size_t        JSON_DOC_SIZE           = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg                 = jsonDoc.to<JsonObject>();
        JsonVariantConst    jsonLongitude           = value["longitude"];
        JsonVariantConst    jsonLatitude            = value["latitude"];
        JsonVariantConst    jsonTimeFormat          = value["timeFormat"];

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
            jsonCfg["longitude"] = jsonLongitude.as<String>();
            isSuccessful = true;
        }

        if (false == jsonLatitude.isNull())
        {
            jsonCfg["latitude"] = jsonLatitude.as<String>();
            isSuccessful = true;
        }

        if (false == jsonTimeFormat.isNull())
        {
            jsonCfg["timeFormat"] = jsonTimeFormat.as<String>();
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

bool SunrisePlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    bool                        hasTopicChanged = m_hasTopicChanged;

    /* Only a single topic, therefore its not necessary to check. */
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void SunrisePlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_iconCanvas.setPosAndSize(0, 0, ICON_WIDTH, ICON_HEIGHT);
    (void)m_iconCanvas.addWidget(m_bitmapWidget);

    (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH);

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
    else
    {
        /* Remember current timestamp to detect updates of the configuration in the
         * filesystem without using the plugin API.
         */
        updateTimestampLastUpdate();
    }

    m_cfgReloadTimer.start(CFG_RELOAD_PERIOD);

    initHttpClient();
}

void SunrisePlugin::stop()
{
    String                      configurationFilename = getFullPathToConfiguration();
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_cfgReloadTimer.stop();
    m_requestTimer.stop();

    if (false != FILESYSTEM.remove(configurationFilename))
    {
        LOG_INFO("File %s removed", configurationFilename.c_str());
    }
}

void SunrisePlugin::process(bool isConnected)
{
    Msg                         msg;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    /* Configuration in persistent memory updated? */
    if ((true == m_cfgReloadTimer.isTimerRunning()) &&
        (true == m_cfgReloadTimer.isTimeout()))
    {
        if (true == isConfigurationUpdated())
        {
            m_reloadConfigReq = true;
        }

        m_cfgReloadTimer.restart();
    }

    if (true == m_storeConfigReq)
    {
        if (false == saveConfiguration())
        {
            LOG_WARNING("Failed to save configuration: %s", getFullPathToConfiguration().c_str());
        }

        m_storeConfigReq = false;
    }
    else if (true == m_reloadConfigReq)
    {
        LOG_INFO("Reload configuration: %s", getFullPathToConfiguration().c_str());

        if (true == loadConfiguration())
        {
            updateTimestampLastUpdate();
        }

        m_reloadConfigReq = false;
    }
    else
    {
        ;
    }

    /* Only if a network connection is established the required information
     * shall be periodically requested via REST API.
     */
    if (false == m_requestTimer.isTimerRunning())
    {
        if (true == isConnected)
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
                m_requestTimer.start(UPDATE_PERIOD_SHORT);
            }
            else
            {
                m_requestTimer.start(UPDATE_PERIOD);
            }
        }
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

        default:
            /* Should never happen. */
            break;
        }
    }
}

void SunrisePlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    gfx.fillScreen(ColorDef::BLACK);
    m_iconCanvas.update(gfx);
    m_textCanvas.update(gfx);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void SunrisePlugin::requestStoreToPersistentMemory()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_storeConfigReq = true;
}

void SunrisePlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["longitude"]    = m_longitude;
    jsonCfg["latitude"]     = m_latitude;
    jsonCfg["timeFormat"]   = m_timeFormat;
}

bool SunrisePlugin::setConfiguration(JsonObjectConst& jsonCfg)
{
    bool                status          = false;
    JsonVariantConst    jsonLon         = jsonCfg["longitude"];
    JsonVariantConst    jsonLat         = jsonCfg["latitude"];
    JsonVariantConst    jsonTimeFormat  = jsonCfg["timeFormat"];

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

        m_longitude     = jsonLon.as<String>();
        m_latitude      = jsonLat.as<String>();
        m_timeFormat    = jsonTimeFormat.as<String>();

        /* Force update on display */
        m_requestTimer.start(UPDATE_PERIOD_SHORT);

        m_hasTopicChanged = true;

        status = true;
    }

    return status;
}

bool SunrisePlugin::startHttpRequest()
{
    bool status = false;

    if ((false == m_latitude.isEmpty()) &&
        (false == m_longitude.isEmpty()))
    {
        String url = String(BASE_URI) + "/json?lat=" + m_latitude + "&lng=" + m_longitude + "&formatted=0";

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

void SunrisePlugin::initHttpClient()
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
}

void SunrisePlugin::handleAsyncWebResponse(const HttpResponse& rsp)
{
    if (HttpStatus::STATUS_CODE_OK == rsp.getStatusCode())
    {
        const size_t            JSON_DOC_SIZE   = 512U;
        DynamicJsonDocument*    jsonDoc         = new(std::nothrow) DynamicJsonDocument(JSON_DOC_SIZE);

        if (nullptr != jsonDoc)
        {
            size_t                          payloadSize = 0U;
            const void*                     vPayload    = rsp.getPayload(payloadSize);
            const char*                     payload     = static_cast<const char*>(vPayload);
            const size_t                    FILTER_SIZE = 128U;
            StaticJsonDocument<FILTER_SIZE> filter;

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

            filter["results"]["sunrise"]    = true;
            filter["results"]["sunset"]     = true;

            if (true == filter.overflowed())
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
                DeserializationError error = deserializeJson(*jsonDoc, payload, payloadSize, DeserializationOption::Filter(filter));

                if (DeserializationError::Ok != error.code())
                {
                    LOG_ERROR("Invalid JSON message received: %s", error.c_str());
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
    }
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
        String sunrise  = jsonSunrise.as<String>();
        String sunset   = jsonSunset.as<String>();
        
        sunrise = addCurrentTimezoneValues(sunrise);
        sunset  = addCurrentTimezoneValues(sunset);

        m_relevantResponsePart = sunrise + " / " + sunset;
        m_textWidget.setFormatStr(m_relevantResponsePart);
    }
}

String SunrisePlugin::addCurrentTimezoneValues(const String& dateTimeString) const
{
    tm      gmTimeInfo;
    tm*     lcTimeInfo          = nullptr;
    time_t  gmTime;
    char    timeBuffer[17]      = { 0 };

    /* Example: "2015-05-21T05:05:35+00:00" */

    /* Convert date/time string to GMT time information */
    (void)strptime(dateTimeString.c_str(), "%Y-%m-%dT%H:%M:%S", &gmTimeInfo);

    /* Convert to local time */
    gmTime = mktime(&gmTimeInfo);
    lcTimeInfo = localtime(&gmTime);

    /* Convert time information to user friendly string. */
    (void)strftime(timeBuffer, sizeof(timeBuffer), m_timeFormat.c_str(), lcTimeInfo);

    return timeBuffer;
}

void SunrisePlugin::clearQueue()
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

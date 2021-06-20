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
const char* SunrisePlugin::IMAGE_PATH   = "/images/sunrise.bmp";

/* Initialize plugin topic. */
const char* SunrisePlugin::TOPIC        = "/location";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void SunrisePlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC);
}

bool SunrisePlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC))
    {
        String  longitude;
        String  latitude;

        getLocation(longitude, latitude);

        value["longitude"]  = longitude;
        value["latitude"]   = latitude;

        isSuccessful = true;
    }

    return isSuccessful;
}

bool SunrisePlugin::setTopic(const String& topic, const JsonObject& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC))
    {
        String  longitude;
        String  latitude;

        getLocation(longitude, latitude);

        if (false == value["longitude"].isNull())
        {
            longitude = value["longitude"].as<String>();
            isSuccessful = true;
        }

        if (false == value["latitude"].isNull())
        {
            latitude = value["latitude"].as<String>();
            isSuccessful = true;
        }

        if (true == isSuccessful)
        {
            setLocation(longitude, latitude);
        }
    }

    return isSuccessful;
}

void SunrisePlugin::start(uint16_t width, uint16_t height)
{
    lock();

    if (nullptr == m_iconCanvas)
    {
        m_iconCanvas = new Canvas(ICON_WIDTH, ICON_HEIGHT, 0, 0);

        if (nullptr != m_iconCanvas)
        {
            (void)m_iconCanvas->addWidget(m_bitmapWidget);

            /* Load  icon from filesystem. */
            (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH);
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
        m_requestTimer.start(UPDATE_PERIOD_SHORT);
    }
    else
    {
        m_requestTimer.start(UPDATE_PERIOD);
    }

    unlock();

    return;
}

void SunrisePlugin::stop()
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

void SunrisePlugin::process()
{
    Msg msg;
    
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

    unlock();

    return;
}

void SunrisePlugin::update(YAGfx& gfx)
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

void SunrisePlugin::getLocation(String& longitude, String&latitude) const
{
    lock();

    longitude   = m_longitude;
    latitude    = m_latitude;

    unlock();

    return;
}

void SunrisePlugin::setLocation(const String& longitude, const String& latitude)
{
    lock();

    if ((longitude != m_longitude) ||
        (latitude != m_latitude))
    {
        m_longitude = longitude;
        m_latitude  = latitude;

        /* Always stores the configuration, otherwise it will be overwritten during
         * plugin activation.
         */
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

bool SunrisePlugin::startHttpRequest()
{
    bool    status  = false;
    String  url     = String("http://api.sunrise-sunset.org/json?lat=") + m_latitude + "&lng=" + m_longitude + "&formatted=0";

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

void SunrisePlugin::initHttpClient()
{
    /* Note: All registered callbacks are running in a different task context!
     *       Therefore it is not allowed to access a member here directly.
     *       The processing must be deferred via task proxy.
     */
    m_client.regOnResponse(
        [this](const HttpResponse& rsp)
        {

            const size_t            JSON_DOC_SIZE   = 512U;
            DynamicJsonDocument*    jsonDoc         = new DynamicJsonDocument(JSON_DOC_SIZE);

            if (nullptr != jsonDoc)
            {
                size_t                          payloadSize = 0U;
                const char*                     payload     = reinterpret_cast<const char*>(rsp.getPayload(payloadSize));
                const size_t                    FILTER_SIZE = 128U;
                StaticJsonDocument<FILTER_SIZE> filter;
                DeserializationError            error;

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

                error = deserializeJson(*jsonDoc, payload, payloadSize, DeserializationOption::Filter(filter));

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
    );
}

void SunrisePlugin::handleWebResponse(DynamicJsonDocument& jsonDoc)
{
    String      sunrise;
    String      sunset;
    JsonObject  results;
    
    results = jsonDoc["results"];
    sunrise = results["sunrise"].as<String>();
    sunset  = results["sunset"].as<String>();

    sunrise = addCurrentTimezoneValues(sunrise);
    sunset  = addCurrentTimezoneValues(sunset);

    m_relevantResponsePart = sunrise + " / " + sunset;
    m_textWidget.setFormatStr(m_relevantResponsePart);
}

String SunrisePlugin::addCurrentTimezoneValues(const String& dateTimeString) const
{
    tm          gmTimeInfo;
    tm*         lcTimeInfo          = nullptr;
    time_t      gmTime;
    char        timeBuffer[17]      = { 0 };
    const char* formattedTimeString = ClockDrv::getInstance().getTimeFormat() ? "%H:%M" : "%I:%M %p";

    /* Example: "2015-05-21T05:05:35+00:00" */

    /* Convert date/time string to GMT time information */
    (void)strptime(dateTimeString.c_str(), "%Y-%m-%dT%H:%M:%S", &gmTimeInfo);

    /* Convert to local time */
    gmTime = mktime(&gmTimeInfo);
    lcTimeInfo = localtime(&gmTime);

    /* Convert time information to user friendly string. */
    (void)strftime(timeBuffer, sizeof(timeBuffer), formattedTimeString, lcTimeInfo);

    return timeBuffer;
}

bool SunrisePlugin::saveConfiguration() const
{
    bool                status                  = true;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    String              configurationFilename   = getFullPathToConfiguration();

    jsonDoc["longitude"]    = m_longitude;
    jsonDoc["latitude"]     = m_latitude;
    
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

bool SunrisePlugin::loadConfiguration()
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
        m_longitude = jsonDoc["longitude"].as<String>();
        m_latitude  = jsonDoc["latitude"].as<String>();
    }

    return status;
}

void SunrisePlugin::lock() const
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreTakeRecursive(m_xMutex, portMAX_DELAY);
    }

    return;
}

void SunrisePlugin::unlock() const
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreGiveRecursive(m_xMutex);
    }

    return;
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

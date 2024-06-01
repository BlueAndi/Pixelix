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
 * @brief  DateTime plugin
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "DateTimePlugin.h"
#include "ClockDrv.h"

#include <Logging.h>
#include <Util.h>

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
const char* DateTimePlugin::TOPIC_CONFIG        = "/dateTime";

/* Initialize default time format. */
const char* DateTimePlugin::TIME_FORMAT_DEFAULT = "%I:%M %p";

/* Initialize default date format. */
const char* DateTimePlugin::DATE_FORMAT_DEFAULT = "%m/%d";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void DateTimePlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC_CONFIG);
}

bool DateTimePlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CONFIG))
    {
        getConfiguration(value);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool DateTimePlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CONFIG))
    {
        const size_t        JSON_DOC_SIZE           = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg                 = jsonDoc.to<JsonObject>();
        JsonVariantConst    jsonMode                = value["mode"];
        JsonVariantConst    jsonTimeFormat          = value["timeFormat"];
        JsonVariantConst    jsonDateFormat          = value["dateFormat"];
        JsonVariantConst    jsonTimeZone            = value["timeZone"];
        JsonVariantConst    jsonDayOnColor          = value["dayOnColor"];
        JsonVariantConst    jsonDayOffColor         = value["dayOffColor"];

        /* The received configuration may not contain all single key/value pair.
         * Therefore read first the complete internal configuration and
         * overwrite them with the received ones.
         */
        getConfiguration(jsonCfg);

        /* Note:
         * Check only for the key/value pair availability.
         * The type check will follow in the setConfiguration().
         */

        if (false == jsonMode.isNull())
        {
            jsonCfg["mode"] = jsonMode.as<uint8_t>();
            isSuccessful = true;
        }
        
        if (false == jsonTimeFormat.isNull())
        {
            jsonCfg["timeFormat"] = jsonTimeFormat.as<String>();
            isSuccessful = true;
        }

        if (false == jsonDateFormat.isNull())
        {
            jsonCfg["dateFormat"] = jsonDateFormat.as<String>();
            isSuccessful = true;
        }

        if (false == jsonTimeZone.isNull())
        {
            jsonCfg["timeZone"] = jsonTimeZone.as<String>();
            isSuccessful = true;
        }
        
        if (false == jsonDayOnColor.isNull())
        {
            jsonCfg["dayOnColor"] = jsonDayOnColor.as<String>();
            isSuccessful = true;
        }

        if (false == jsonDayOffColor.isNull())
        {
            jsonCfg["dayOffColor"] = jsonDayOffColor.as<String>();
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

bool DateTimePlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    bool                        hasTopicChanged = m_hasTopicChanged;

    /* Only a single topic, therefore its not necessary to check. */
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void DateTimePlugin::setSlot(const ISlotPlugin* slotInterf)
{
    m_slotInterf = slotInterf;
}

void DateTimePlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_view.init(width, height);

    PluginWithConfig::start(width, height);
}

void DateTimePlugin::stop()
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    PluginWithConfig::stop();
}

void DateTimePlugin::process(bool isConnected)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    PluginWithConfig::process(isConnected);

    /* The date/time information shall be retrieved every second while plugin is activated. */
    if ((true == m_checkUpdateTimer.isTimerRunning()) &&
        (true == m_checkUpdateTimer.isTimeout()))
    {
        ++m_durationCounter;
        updateDateTime(false);

        m_checkUpdateTimer.restart();
    }
}

void DateTimePlugin::active(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    /* The date/time information shall be retrieved every second while plugin is activated. */
    m_durationCounter = 0U;
    m_checkUpdateTimer.start(CHECK_UPDATE_PERIOD);

    /* The date/time shall be updated on the display right after plugin activation. */
    updateDateTime(true);
}

void DateTimePlugin::inactive()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_checkUpdateTimer.stop();
}

void DateTimePlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (false != m_isUpdateAvailable)
    {
        m_view.update(gfx);

        m_isUpdateAvailable = false;
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void DateTimePlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["mode"]         = m_mode;
    jsonCfg["timeFormat"]   = m_timeFormat;
    jsonCfg["dateFormat"]   = m_dateFormat;
    jsonCfg["timeZone"]     = m_timeZone;
    jsonCfg["dayOnColor"]   = colorToHtml(m_view.getDayOnColor());
    jsonCfg["dayOffColor"]  = colorToHtml(m_view.getDayOffColor());
}

bool DateTimePlugin::setConfiguration(JsonObjectConst& jsonCfg)
{
    bool             status             = false;
    JsonVariantConst jsonMode           = jsonCfg["mode"];
    JsonVariantConst jsonTimeFormat     = jsonCfg["timeFormat"];
    JsonVariantConst jsonDateFormat     = jsonCfg["dateFormat"];
    JsonVariantConst jsonTimeZone       = jsonCfg["timeZone"];
    JsonVariantConst jsonDayOnColor     = jsonCfg["dayOnColor"];
    JsonVariantConst jsonDayOffColor    = jsonCfg["dayOffColor"];

    if ((false == jsonMode.is<uint8_t>()) &&
        (MODE_MAX <= jsonMode.as<uint8_t>()))
    {
        LOG_WARNING("JSON mode not found or invalid type.");
    }
    else if (false == jsonTimeFormat.is<String>())
    {
        LOG_WARNING("JSON time format not found or invalid type.");
    }
    else if (false == jsonDateFormat.is<String>())
    {
        LOG_WARNING("JSON date format not found or invalid type.");
    }
    else if (false == jsonTimeZone.is<String>())
    {
        LOG_WARNING("JSON timezone not found or invalid type.");
    }
    else if (false == jsonDayOnColor.is<String>())
    {
        LOG_WARNING("JSON day on color not found or invalid type.");
    }
    else if (false == jsonDayOffColor.is<String>())
    {
        LOG_WARNING("JSON day off color not found or invalid type.");
    }
    else
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        m_mode          = static_cast<Mode>(jsonMode.as<uint8_t>());
        m_timeFormat    = jsonTimeFormat.as<String>();
        m_dateFormat    = jsonDateFormat.as<String>();
        m_timeZone      = jsonTimeZone.as<String>();

        m_view.setDayOnColor(colorFromHtml(jsonDayOnColor.as<String>()));
        m_view.setDayOffColor(colorFromHtml(jsonDayOffColor.as<String>()));

        m_hasTopicChanged = true;

        status = true;
    }

    return status;
}

void DateTimePlugin::updateDateTime(bool force)
{
    ClockDrv&   clockDrv            = ClockDrv::getInstance();
    struct tm   timeInfo            = { 0 };
    bool        isClockAvailable    = false;

    /* If not other timezone is given, the local time shall be used. */
    if (true == m_timeZone.isEmpty())
    {
        isClockAvailable = clockDrv.getTime(timeInfo);
    }
    else
    {
        isClockAvailable = clockDrv.getTzTime(m_timeZone.c_str(), timeInfo);
    }
    
    if (true == isClockAvailable)
    {
        bool    showDate    = false;
        bool    showTime    = false;

        /* Decide what to show. */
        switch(m_mode)
        {
        case MODE_DATE_TIME:
            {
                uint32_t    duration            = (nullptr == m_slotInterf) ? 0U : m_slotInterf->getDuration();
                uint8_t     halfDurationTicks   = 0U;
                uint8_t     fullDurationTicks   = 0U;

                /* If infinite duration was set, switch between time and date with a fix period. */
                if (0U == duration)
                {
                    duration = DURATION_DEFAULT;
                }

                halfDurationTicks   = (duration / (2U * MS_TO_SEC_DIVIDER));
                fullDurationTicks   = 2U * halfDurationTicks;

                /* The time shall be shown in the first half slot duration. */
                if ((halfDurationTicks >= m_durationCounter) ||
                    (fullDurationTicks < m_durationCounter))
                {
                    showTime = true;
                }
                else
                {
                    showDate = true;
                }

                /* Reset duration counter after a complete plugin slot duration is finished. */
                if (fullDurationTicks < m_durationCounter)
                {
                    m_durationCounter = 0U;
                }

                /* Force the update in case it changes from time to date or vice versa.
                 * This must be done, because we can not rely on the comparison whether
                 * the date/time changed and a update is necessary anyway.
                 */
                if ((0U == m_durationCounter) ||
                    ((halfDurationTicks + 1U) == m_durationCounter))
                {
                    force = true;
                }
            }
            break;

        case MODE_DATE_ONLY:
            showDate = true;
            break;
        
        case MODE_TIME_ONLY:
            showTime = true;
            break;
        
        default:
            /* Should never happen. */
            m_mode = MODE_DATE_TIME;
            break;
        };

        if (true == showTime)
        {
            /* Show the time only in case
             * its forced to do it or
             * the time changed.
             */
            if ((true == force) ||
                (m_shownSecond != timeInfo.tm_sec))
            {
                String  extTimeFormat   = "\\calign" + m_timeFormat;
                String  timeAsStr;
                
                if (true == getTimeAsString(timeAsStr, extTimeFormat, &timeInfo))
                {
                    m_view.setFormatText(timeAsStr);

                    m_shownSecond       = timeInfo.tm_sec;
                    m_isUpdateAvailable = true;
                }
                
                m_view.setWeekdayIndicator(timeInfo);
            }
        }
        else if (true == showDate)
        {
            /* Show the date only in case
             * its forced to do it or
             * the day changed.
             */
            if ((true == force) ||
                (m_shownDayOfTheYear != timeInfo.tm_yday))
            {
                String  extDateFormat   = "\\calign" + m_dateFormat;
                String  dateAsStr;
                
                if (true == getTimeAsString(dateAsStr, extDateFormat, &timeInfo))
                {
                    m_view.setFormatText(dateAsStr);

                    m_shownDayOfTheYear = timeInfo.tm_yday;
                    m_isUpdateAvailable = true;
                }
                
                m_view.setWeekdayIndicator(timeInfo);
            }
        }
        else
        {
            /* Nothing to update. */
            ;
        }
    }
    else
    {
        if(true == force)
        {
            m_view.setFormatText("\\calign?");
            m_isUpdateAvailable = true;
        }
    }
}

bool DateTimePlugin::getTimeAsString(String& time, const String& format, const tm *currentTime)
{
    bool        isSuccessful    = false;
    tm          timeStruct;
    const tm*   timeStructPtr   = nullptr;

    if (nullptr == currentTime)
    {
        timeStructPtr = &timeStruct;

        if (false == ClockDrv::getInstance().getTime(timeStruct))
        {
            timeStructPtr = nullptr;
        }
    }
    else
    {
        timeStructPtr = currentTime;
    }

    if (nullptr != timeStructPtr)
    {
        const uint32_t  MAX_TIME_BUFFER_SIZE = 128U;
        char            buffer[MAX_TIME_BUFFER_SIZE];

        if (0U != strftime(buffer, sizeof(buffer), format.c_str(), currentTime))
        {
            time = buffer;
            isSuccessful = true;
        }
    }

    return isSuccessful;
}

String DateTimePlugin::colorToHtml(const Color& color) const
{
    char buffer[8]; /* '#' + 3x byte in hex + '\0' */

    (void)snprintf(buffer, sizeof(buffer), "#%02X%02X%02X", color.getRed(), color.getGreen(), color.getBlue());

    return String(buffer);
}

Color DateTimePlugin::colorFromHtml(const String& htmlColor) const
{
    Color color;

    if ('#' == htmlColor[0])
    {
        color = Util::hexToUInt32(htmlColor.substring(1U));
    }

    return color;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

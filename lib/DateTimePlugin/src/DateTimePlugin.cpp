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
const char DateTimePlugin::TOPIC_CONFIG[]        = "/dateTime";

/* Initialize default time format. */
const char DateTimePlugin::TIME_FORMAT_DEFAULT[] = "%I:%M %p";

/* Initialize default date format. */
const char DateTimePlugin::DATE_FORMAT_DEFAULT[] = "%m/%d";

/* Color key names for the analog clock configuration. */
const char* DateTimePlugin::ANALOG_CLOCK_COLOR_KEYS[IDateTimeView::ANA_CLK_COL_MAX] =
{
    "handHourCol",
    "handMinCol",
    "handSecCol",
    "ringFiveMinCol",
    "ringMinDotCol"
};

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

    if (true == topic.equals(TOPIC_CONFIG))
    {
        getConfiguration(value);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool DateTimePlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_CONFIG))
    {
        const IDateTimeView::AnalogClockConfig* analogClockCfg = nullptr;

        const size_t        JSON_DOC_SIZE           = 768U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg                 = jsonDoc.to<JsonObject>();
        JsonVariantConst    jsonMode                = value["mode"];
        JsonVariantConst    jsonViewMode            = value["viewMode"];
        JsonVariantConst    jsonTimeFormat          = value["timeFormat"];
        JsonVariantConst    jsonDateFormat          = value["dateFormat"];
        JsonVariantConst    jsonTimeZone            = value["timeZone"];
        JsonVariantConst    jsonStartOfWeek         = value["startOfWeek"];
        JsonVariantConst    jsonDayOnColor          = value["dayOnColor"];
        JsonVariantConst    jsonDayOffColor         = value["dayOffColor"];
        JsonObjectConst     jsonAnalogClock         = value["analogClock"];
       
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

        if (false == jsonViewMode.isNull())
        {
            jsonCfg["viewMode"] = jsonViewMode.as<uint8_t>();
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

        if (false == jsonStartOfWeek.isNull())
        {
            jsonCfg["startOfWeek"] = jsonStartOfWeek.as<uint8_t>();
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
        
        if (false == jsonAnalogClock.isNull())
        {
            jsonCfg["analogClock"] = jsonAnalogClock;
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
    
    m_view.update(gfx);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void DateTimePlugin::getConfiguration(JsonObject& jsonCfg) const
{
    const IDateTimeView::AnalogClockConfig* analogClockCfg = nullptr;

    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["mode"]         = m_mode;
    jsonCfg["viewMode"]     = m_view.getViewMode();
    jsonCfg["timeFormat"]   = m_timeFormat;
    jsonCfg["dateFormat"]   = m_dateFormat;
    jsonCfg["timeZone"]     = m_timeZone;
    jsonCfg["startOfWeek"]  = m_view.getStartOfWeek();
    jsonCfg["dayOnColor"]   = colorToHtml(m_view.getDayOnColor());
    jsonCfg["dayOffColor"]  = colorToHtml(m_view.getDayOffColor());

    if (nullptr != (analogClockCfg = m_view.getAnalogClockConfig()))
    {
        /* View supports analog clock, add the additinal config elements for it.
         */
        JsonObject jsonAnalogClock = jsonCfg.createNestedObject("analogClock");
        jsonAnalogClock["secondsMode"] = analogClockCfg->m_secondsMode;
        for (uint32_t index = 0U;  index < IDateTimeView::ANA_CLK_COL_MAX; ++index)
        {
            jsonAnalogClock[ANALOG_CLOCK_COLOR_KEYS[index]]= colorToHtml(analogClockCfg->m_colors[index]);
        }
    }

}

bool DateTimePlugin::setConfiguration(const JsonObjectConst& jsonCfg)
{
    bool             status             = false;
    JsonVariantConst jsonMode           = jsonCfg["mode"];
    JsonVariantConst jsonViewMode       = jsonCfg["viewMode"];
    JsonVariantConst jsonTimeFormat     = jsonCfg["timeFormat"];
    JsonVariantConst jsonDateFormat     = jsonCfg["dateFormat"];
    JsonVariantConst jsonTimeZone       = jsonCfg["timeZone"];
    JsonVariantConst jsonStartOfWeek    = jsonCfg["startOfWeek"];
    JsonVariantConst jsonDayOnColor     = jsonCfg["dayOnColor"];
    JsonVariantConst jsonDayOffColor    = jsonCfg["dayOffColor"];
    JsonVariantConst jsonAnalogClock    = jsonCfg["analogClock"];

    IDateTimeView::AnalogClockConfig analogClockConfig;

    if ((false == jsonMode.is<uint8_t>()) &&
        (MODE_MAX <= jsonMode.as<uint8_t>()))
    {
        LOG_WARNING("JSON mode not found or invalid type.");
    }
    else if ((false == jsonViewMode.is<uint8_t>()) &&
        (IDateTimeView::VIEW_MODE_MAX <= jsonViewMode.as<uint8_t>()))
    {
        LOG_WARNING("JSON view mode not found or invalid type.");
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
    else if (false == jsonStartOfWeek.is<uint8_t>())
    {
        LOG_WARNING("JSON start of week not found or invalid type.");
    }
    else if (false == jsonDayOnColor.is<String>())
    {
        LOG_WARNING("JSON day on color not found or invalid type.");
    }
    else if (false == jsonDayOffColor.is<String>())
    {
        LOG_WARNING("JSON day off color not found or invalid type.");
    }
    else if (false == checkAnalogClockConfig(jsonAnalogClock, analogClockConfig))
        /* Error printed in checkAnalogClockConfig() already. */
        ;
    else
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        m_mode          = static_cast<Mode>(jsonMode.as<uint8_t>());
        m_timeFormat    = jsonTimeFormat.as<String>();
        m_dateFormat    = jsonDateFormat.as<String>();
        m_timeZone      = jsonTimeZone.as<String>();

        status = m_view.setStartOfWeek(jsonStartOfWeek.as<uint8_t>());
        m_view.setDayOnColor(colorFromHtml(jsonDayOnColor.as<String>()));
        m_view.setDayOffColor(colorFromHtml(jsonDayOffColor.as<String>()));
        m_view.setViewMode(static_cast<IDateTimeView::ViewMode>(jsonViewMode.as<uint8_t>()));

        if (false == jsonAnalogClock.isNull())
        {
            m_view.setAnalogClockConfig(analogClockConfig);
        }

        m_hasTopicChanged = true;
    }

    return status;
}

void DateTimePlugin::updateDateTime(bool force)
{
    ClockDrv&   clockDrv            = ClockDrv::getInstance();
    struct tm   timeInfo            = { 0 };
    bool        isClockAvailable    = false;

    /* If no other timezone is given, the local time shall be used. */
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
        }

        /* cache time every second in view  (i.e. for analog clock) */
        if ((true == force) ||
            (m_shownSecond != timeInfo.tm_sec))
        {
            m_view.setCurrentTime(timeInfo);
        }

        if (true == showTime)
        {
            /* Show the time only in case
             * its forced to do it or
             * the time changed.
             */
            if ((true == force) ||
                (m_shownSecond != timeInfo.tm_sec))
            {
                String  extTimeFormat   = "{hc}" + m_timeFormat;
                String  timeAsStr;
                
                if (true == getTimeAsString(timeAsStr, extTimeFormat, &timeInfo))
                {
                    m_view.setFormatText(timeAsStr);

                    m_shownSecond = timeInfo.tm_sec;
                } 
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
                String  extDateFormat   = "{hc}" + m_dateFormat;
                String  dateAsStr;
                
                if (true == getTimeAsString(dateAsStr, extDateFormat, &timeInfo))
                {
                    m_view.setFormatText(dateAsStr);

                    m_shownDayOfTheYear = timeInfo.tm_yday;
                }              
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
            m_view.setFormatText("{hc}?");
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

String DateTimePlugin::colorToHtml(const Color& color)
{
    char buffer[8]; /* '#' + 3x byte in hex + '\0' */

    (void)snprintf(buffer, sizeof(buffer), "#%02X%02X%02X", color.getRed(), color.getGreen(), color.getBlue());

    return String(buffer);
}

Color DateTimePlugin::colorFromHtml(const String& htmlColor)
{
    Color color;

    if ('#' == htmlColor[0])
    {
        color = Util::hexToUInt32(htmlColor.substring(1U));
    }

    return color;
}

bool DateTimePlugin::checkAnalogClockConfig(JsonVariantConst& jsonCfg, IDateTimeView::AnalogClockConfig & cfg)
{
    bool result = true;

    if (false == jsonCfg.isNull())
    {
        JsonVariantConst jsonSecondsMode = jsonCfg["secondsMode"];

        if ((false == jsonSecondsMode.is<uint8_t>()) &&
            (IDateTimeView::SECONDS_DISP_MAX <= jsonSecondsMode.as<uint8_t>()))
        {
            LOG_WARNING("JSON seconds mode not found or invalid type.");
            result = false;
        } 
        else
        {
            cfg.m_secondsMode = static_cast<IDateTimeView::SecondsDisplayMode>(jsonSecondsMode.as<uint8_t>());

            for (uint32_t idx = 0U; idx < IDateTimeView::ANA_CLK_COL_MAX; ++ idx)
            {
                JsonVariantConst color = jsonCfg[ANALOG_CLOCK_COLOR_KEYS[idx]];

                if (false == color.is<String>())
                {
                    LOG_WARNING(
                        "JSON attribute %s not found or invalid type.",
                        ANALOG_CLOCK_COLOR_KEYS[idx]);
                    result = false;
                }
                else
                {
                    cfg.m_colors[idx] = colorFromHtml(color);
                }
            }
        }
    }

    return result;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

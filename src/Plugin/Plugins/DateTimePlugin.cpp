/* MIT License
 *
 * Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>
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
#include <FileSystem.h>
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

/* Initialize plugin topic. */
const char*     DateTimePlugin::TOPIC_CFG   = "/dateTime";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void DateTimePlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC_CFG);
}

bool DateTimePlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CFG))
    {
        value["cfg"] = getCfg();

        isSuccessful = true;
    }

    return isSuccessful;
}

bool DateTimePlugin::setTopic(const String& topic, const JsonObject& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CFG))
    {
        JsonVariant jsonCfg = value["cfg"];

        if (false == jsonCfg.isNull())
        {
            setCfg(static_cast<Cfg>(jsonCfg.as<uint8_t>()));

            isSuccessful = true;
        }
    }

    return isSuccessful;
}

void DateTimePlugin::setSlot(const ISlotPlugin* slotInterf)
{
    m_slotInterf = slotInterf;
    return;
}

void DateTimePlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    uint16_t                    lampWidth       = 0U;
    uint16_t                    lampDistance    = 0U;
    const uint16_t              minDistance     = 1U;   /* Min. distance between lamps. */
    const uint16_t              minBorder       = 1U;   /* Min. border left and right of all lamps. */

    m_textCanvas.setPosAndSize(0, 0, width, height - 2U);
    (void)m_textCanvas.addWidget(m_textWidget);

    m_lampCanvas.setPosAndSize(1, height - 1, width, 1U);

    if (true == calcLayout(width, MAX_LAMPS, minDistance, minBorder, lampWidth, lampDistance))
    {
        /* Calculate the border to have the days (lamps) shown aligned to center. */
        uint16_t    border  = (width - (MAX_LAMPS * (lampWidth + lampDistance))) / 2U;
        uint8_t     index   = 0U;

        for(index = 0U; index < MAX_LAMPS; ++index)
        {
            int16_t x = (lampWidth + lampDistance) * index + border;

            m_lampWidgets[index].setColorOn(ColorDef::LIGHTGRAY);
            m_lampWidgets[index].setColorOff(ColorDef::ULTRADARKGRAY);
            m_lampWidgets[index].setWidth(lampWidth);

            (void)m_lampCanvas.addWidget(m_lampWidgets[index]);
            m_lampWidgets[index].move(x, 0);
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

    return;
}

void DateTimePlugin::stop()
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    String                      configurationFilename   = getFullPathToConfiguration();

    if (false != FILESYSTEM.remove(configurationFilename))
    {
        LOG_INFO("File %s removed", configurationFilename.c_str());
    }

    return;
}

void DateTimePlugin::process()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if ((true == m_checkUpdateTimer.isTimerRunning()) &&
        (true == m_checkUpdateTimer.isTimeout()))
    {
        ++m_durationCounter;
        updateDateTime(false);

        m_checkUpdateTimer.restart();
    }

    return;
}

void DateTimePlugin::active(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    /* Force immediate date/time update on activation */
    updateDateTime(true);

    /* Force drawing on display in the update() method for the very first time
     * after activation.
     */
    m_isUpdateAvailable = true;
    m_durationCounter = 0U;
    m_checkUpdateTimer.start(CHECK_UPDATE_PERIOD);
}

void DateTimePlugin::inactive()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_checkUpdateTimer.stop();

    return;
}

void DateTimePlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (false != m_isUpdateAvailable)
    {
        gfx.fillScreen(ColorDef::BLACK);
        m_textCanvas.update(gfx);
        m_lampCanvas.update(gfx);

        m_isUpdateAvailable = false;
    }

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void DateTimePlugin::updateDateTime(bool force)
{
    struct tm   timeinfo    = { 0 };
    
    if (true == ClockDrv::getInstance().getTime(&timeinfo))
    {
        bool    showDate = false;
        bool    showTime = false;
        
        /* Decide what to show. */
        switch(m_cfg)
        {
        case CFG_DATE_TIME:
            {
                uint32_t    duration            = (nullptr == m_slotInterf) ? 0U : m_slotInterf->getDuration();
                uint8_t     halfDurationTicks   = 0U;
                uint8_t     fullDurationTicks   = 0U;

                /* If infinite duration was set, switch between time and date with a fix period. */
                if (0U == duration)
                {
                    halfDurationTicks   = MAX_COUNTER_VALUE_FOR_DURATION_INFINITE;
                    fullDurationTicks   = 2U * halfDurationTicks;
                }
                else
                {
                    halfDurationTicks   = (duration / (2U * MS_TO_SEC_DIVIDER));
                    fullDurationTicks   = 2U * halfDurationTicks;
                }

                if (false == force)
                {
                    /* Show time in the first period. */
                    if (0U == m_durationCounter)
                    {
                        showTime = true;
                    }
                    /* Show date in the second period. */
                    else if (halfDurationTicks == m_durationCounter)
                    {
                        showDate = true;
                    }
                    else
                    {
                        ;
                    }
                }
                else
                {
                    if (halfDurationTicks > m_durationCounter)
                    {
                        showTime = true;
                    }
                    else
                    {
                        showDate = true;
                    }
                }

                /* Reset duration after a complete plugin slot duration is finished. */
                if (fullDurationTicks <= m_durationCounter)
                {
                    m_durationCounter = 0U;
                }
            }
            break;

        case CFG_DATE_ONLY:
            if (false == force)
            {
                if (0U == m_durationCounter)
                {
                    showDate = true;
                }
            }
            else
            {
                showDate = true;
            }
            break;
        
        case CFG_TIME_ONLY:
            if (false == force)
            {
                if (0U == m_durationCounter)
                {
                    showTime = true;
                }
            }
            else
            {
                showTime = true;
            }
            break;
        
        default:
            /* Should never happen. */
            m_cfg = CFG_DATE_TIME;
            break;
        };

        if (true == showTime)
        {
            char timeBuffer [SIZE_OF_FORMATED_DATE_TIME_STRING];
            const char* formattedTimeString = ClockDrv::getInstance().getTimeFormat() ? "\\calign%H:%M":"\\calign%I:%M %p";

            setWeekdayIndicator(timeinfo);

            strftime(timeBuffer, sizeof(timeBuffer), formattedTimeString, &timeinfo);
            m_textWidget.setFormatStr(timeBuffer);
            m_isUpdateAvailable = true;
        }
        else if (true == showDate)
        {
            char dateBuffer [SIZE_OF_FORMATED_DATE_TIME_STRING];
            const char* formattedDateString = ClockDrv::getInstance().getDateFormat() ? "\\calign%d.%m.":"\\calign%m/%d";

            setWeekdayIndicator(timeinfo);

            strftime(dateBuffer, sizeof(dateBuffer), formattedDateString, &timeinfo);
            m_textWidget.setFormatStr(dateBuffer);

            m_isUpdateAvailable = true;
        }
        else
        {
            /* Nothing to update. */
            ;
        }
    }
}

void DateTimePlugin::setWeekdayIndicator(tm timeinfo)
{
    /* tm_wday starts at sunday, first lamp indicates monday.*/
    uint8_t activeLamp = (0U < timeinfo.tm_wday) ? (timeinfo.tm_wday - 1U) : (DateTimePlugin::MAX_LAMPS - 1U);

    /* Last active lamp has to be deactivated. */
    uint8_t lampToDeactivate = (0U < activeLamp) ? (activeLamp - 1U) : (DateTimePlugin::MAX_LAMPS - 1U);

    m_lampWidgets[activeLamp].setOnState(true);
    m_lampWidgets[lampToDeactivate].setOnState(false);
}

bool DateTimePlugin::calcLayout(uint16_t width, uint16_t cnt, uint16_t minDistance, uint16_t minBorder, uint16_t& elementWidth, uint16_t& elementDistance)
{
    bool    status  = false;

    /* The min. border (left and right) must not be greater than the given width. */
    if (width > (2U * minBorder))
    {
        uint16_t    availableWidth  = width - (2U * minBorder); /* The available width is calculated considering the min. borders. */

        /* The available width must be greater than the number of elements, including the min. element distance. */
        if (availableWidth > (cnt + ((cnt - 1U) * minDistance)))
        {
            uint16_t    maxElementWidth                     = (availableWidth - ((cnt - 1U) * minDistance)) / cnt; /* Max. element width, considering the given limitation. */
            uint16_t    elementWidthToAvailWidthRatio       = 8U;   /* 1 / N */
            uint16_t    elementDistanceToElementWidthRatio  = 4U;   /* 1 / N */
            uint16_t    elementWidthConsideringRatio        = availableWidth / elementWidthToAvailWidthRatio;

            /* Consider the ratio between element width to available width and
             * ratio between element distance to element width.
             * This is just to have a nice look.
             */
            if (maxElementWidth > elementWidthConsideringRatio)
            {
                uint16_t    elementDistanceConsideringRatio = elementWidthConsideringRatio / elementDistanceToElementWidthRatio;

                if (0U == elementDistanceConsideringRatio)
                {
                    if (0U == minDistance)
                    {
                        elementDistance = 0U;
                    }
                    else
                    {
                        elementWidth    = maxElementWidth;
                        elementDistance = (availableWidth - (cnt * maxElementWidth)) / (cnt - 1U);
                    }
                }
                else
                {
                    elementWidth    = elementWidthConsideringRatio - elementDistanceConsideringRatio;
                    elementDistance = elementDistanceConsideringRatio;
                }
            }
            else
            {
                elementWidth    = maxElementWidth;
                elementDistance = minDistance;
            }

            status = true;
        }
    }

    return status;
}

bool DateTimePlugin::saveConfiguration() const
{
    bool                status                  = true;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    String              configurationFilename   = getFullPathToConfiguration();

    jsonDoc["cfg"] = m_cfg;
    
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

bool DateTimePlugin::loadConfiguration()
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
        JsonVariant jsonCfg = jsonDoc["cfg"];

        if (false == jsonCfg.is<uint8_t>())
        {
            LOG_WARNING("JSON cfg not found or invalid type.");
            status = false;
        }
        else
        {
            m_cfg = static_cast<Cfg>(jsonCfg.as<uint8_t>());
        }
    }

    return status;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

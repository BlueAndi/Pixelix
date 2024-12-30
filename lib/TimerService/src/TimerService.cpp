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
 * @brief  Timer service
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "TimerService.h"

#include <Logging.h>
#include <JsonFile.h>
#include <ClockDrv.h>
#include <DisplayMgr.h>
#include <FileSystem.h>
#include <TopicHandlerService.h>
#include <SettingsService.h>
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

/* Initialize constant values. */
const char* TimerService::FILE_NAME = "/configuration/timerService.json";
const char* TimerService::TOPIC     = "/timer";
const char* TimerService::ENTITY    = "timerService";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool TimerService::start()
{
    bool                        isSuccessful = true;
    SettingsService&            settings     = SettingsService::getInstance();
    JsonObjectConst             jsonExtra;
    ITopicHandler::GetTopicFunc getTopicFunc =
        [this](const String& topic, JsonObject& jsonValue) -> bool {
        return this->getTopic(topic, jsonValue);
    };
    TopicHandlerService::HasChangedFunc hasChangedFunc =
        [this](const String& topic) -> bool {
        return this->hasTopicChanged(topic);
    };
    ITopicHandler::SetTopicFunc setTopicFunc =
        [this](const String& topic, const JsonObjectConst& jsonValue) -> bool {
        return this->setTopic(topic, jsonValue);
    };

    if (false == settings.open(true))
    {
        m_deviceId = settings.getHostname().getDefault();
    }
    else
    {
        m_deviceId = settings.getHostname().getValue();

        settings.close();
    }

    if (false == loadSettings())
    {
        saveSettings();
    }

    TopicHandlerService::getInstance().registerTopic(m_deviceId, ENTITY, TOPIC, jsonExtra, getTopicFunc, hasChangedFunc, setTopicFunc, nullptr);

    if (false == isSuccessful)
    {
        stop();
    }
    else
    {
        m_processTimer.start(PROCESS_PERIOD);

        LOG_INFO("Timer service started.");
    }

    return isSuccessful;
}

void TimerService::stop()
{
    m_processTimer.stop();
    TopicHandlerService::getInstance().unregisterTopic(m_deviceId, ENTITY, TOPIC);

    LOG_INFO("Timer service stopped.");
}

void TimerService::process()
{
    if ((true == m_processTimer.isTimerRunning()) &&
        (true == m_processTimer.isTimeout()))
    {
        ClockDrv& clockDrv = ClockDrv::getInstance();
        struct tm time;

        if (true == clockDrv.getTime(time))
        {
            MutexGuard<Mutex> guard(m_mutex);
            size_t            idx;

            for (idx = 0U; idx < MAX_TIMER_COUNT; ++idx)
            {
                if (true == m_settings[idx].isEnabled())
                {
                    if (true == m_settings[idx].isSignalling(time))
                    {
                        TimerSetting::DisplayState displayState = m_settings[idx].getDisplayState();
                        int16_t                    brightness   = m_settings[idx].getBrightness();

                        if (TimerSetting::DISPLAY_STATE_ON == displayState)
                        {
                            LOG_INFO("Timer %u is switching display on.", idx);

                            DisplayMgr::getInstance().displayOn();
                        }
                        else if (TimerSetting::DISPLAY_STATE_OFF == displayState)
                        {
                            LOG_INFO("Timer %u is switching display off.", idx);

                            DisplayMgr::getInstance().displayOff();
                        }
                        else
                        {
                            ;
                        }

                        if ((0 <= brightness) && (255 >= brightness))
                        {
                            LOG_INFO("Timer %u is setting brightness to %d.", idx, brightness);

                            DisplayMgr::getInstance().setBrightness(static_cast<uint8_t>(brightness));
                        }
                    }
                }
            }
        }

        m_processTimer.restart();
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void TimerService::clear()
{
    size_t idx;

    for (idx = 0U; idx < MAX_TIMER_COUNT; ++idx)
    {
        m_settings[idx].clear();
    }
}

bool TimerService::loadSettings()
{
    bool                isSuccessful = false;
    const size_t        JSON_SIZE    = 4096U;
    DynamicJsonDocument jsonDoc(JSON_SIZE);
    JsonFile            jsonFile(FILESYSTEM);

    if (false == jsonFile.load(FILE_NAME, jsonDoc))
    {
        LOG_WARNING("Failed to load timer settings.");
    }
    else
    {
        JsonVariantConst jsonTimerSettings = jsonDoc["timerSettings"];

        if (false == jsonTimerSettings.is<JsonArrayConst>())
        {
            LOG_ERROR("No timer settings found.");
        }
        else
        {
            JsonArrayConst jsonTimerSettingsArray = jsonTimerSettings.as<JsonArrayConst>();
            size_t         idx                    = 0U;

            clear();

            for (JsonObjectConst jsonTimerSetting : jsonTimerSettingsArray)
            {
                if (false == m_settings[idx].fromJson(jsonTimerSetting))
                {
                    LOG_WARNING("Failed to load timer setting %u.", idx);
                }
                else
                {
                    ++idx;
                }

                if (MAX_TIMER_COUNT <= idx)
                {
                    break;
                }
            }

            m_hasSettingsChanged = true;

            isSuccessful         = true;
        }
    }

    return isSuccessful;
}

bool TimerService::saveSettings()
{
    bool                isSuccessful = false;
    const size_t        JSON_SIZE    = 4096U;
    DynamicJsonDocument jsonDoc(JSON_SIZE);
    JsonArray           jsonTimerSettings = jsonDoc.createNestedArray("timerSettings");
    JsonFile            jsonFile(FILESYSTEM);
    size_t              idx;

    for (idx = 0U; idx < MAX_TIMER_COUNT; ++idx)
    {
        JsonObject jsonTimerSetting = jsonTimerSettings.createNestedObject();

        m_settings[idx].toJson(jsonTimerSetting);
    }

    if (false == jsonFile.save(FILE_NAME, jsonDoc))
    {
        LOG_ERROR("Failed to save timer settings.");
    }
    else
    {
        isSuccessful = true;
    }

    return isSuccessful;
}

bool TimerService::getTopic(const String& topic, JsonObject& jsonValue)
{
    size_t            idx;
    JsonArray         jsonTimerSettings = jsonValue.createNestedArray("timerSettings");
    MutexGuard<Mutex> guard(m_mutex);

    /* The callback is dedicated to a topic, therefore the
     * topic parameter is not used.
     */
    UTIL_NOT_USED(topic);

    for (idx = 0U; idx < MAX_TIMER_COUNT; ++idx)
    {
        JsonObject jsonTimerSetting = jsonTimerSettings.createNestedObject();

        m_settings[idx].toJson(jsonTimerSetting);
    }

    return true;
}

bool TimerService::hasTopicChanged(const String& topic)
{
    bool hasChanged      = m_hasSettingsChanged;

    m_hasSettingsChanged = false;

    return hasChanged;
}

bool TimerService::setTopic(const String& topic, const JsonObjectConst& jsonValue)
{
    bool              isSuccessful = false;
    size_t            idx;
    JsonVariantConst  jsonTimerSettings = jsonValue["timerSettings"];
    MutexGuard<Mutex> guard(m_mutex);

    /* The callback is dedicated to a topic, therefore the
     * topic parameter is not used.
     */
    UTIL_NOT_USED(topic);

    if (true == jsonTimerSettings.is<JsonArrayConst>())
    {
        JsonArrayConst jsonTimerSettingsArray = jsonTimerSettings.as<JsonArrayConst>();
        size_t         count                  = (MAX_TIMER_COUNT >= jsonTimerSettingsArray.size()) ? jsonTimerSettingsArray.size() : MAX_TIMER_COUNT;

        for (idx = 0U; idx < count; ++idx)
        {
            if (false == m_settings[idx].fromJson(jsonTimerSettingsArray[idx]))
            {
                LOG_WARNING("Failed to set timer setting %u.", idx);
            }
        }

        m_hasSettingsChanged = true;
        isSuccessful         = true;
    }

    if (true == isSuccessful)
    {
        isSuccessful = saveSettings();
    }

    return isSuccessful;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

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
 * @brief  Timer setting
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "TimerSetting.h"

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

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void TimerSetting::clear()
{
    m_isEnabled    = false;
    m_hour         = 0U;
    m_minute       = 0U;
    m_daysOfWeek   = 0U;
    m_displayState = DISPLAY_STATE_NONE;
    m_brightness   = -1;
}

void TimerSetting::toJson(JsonObject& jsonTimerSetting) const
{
    jsonTimerSetting["enabled"]      = m_isEnabled;
    jsonTimerSetting["hour"]         = m_hour;
    jsonTimerSetting["minute"]       = m_minute;
    jsonTimerSetting["sunday"]       = isDayOfWeek(0U);
    jsonTimerSetting["monday"]       = isDayOfWeek(1U);
    jsonTimerSetting["tuesday"]      = isDayOfWeek(2U);
    jsonTimerSetting["wednesday"]    = isDayOfWeek(3U);
    jsonTimerSetting["thursday"]     = isDayOfWeek(4U);
    jsonTimerSetting["friday"]       = isDayOfWeek(5U);
    jsonTimerSetting["saturday"]     = isDayOfWeek(6U);
    jsonTimerSetting["displayState"] = m_displayState;
    jsonTimerSetting["brightness"]   = m_brightness;
}

bool TimerSetting::fromJson(const JsonObjectConst& jsonTimerSetting)
{
    bool             isSuccessful     = false;
    JsonVariantConst jsonEnabled      = jsonTimerSetting["enabled"];
    JsonVariantConst jsonHour         = jsonTimerSetting["hour"];
    JsonVariantConst jsonMinute       = jsonTimerSetting["minute"];
    JsonVariantConst jsonSunday       = jsonTimerSetting["sunday"];
    JsonVariantConst jsonMonday       = jsonTimerSetting["monday"];
    JsonVariantConst jsonTuesday      = jsonTimerSetting["tuesday"];
    JsonVariantConst jsonWednesday    = jsonTimerSetting["wednesday"];
    JsonVariantConst jsonThursday     = jsonTimerSetting["thursday"];
    JsonVariantConst jsonFriday       = jsonTimerSetting["friday"];
    JsonVariantConst jsonSaturday     = jsonTimerSetting["saturday"];
    JsonVariantConst jsonDisplayState = jsonTimerSetting["displayState"];
    JsonVariantConst jsonBrightness   = jsonTimerSetting["brightness"];

    if ((false == jsonEnabled.isNull()) &&
        (false == jsonHour.isNull()) &&
        (false == jsonMinute.isNull()) &&
        (false == jsonSunday.isNull()) &&
        (false == jsonMonday.isNull()) &&
        (false == jsonTuesday.isNull()) &&
        (false == jsonWednesday.isNull()) &&
        (false == jsonThursday.isNull()) &&
        (false == jsonFriday.isNull()) &&
        (false == jsonSaturday.isNull()) &&
        (false == jsonDisplayState.isNull()) &&
        (false == jsonBrightness.isNull()))
    {
        clear();

        m_hour         = jsonHour.as<uint8_t>();
        m_minute       = jsonMinute.as<uint8_t>();
        m_displayState = static_cast<DisplayState>(jsonDisplayState.as<uint8_t>());
        m_brightness   = jsonBrightness.as<int16_t>();

        if (true == jsonEnabled.is<bool>())
        {
            m_isEnabled = jsonEnabled.as<bool>();
        }
        else if (true == jsonEnabled.as<String>().equals("false"))
        {
            m_isEnabled = false;
        }
        else
        {
            m_isEnabled = true;
        }

        if (((true == jsonSunday.is<bool>()) && (true == jsonSunday.as<bool>())) ||
            (true == jsonSunday.as<String>().equals("true")))
        {
            m_daysOfWeek |= (1U << 0U);
        }

        if (((true == jsonMonday.is<bool>()) && (true == jsonMonday.as<bool>())) ||
            (true == jsonMonday.as<String>().equals("true")))
        {
            m_daysOfWeek |= (1U << 1U);
        }

        if (((true == jsonTuesday.is<bool>()) && (true == jsonTuesday.as<bool>())) ||
            (true == jsonTuesday.as<String>().equals("true")))
        {
            m_daysOfWeek |= (1U << 2U);
        }

        if (((true == jsonWednesday.is<bool>()) && (true == jsonWednesday.as<bool>())) ||
            (true == jsonWednesday.as<String>().equals("true")))
        {
            m_daysOfWeek |= (1U << 3U);
        }

        if (((true == jsonThursday.is<bool>()) && (true == jsonThursday.as<bool>())) ||
            (true == jsonThursday.as<String>().equals("true")))
        {
            m_daysOfWeek |= (1U << 4U);
        }

        if (((true == jsonFriday.is<bool>()) && (true == jsonFriday.as<bool>())) ||
            (true == jsonFriday.as<String>().equals("true")))
        {
            m_daysOfWeek |= (1U << 5U);
        }

        if (((true == jsonSaturday.is<bool>()) && (true == jsonSaturday.as<bool>())) ||
            (true == jsonSaturday.as<String>().equals("true")))
        {
            m_daysOfWeek |= (1U << 6U);
        }

        isSuccessful = true;
    }

    return isSuccessful;
}

bool TimerSetting::isSignalling(const struct tm& currentTime)
{
    bool isSignalling = (m_hour == currentTime.tm_hour) && (m_minute == currentTime.tm_min) && isDayOfWeek(currentTime.tm_wday);

    if (true == isSignalling)
    {
        if (false == m_isSignalling)
        {
            m_isSignalling = true;
        }
        else
        {
            isSignalling = false;
        }
    }
    else
    {
        m_isSignalling = false;
    }

    return isSignalling;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

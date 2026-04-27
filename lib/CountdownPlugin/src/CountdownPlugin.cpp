/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   CountdownPlugin.cpp
 * @brief  Countdown plugin.
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ClockDrv.h"
#include "CountdownPlugin.h"

#include <ArduinoJson.h>
#include <Logging.h>

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
const char* CountdownPlugin::TOPIC_CONFIG = "countdown";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void CountdownPlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC_CONFIG);
}

bool CountdownPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_CONFIG))
    {
        getConfiguration(value);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool CountdownPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_CONFIG))
    {
        const size_t        JSON_DOC_SIZE = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg          = jsonDoc.to<JsonObject>();
        JsonVariantConst    jsonDay          = value["day"];
        JsonVariantConst    jsonMonth        = value["month"];
        JsonVariantConst    jsonYear         = value["year"];
        JsonVariantConst    jsonDescPlural   = value["descPlural"];
        JsonVariantConst    jsonDescSingular = value["descSingular"];

        /* The received configuration may not contain all single key/value pair.
         * Therefore read first the complete internal configuration and
         * overwrite them with the received ones.
         */
        getConfiguration(jsonCfg);

        /* Note:
         * Check only for the key/value pair availability.
         * The type check will follow in the setConfiguration().
         */

        if (false == jsonDay.isNull())
        {
            jsonCfg["day"] = jsonDay.as<uint8_t>();
            isSuccessful   = true;
        }

        if (false == jsonMonth.isNull())
        {
            jsonCfg["month"] = jsonMonth.as<uint8_t>();
            isSuccessful     = true;
        }

        if (false == jsonYear.isNull())
        {
            jsonCfg["year"] = jsonYear.as<uint16_t>();
            isSuccessful    = true;
        }

        if (false == jsonDescPlural.isNull())
        {
            jsonCfg["descPlural"] = jsonDescPlural.as<const char*>();
            isSuccessful          = true;
        }

        if (false == jsonDescSingular.isNull())
        {
            jsonCfg["descSingular"] = jsonDescSingular.as<const char*>();
            isSuccessful            = true;
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

bool CountdownPlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive> guard(m_mutex);
    bool                       hasTopicChanged = m_hasTopicChanged;

    /* Only a single topic, therefore its not necessary to check. */
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void CountdownPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.init(width, height);

    PluginWithConfig::start(width, height);

    calculateRemainingDays();
}

void CountdownPlugin::stop()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    PluginWithConfig::stop();
}

void CountdownPlugin::process(bool isConnected)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    PluginWithConfig::process(isConnected);

    calculateRemainingDays();
}

void CountdownPlugin::update(YAGfx& gfx)
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

void CountdownPlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["day"]          = m_targetDate.day;
    jsonCfg["month"]        = m_targetDate.month;
    jsonCfg["year"]         = m_targetDate.year;
    jsonCfg["descPlural"]   = m_targetDateInformation.plural;
    jsonCfg["descSingular"] = m_targetDateInformation.singular;
}

bool CountdownPlugin::setConfiguration(const JsonObjectConst& jsonCfg)
{
    bool             status           = false;
    JsonVariantConst jsonDay          = jsonCfg["day"];
    JsonVariantConst jsonMonth        = jsonCfg["month"];
    JsonVariantConst jsonYear         = jsonCfg["year"];
    JsonVariantConst jsonDescPlural   = jsonCfg["descPlural"];
    JsonVariantConst jsonDescSingular = jsonCfg["descSingular"];

    if (false == jsonDay.is<uint8_t>())
    {
        LOG_WARNING("JSON day not found or invalid type.");
    }
    else if (false == jsonMonth.is<uint8_t>())
    {
        LOG_WARNING("JSON month not found or invalid type.");
    }
    else if (false == jsonYear.is<uint16_t>())
    {
        LOG_WARNING("JSON year not found or invalid type.");
    }
    else if (false == jsonDescPlural.is<String>())
    {
        LOG_WARNING("JSON descriptionPlural not found or invalid type.");
    }
    else if (false == jsonDescSingular.is<String>())
    {
        LOG_WARNING("JSON descriptionSingular not found or invalid type.");
    }
    else
    {
        const uint8_t  DAY_LOWER_LIMIT   = 1U;
        const uint8_t  DAY_UPPER_LIMIT   = 31U;
        const uint8_t  MONTH_LOWER_LIMIT = 1U;
        const uint8_t  MONTH_UPPER_LIMIT = 12U;
        const uint16_t YEAR_LOWER_LIMIT  = 1970U;
        const uint16_t YEAR_UPPER_LIMIT  = 2100U;
        uint8_t        day               = jsonDay.as<uint8_t>();
        uint8_t        month             = jsonMonth.as<uint8_t>();
        uint16_t       year              = jsonYear.as<uint16_t>();

        if ((DAY_LOWER_LIMIT > day) || (DAY_UPPER_LIMIT < day))
        {
            status = false;
        }
        else if ((MONTH_LOWER_LIMIT > month) || (MONTH_UPPER_LIMIT < month))
        {
            status = false;
        }
        else if ((YEAR_LOWER_LIMIT > year) || (YEAR_UPPER_LIMIT < year))
        {
            status = false;
        }
        else if (0U == strlen(jsonDescPlural.as<const char*>()))
        {
            status = false;
        }
        else if (0U == strlen(jsonDescSingular.as<const char*>()))
        {
            status = false;
        }
        else
        {
            MutexGuard<MutexRecursive> guard(m_mutex);

            m_targetDate.day                 = day;
            m_targetDate.month               = month;
            m_targetDate.year                = year;
            m_targetDateInformation.plural   = jsonDescPlural.as<const char*>();
            m_targetDateInformation.singular = jsonDescSingular.as<const char*>();

            m_hasTopicChanged                = true;

            status                           = true;
        }
    }

    return status;
}

void CountdownPlugin::calculateRemainingDays()
{
    tm currentTime;

    if (true == ClockDrv::getInstance().getTime(currentTime))
    {
        int32_t currentDateInDays  = 0;
        int32_t targetDateInDays   = 0;
        int32_t numberOfDays       = 0;

        m_currentDate.day          = static_cast<uint8_t>(currentTime.tm_mday);
        m_currentDate.month        = static_cast<uint8_t>(currentTime.tm_mon);
        m_currentDate.year         = static_cast<uint16_t>(currentTime.tm_year);
        m_currentDate.year        += TM_OFFSET_YEAR;
        m_currentDate.month       += TM_OFFSET_MONTH;

        currentDateInDays          = static_cast<int32_t>(dateToDays(m_currentDate));
        targetDateInDays           = static_cast<int32_t>(dateToDays(m_targetDate));

        numberOfDays               = targetDateInDays - currentDateInDays;

        if (0 < numberOfDays)
        {
            char remaining[10] = "";

            (void)snprintf(remaining, sizeof(remaining), " %d", numberOfDays);
            m_remainingDays  = remaining;
            m_remainingDays += " ";

            if (numberOfDays > 1)
            {
                m_remainingDays += m_targetDateInformation.plural;
            }
            else
            {
                m_remainingDays += m_targetDateInformation.singular;
            }
        }
        else
        {
            m_remainingDays = "{hc}ELAPSED!";
        }

        m_view.setFormatText(m_remainingDays);
    }
}

uint16_t CountdownPlugin::countLeapYears(const CountdownPlugin::DateDMY& date) const
{
    uint16_t years = date.year;

    /* Check if the current year needs to be considered for the count of leap years or not. */
    if (date.month <= 2U)
    {
        --years;
    }

    /* An year is a leap year if it is a multiple of 4, multiple of 400 and not a multiple of 100. */
    return years / 4U - years / 100U + years / 400U;
}

uint32_t CountdownPlugin::dateToDays(const CountdownPlugin::DateDMY& date) const
{
    const uint8_t monthDays[12] = { 31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U };
    uint32_t      dateInDays    = 0U;
    uint8_t       i             = 0U;

    dateInDays                  = date.year * 365U + date.day;

    for (i = 0U; i < (date.month - 1U); i++)
    {
        dateInDays += monthDays[i];
    }

    dateInDays += countLeapYears(date);

    return dateInDays;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

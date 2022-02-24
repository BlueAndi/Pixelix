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
 * @brief  Countdown plugin.
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ClockDrv.h"
#include "CountdownPlugin.h"
#include "RestApi.h"
#include "Util.h"
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

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/* Initialize image path. */
const char* CountdownPlugin::IMAGE_PATH = "/images/countdown.bmp";

/* Initialize plugin topic. */
const char* CountdownPlugin::TOPIC      = "/countdown";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void CountdownPlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC);
}

bool CountdownPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC))
    {
        DateDMY                 targetDate              = getTargetDate();
        TargetDayDescription    targetDayDescription    = getTargetDayDescription();

        value["day"]      = targetDate.day;
        value["month"]    = targetDate.month;
        value["year"]     = targetDate.year;
        value["plural"]   = targetDayDescription.plural;
        value["singular"] = targetDayDescription.singular;

        isSuccessful = true;
    }

    return isSuccessful;
}

bool CountdownPlugin::setTopic(const String& topic, const JsonObject& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC))
    {
        DateDMY                 targetDate              = getTargetDate();
        TargetDayDescription    targetDayDescription    = getTargetDayDescription();

        if (false == value["day"].isNull())
        {
            targetDate.day = value["day"].as<uint8_t>();
            isSuccessful = true;
        }

        if (false == value["month"].isNull())
        {
            targetDate.month = value["month"].as<uint8_t>();
            isSuccessful = true;
        }

        if (false == value["year"].isNull())
        {
            targetDate.year = value["year"].as<uint16_t>();
            isSuccessful = true;
        }

        if (false == value["plural"].isNull())
        {
            targetDayDescription.plural = value["plural"].as<String>();
            isSuccessful = true;
        }

        if (false == value["singular"].isNull())
        {
            targetDayDescription.singular = value["singular"].as<String>();
            isSuccessful = true;
        }

        if (true == isSuccessful)
        {
            setTargetDate(targetDate);
            setTargetDayDescription(targetDayDescription);
        }
    }

    return isSuccessful;
}

void CountdownPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_iconCanvas.setPosAndSize(0, 0, ICON_WIDTH, ICON_HEIGHT);
    (void)m_iconCanvas.addWidget(m_bitmapWidget);

    (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH);

    m_textCanvas.setPosAndSize(ICON_WIDTH, 0, width - ICON_WIDTH, height);
    (void)m_textCanvas.addWidget(m_textWidget);

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

    calculateDifferenceInDays();

    m_cfgReloadTimer.start(CFG_RELOAD_PERIOD);

    return;
}

void CountdownPlugin::stop()
{
    String                      configurationFilename = getFullPathToConfiguration();
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_cfgReloadTimer.stop();

    if (false != FILESYSTEM.remove(configurationFilename))
    {
        LOG_INFO("File %s removed", configurationFilename.c_str());
    }

    return;
}

void CountdownPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if ((true == m_cfgReloadTimer.isTimerRunning()) &&
        (true == m_cfgReloadTimer.isTimeout()))
    {
        (void)loadConfiguration();
        calculateDifferenceInDays();

        m_cfgReloadTimer.restart();
    }

    gfx.fillScreen(ColorDef::BLACK);
    m_iconCanvas.update(gfx);
    m_textCanvas.update(gfx);

    return;
}

CountdownPlugin::DateDMY CountdownPlugin::getTargetDate() const
{
    DateDMY                     targetDate;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    targetDate = m_targetDate;

    return targetDate;
}

void CountdownPlugin::setTargetDate(const DateDMY& targetDate)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if ((targetDate.day != m_targetDate.day) ||
        (targetDate.month != m_targetDate.month) ||
        (targetDate.year != m_targetDate.year))
    {
        LOG_INFO("New target date: %04u-%02u-%02u", targetDate.year, targetDate.month, targetDate.day);

        m_targetDate = targetDate;

        /* Always stores the configuration, otherwise it will be overwritten during
         * plugin activation.
         */
        (void)saveConfiguration();
    }

    return;
}

CountdownPlugin::TargetDayDescription CountdownPlugin::getTargetDayDescription() const
{
    TargetDayDescription        desc;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    desc = m_targetDateInformation;

    return desc;
}

void CountdownPlugin::setTargetDayDescription(const TargetDayDescription& targetDayDescription)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if ((targetDayDescription.plural != m_targetDateInformation.plural) ||
        (targetDayDescription.singular != m_targetDateInformation.singular))
    {
        LOG_INFO("New unit description: \"%s\" / \"%s\"", targetDayDescription.plural.c_str(), targetDayDescription.singular.c_str());

        m_targetDateInformation = targetDayDescription;

        /* Always stores the configuration, otherwise it will be overwritten during
         * plugin activation.
         */
        (void)saveConfiguration();
    }

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool CountdownPlugin::saveConfiguration() const
{
    bool                status                  = true;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    String              configurationFilename   = getFullPathToConfiguration();

    jsonDoc["day"]                  = m_targetDate.day;
    jsonDoc["month"]                = m_targetDate.month;
    jsonDoc["year"]                 = m_targetDate.year;
    jsonDoc["descriptionPlural"]    = m_targetDateInformation.plural;
    jsonDoc["descriptionSingular"]  = m_targetDateInformation.singular;
    
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

bool CountdownPlugin::loadConfiguration()
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
        m_targetDate.day                    = jsonDoc["day"].as<uint8_t>();
        m_targetDate.month                  = jsonDoc["month"].as<uint8_t>();
        m_targetDate.year                   = jsonDoc["year"].as<uint16_t>();
        m_targetDateInformation.plural      = jsonDoc["descriptionPlural"].as<String>();
        m_targetDateInformation.singular    = jsonDoc["descriptionSingular"].as<String>();
    }

    return status;
}

void CountdownPlugin::calculateDifferenceInDays()
{
    tm currentTime;

    if (false != ClockDrv::getInstance().getTime(&currentTime))
    {
        uint32_t    currentDateInDays   = 0U;
        uint32_t    targetDateInDays    = 0U;
        int32_t     numberOfDays        = 0;

        m_currentDate.day = currentTime.tm_mday;
        m_currentDate.month = currentTime.tm_mon;
        m_currentDate.year = currentTime.tm_year;
        m_currentDate.year += TM_OFFSET_YEAR;
        m_currentDate.month += TM_OFFSET_MONTH;

        currentDateInDays = dateToDays(m_currentDate);

        targetDateInDays = dateToDays(m_targetDate);

        numberOfDays = targetDateInDays - currentDateInDays;

        if( numberOfDays > 0)
        {
            char remaining[10] = "";

            (void)snprintf(remaining, sizeof(remaining), " %d", numberOfDays);
            m_remainingDays  = remaining;
            m_remainingDays += " ";

            if(numberOfDays > 1)
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
            m_remainingDays = "ELAPSED!";
        }

        m_textWidget.setFormatStr(m_remainingDays);
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
    const uint8_t   monthDays[12]   = { 31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U };
    uint32_t        dateInDays      = 0U;
    uint8_t         i               = 0U;

    dateInDays = date.year * 365U + date.day;

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

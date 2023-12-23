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
 * @brief  Countdown plugin.
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ClockDrv.h"
#include "CountdownPlugin.h"
#include "Util.h"

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

/* Initialize image path. */
const char* CountdownPlugin::IMAGE_PATH     = "/plugins/CountdownPlugin/countdown.bmp";

/* Initialize plugin topic. */
const char* CountdownPlugin::TOPIC_CONFIG   = "/countdown";

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

    if (0U != topic.equals(TOPIC_CONFIG))
    {
        getConfiguration(value);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool CountdownPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CONFIG))
    {
        const size_t        JSON_DOC_SIZE           = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg                 = jsonDoc.to<JsonObject>();
        JsonVariantConst    jsonDay                 = value["day"];
        JsonVariantConst    jsonMonth               = value["month"];
        JsonVariantConst    jsonYear                = value["year"];
        JsonVariantConst    jsonDescPlural          = value["descPlural"];
        JsonVariantConst    jsonDescSingular        = value["descSingular"];

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
            isSuccessful = true;
        }

        if (false == jsonMonth.isNull())
        {
            jsonCfg["month"] = jsonMonth.as<uint8_t>();
            isSuccessful = true;
        }

        if (false == jsonYear.isNull())
        {
            jsonCfg["year"] = jsonYear.as<uint16_t>();
            isSuccessful = true;
        }

        if (false == jsonDescPlural.isNull())
        {
            jsonCfg["descPlural"] = jsonDescPlural.as<String>();
            isSuccessful = true;
        }

        if (false == jsonDescSingular.isNull())
        {
            jsonCfg["descSingular"] = jsonDescSingular.as<String>();
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

bool CountdownPlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    bool                        hasTopicChanged = m_hasTopicChanged;

    /* Only a single topic, therefore its not necessary to check. */
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void CountdownPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

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

    calculateRemainingDays();
}

void CountdownPlugin::stop()
{
    String                      configurationFilename   = getFullPathToConfiguration();
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_cfgReloadTimer.stop();

    if (false != FILESYSTEM.remove(configurationFilename))
    {
        LOG_INFO("File %s removed", configurationFilename.c_str());
    }
}

void CountdownPlugin::process(bool isConnected)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    PLUGIN_NOT_USED(isConnected);

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

    calculateRemainingDays();
}

void CountdownPlugin::update(YAGfx& gfx)
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

void CountdownPlugin::requestStoreToPersistentMemory()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_storeConfigReq = true;
}

void CountdownPlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["day"]          = m_targetDate.day;
    jsonCfg["month"]        = m_targetDate.month;
    jsonCfg["year"]         = m_targetDate.year;
    jsonCfg["descPlural"]   = m_targetDateInformation.plural;
    jsonCfg["descSingular"] = m_targetDateInformation.singular;
}

bool CountdownPlugin::setConfiguration(JsonObjectConst& jsonCfg)
{
    bool                status              = false;
    JsonVariantConst    jsonDay             = jsonCfg["day"];
    JsonVariantConst    jsonMonth           = jsonCfg["month"];
    JsonVariantConst    jsonYear            = jsonCfg["year"];
    JsonVariantConst    jsonDescPlural      = jsonCfg["descPlural"];
    JsonVariantConst    jsonDescSingular    = jsonCfg["descSingular"];

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
        MutexGuard<MutexRecursive> guard(m_mutex);

        m_targetDate.day                    = jsonDay.as<uint8_t>();
        m_targetDate.month                  = jsonMonth.as<uint8_t>();
        m_targetDate.year                   = jsonYear.as<uint16_t>();
        m_targetDateInformation.plural      = jsonDescPlural.as<String>();
        m_targetDateInformation.singular    = jsonDescSingular.as<String>();

        m_hasTopicChanged = true;

        status = true;
    }

    return status;
}

void CountdownPlugin::calculateRemainingDays()
{
    tm currentTime;

    if (true == ClockDrv::getInstance().getTime(currentTime))
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

        if (0 < numberOfDays)
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

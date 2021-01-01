/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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

#include <ArduinoJson.h>
#include <Logging.h>
#include <SPIFFS.h>
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
const char* CountdownPlugin::IMAGE_PATH     = "/images/countdown.bmp";

/* Initialize configuration path. */
const char* CountdownPlugin::CONFIG_PATH    = "/configuration";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void CountdownPlugin::registerWebInterface(AsyncWebServer& srv, const String& baseUri)
{
    m_url = baseUri + "/countdown";

    m_callbackWebHandler = &srv.on( m_url.c_str(),
                                    [this](AsyncWebServerRequest *request)
                                    {
                                        this->webReqHandler(request);
                                    });

    LOG_INFO("[%s] Register: %s", getName(), m_url.c_str());

    return;
}

void CountdownPlugin::unregisterWebInterface(AsyncWebServer& srv)
{
    LOG_INFO("[%s] Unregister: %s", getName(), m_url.c_str());

    if (false == srv.removeHandler(m_callbackWebHandler))
    {
        LOG_WARNING("Couldn't remove %s handler.", getName());
    }

    m_callbackWebHandler = nullptr;

    return;
}

void CountdownPlugin::active(IGfx& gfx)
{
    lock();

    if (nullptr == m_iconCanvas)
    {
        m_iconCanvas = new Canvas(ICON_WIDTH, ICON_HEIGHT, 0, 0);

        if (nullptr != m_iconCanvas)
        {
            (void)m_iconCanvas->addWidget(m_bitmapWidget);

            /* Load  icon from filesystem. */
            (void)m_bitmapWidget.load(IMAGE_PATH);
        }
    }

    if (nullptr == m_textCanvas)
    {
        m_textCanvas = new Canvas(gfx.getWidth() - ICON_WIDTH, gfx.getHeight(), ICON_WIDTH, 0);

        if (nullptr != m_textCanvas)
        {
            (void)m_textCanvas->addWidget(m_textWidget);
        }
    }

    unlock();

    return;
}

void CountdownPlugin::inactive()
{
    /* Nothing to do */
    return;
}

void CountdownPlugin::update(IGfx& gfx)
{
    lock();

    if ((true == m_cfgReloadTimer.isTimerRunning()) &&
        (true == m_cfgReloadTimer.isTimeout()))
    {
        (void)loadConfiguration();
        calculateDifferenceInDays();

        m_cfgReloadTimer.restart();
    }

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

void CountdownPlugin::start()
{
    lock();

    m_configurationFilename = String(CONFIG_PATH) + "/" + getUID() + ".json";

    /* Try to load configuration. If there is no configuration available, a default configuration
     * will be created.
     */
    createConfigDirectory();
    if (false == loadConfiguration())
    {
        if (false == saveConfiguration())
        {
            LOG_WARNING("Failed to create initial configuration file %s.", m_configurationFilename.c_str());
        }
    }

    calculateDifferenceInDays();

    m_cfgReloadTimer.start(CFG_RELOAD_PERIOD);

    unlock();

    return;
}

void CountdownPlugin::stop()
{
    lock();

    m_cfgReloadTimer.stop();

    if (false != SPIFFS.remove(m_configurationFilename))
    {
        LOG_INFO("File %s removed", m_configurationFilename.c_str());
    }

    unlock();

    return;
}

CountdownPlugin::DateDMY CountdownPlugin::getTargetDate() const
{
    DateDMY targetDate;

    lock();

    targetDate = m_targetDate;

    unlock();

    return targetDate;
}

void CountdownPlugin::setTargetDate(const DateDMY& targetDate)
{
    lock();

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

    unlock();

    return;
}

CountdownPlugin::TargetDayDescription CountdownPlugin::getTargetDayDescription() const
{
    TargetDayDescription desc;

    lock();

    desc = m_targetDateInformation;

    unlock();

    return desc;
}

void CountdownPlugin::setTargetDayDescription(const TargetDayDescription& targetDayDescription)
{
    lock();

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

    unlock();

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void CountdownPlugin::webReqHandler(AsyncWebServerRequest *request)
{
    String              content;
    const size_t        JSON_DOC_SIZE   = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;
    const size_t        MAX_USAGE       = 80U;
    size_t              usageInPercent  = 0U;

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET == request->method())
    {
        DateDMY                 targetDate              = getTargetDate();
        TargetDayDescription    targetDayDescription    = getTargetDayDescription();
        JsonObject              dataObj                 = jsonDoc.createNestedObject("data");

        dataObj["day"]      = targetDate.day;
        dataObj["month"]    = targetDate.month;
        dataObj["year"]     = targetDate.year;
        dataObj["plural"]   = targetDayDescription.plural;
        dataObj["singular"] = targetDayDescription.singular;

        /* Prepare response */
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);
        httpStatusCode      = HttpStatus::STATUS_CODE_OK;
    }
    else if (HTTP_POST == request->method())
    {
        /* Target date missing? */
        if ((false == request->hasArg("day")) ||
            (false == request->hasArg("month")) ||
            (false == request->hasArg("year")) ||
            (false == request->hasArg("plural")) ||
            (false == request->hasArg("singular")))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Argument is missing.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else
        {
            DateDMY                 targetDate              = getTargetDate();
            TargetDayDescription    targetDayDescription    = getTargetDayDescription();
            bool                    isSuccess               = true;

            if ((true == isSuccess) &&
                (true == request->hasArg("day")))
            {
                isSuccess = Util::strToUInt8(request->arg("day"), targetDate.day);
            }

            if ((true == isSuccess) &&
                (true == request->hasArg("month")))
            {
                isSuccess = Util::strToUInt8(request->arg("month"), targetDate.month);
            }

            if ((true == isSuccess) &&
                (true == request->hasArg("year")))
            {
                isSuccess = Util::strToUInt16(request->arg("year"), targetDate.year);
            }

            if ((true == isSuccess) &&
                (true == request->hasArg("plural")))
            {
                targetDayDescription.plural = request->arg("plural");
            }

            if ((true == isSuccess) &&
                (true == request->hasArg("singular")))
            {
                targetDayDescription.singular = request->arg("singular");
            }

            if (false == isSuccess)
            {
                JsonObject errorObj = jsonDoc.createNestedObject("error");

                /* Prepare response */
                jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
                errorObj["msg"]     = "Invalid arguments.";
                httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
            }
            else
            {
                setTargetDate(targetDate);
                setTargetDayDescription(targetDayDescription);

                /* Prepare response */
                (void)jsonDoc.createNestedObject("data");
                jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);
                httpStatusCode      = HttpStatus::STATUS_CODE_OK;
            }
        }
    }
    else
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }


    usageInPercent = (100U * jsonDoc.memoryUsage()) / jsonDoc.capacity();
    if (MAX_USAGE < usageInPercent)
    {
        LOG_WARNING("JSON document uses %u%% of capacity.", usageInPercent);
    }

    (void)serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

bool CountdownPlugin::saveConfiguration()
{
    bool                status                  = true;
    JsonFile            jsonFile(SPIFFS);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    jsonDoc["day"]                  = m_targetDate.day;
    jsonDoc["month"]                = m_targetDate.month;
    jsonDoc["year"]                 = m_targetDate.year;
    jsonDoc["descriptionPlural"]    = m_targetDateInformation.plural;
    jsonDoc["descriptionSingular"]  = m_targetDateInformation.singular;
    
    if (false == jsonFile.save(m_configurationFilename, jsonDoc))
    {
        LOG_WARNING("Failed to save file %s.", m_configurationFilename.c_str());
        status = false;
    }
    else
    {
        LOG_INFO("File %s saved.", m_configurationFilename.c_str());
    }

    return status;
}

bool CountdownPlugin::loadConfiguration()
{
    bool                status                  = true;
    JsonFile            jsonFile(SPIFFS);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (false == jsonFile.load(m_configurationFilename, jsonDoc))
    {
        LOG_WARNING("Failed to load file %s.", m_configurationFilename.c_str());
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

void CountdownPlugin::createConfigDirectory()
{
    if (false == SPIFFS.exists(CONFIG_PATH))
    {
        if (false == SPIFFS.mkdir(CONFIG_PATH))
        {
            LOG_WARNING("Couldn't create directory: %s", CONFIG_PATH);
        }
    }
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

            snprintf(remaining, sizeof(remaining), " %d", numberOfDays);
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

void CountdownPlugin::lock() const
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreTakeRecursive(m_xMutex, portMAX_DELAY);
    }

    return;
}

void CountdownPlugin::unlock() const
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreGiveRecursive(m_xMutex);
    }

    return;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

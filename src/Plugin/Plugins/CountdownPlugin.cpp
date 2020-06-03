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
 * @brief  Countdown plugin
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "CountdownPlugin.h"
#include "ClockDrv.h"
#include "Logging.h"

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

#include <ArduinoJson.h>
#include <Logging.h>
#include <SPIFFS.h>
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
/**
 * Counts the number of leap years.
 */
uint16_t countLeapYears(CountdownPlugin::DateDMY date);

/**
 * Convert a given date in days starting from year 0.;
 */
uint32_t dateToDays(CountdownPlugin::DateDMY date);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/* Initialize image path. */
const char* CountdownPlugin::IMAGE_PATH     = "/images/countdown.bmp";

/* Initialize configuration path. */
const char* CountdownPlugin::CONFIG_PATH    = "/configuration/";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void CountdownPlugin::active(IGfx& gfx)
{
    m_isConfigured = loadOrGenerateConfigFile();

    if (nullptr == m_iconCanvas)
    {
        m_iconCanvas = new Canvas(ICON_WIDTH, ICON_HEIGHT, 0, 0);

        if (nullptr != m_iconCanvas)
        {
            m_iconCanvas->addWidget(m_bitmapWidget);

            /* Load  icon from filesystem. */
            (void)m_bitmapWidget.load("/images/countdown.bmp");
            gfx.fillScreen(ColorDef::convert888To565(ColorDef::BLACK));

            m_iconCanvas->update(gfx);
        }
    }

    if (nullptr == m_textCanvas)
    {
        m_textCanvas = new Canvas(gfx.width() - ICON_WIDTH, gfx.height(), ICON_WIDTH, 0);

        if (nullptr != m_textCanvas)
        {
            m_textCanvas->addWidget(m_textWidget);
            setText("\\calign?");
         
            m_textCanvas->update(gfx);
        }
    }
    
    calculateDifferenceInDays();

    return;
}

void CountdownPlugin::inactive()
{
   m_isConfigured = false;
   m_remainingDays = "";

    return;
}

void CountdownPlugin::update(IGfx& gfx)
{
    if (false != m_isUpdateAvailable)
    {
        gfx.fillScreen(ColorDef::convert888To565(ColorDef::BLACK));

        if (nullptr != m_iconCanvas)
        {
            m_iconCanvas->update(gfx);
        }

        if (nullptr != m_textCanvas)
        {
            m_textCanvas->update(gfx);
        }

        m_isUpdateAvailable = true;
    }

    return;
}

void CountdownPlugin::setText(const String& formatText)
{
    m_textWidget.setFormatStr(formatText);

    return;
}

void CountdownPlugin::start()
{
    String configPath = CONFIG_PATH;

    m_configurationFilename = configPath + getUID() + ".json";

    if (false == loadOrGenerateConfigFile())
    {
        LOG_WARNING("Error on loading/generating plugin configfile: %d");
        m_isConfigured = false;
    }
    else
    {
        LOG_ERROR("Config loaded");
        m_isConfigured = true;
    }

    return;
}

void CountdownPlugin::stop() 
{
    if (false != SPIFFS.remove(m_configurationFilename))
    {
        LOG_INFO("File %s removed", m_configurationFilename.c_str());
    }

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool CountdownPlugin::loadOrGenerateConfigFile()
{
    bool status = true;
    const size_t        JSON_DOC_SIZE   = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    const size_t        MAX_USAGE       = 80U;
    size_t              usageInPercent  = (100U * jsonDoc.memoryUsage()) / jsonDoc.capacity();

    /* Check if the plugin has already created it's configuration file in the filesystem.*/
    if (false == SPIFFS.exists(m_configurationFilename))
    {
        LOG_WARNING("File %s doesn't exists.", m_configurationFilename.c_str());

        /* If not we are on the very first installation of the plugin
           First we create the directory. */
        if (false == SPIFFS.mkdir(m_configurationFilename))
        {
            LOG_WARNING("Couldn't create directory: %s", m_configurationFilename);
            status = false;
        }
        else
        {
            /* And afterwards the plugin(UID)specific configuration file with default configuration values. */
            m_fd = SPIFFS.open(m_configurationFilename, "w");

            jsonDoc["day"] = 0u;
            jsonDoc["month"] = 0u;
            jsonDoc["year"] = 0u;
            jsonDoc["description_pl"] = " DAYS";
            jsonDoc["description_sg"] = " DAY";
            serializeJson(jsonDoc, m_fd);

            m_fd.close();

            LOG_INFO("File %s created", m_configurationFilename.c_str());
        }
    }
    else
    {
        m_fd = SPIFFS.open(m_configurationFilename, "r");

        if (false == m_fd)
        {
            LOG_WARNING("Failed to open file %s.", m_configurationFilename.c_str());
            status = false;
        }
        else
        {
            String file_content = m_fd.readString();

            deserializeJson(jsonDoc, file_content);
            JsonObject obj = jsonDoc.as<JsonObject>();
            
            m_targetDate.day = obj["day"];
            m_targetDate.month = obj["month"];
            m_targetDate.year = obj["year"];
            String description_pl = obj["description_pl"];
            String description_sg = obj["description_sg"];
            m_targetDateInformation.plural = description_pl;
            m_targetDateInformation.singular = description_sg;
            
            m_fd.close();
            LOG_ERROR("Config loaded");
        }
    }

    if (MAX_USAGE < usageInPercent)
    {
        LOG_WARNING("JSON document uses %u%% of capacity.", usageInPercent);
    }

    return status;
}

void CountdownPlugin::calculateDifferenceInDays()
{
    tm currentTime;
    uint32_t currentDateInDays = 0u;
    uint32_t targetDateInDays = 0u;
    int32_t numberOfDays = 0;

    if ((false != m_isConfigured) && (false != ClockDrv::getInstance().getTime(&currentTime)))
    {
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
            char remaining[10] ={""};
            snprintf(remaining, sizeof(remaining), " %d", numberOfDays);
            m_remainingDays += remaining;

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
        
        setText(m_remainingDays);

        m_isUpdateAvailable = true;
    }
}
/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
uint16_t countLeapYears(CountdownPlugin::DateDMY date)
{ 
    uint16_t years = date.year; 
    /** Check if the current year needs to be considered 
     * for the count of leap years or not */
    if (date.month <= 2u) 
        years--; 
    /** An year is a leap year if it is a multiple of 4,
     *  multiple of 400 and not a multiple of 100. */
    return years / 4u - years / 100u + years / 400u; 
} 

uint32_t dateToDays(CountdownPlugin::DateDMY date)
{
    const uint8_t monthDays[12] = {31u, 28u, 31u, 30u, 31u, 30u, 31u, 31u, 30u, 31u, 30u, 31u}; 
    uint32_t dateInDays = 0u;

    dateInDays = date.year * 365u + date.day;
    
    for (uint8_t i=0u; i< date.month - 1u; i++) 
    {
        dateInDays += monthDays[i]; 
    }
    
    dateInDays += countLeapYears(date);

    return dateInDays;
}
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
 * @brief  Reads temperature and humidity from a sensor and displays it
 * @author Flavio Curti <fcu-github@no-way.org>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "TempHumidPlugin.h"
#include "FileSystem.h"
#include "SensorDataProvider.h"

#include <Board.h>
#include <SensorChannelType.hpp>
#include <YAColor.h>
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

/* Initialize image path for temperature (scale) icon. */
const char* TempHumidPlugin::IMAGE_PATH_TEMP_ICON      = "/images/temp.bmp";

/* Initialize image path for humidity (drop) icon. */
const char* TempHumidPlugin::IMAGE_PATH_HUMID_ICON     = "/images/drop.bmp";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void TempHumidPlugin::setSlot(const ISlotPlugin* slotInterf)
{
    m_slotInterf = slotInterf;
    return;
}

void TempHumidPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if (false == m_isInitialized)
    {
        SensorDataProvider&         sensorDataProv  = SensorDataProvider::getInstance();
        uint8_t                     sensorIdx       = 0U;
        uint8_t                     channelIdx      = 0U;

        m_iconCanvas.setPosAndSize(0, 0, ICON_WIDTH, ICON_HEIGHT);
        (void)m_iconCanvas.addWidget(m_bitmapWidget);

        (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_TEMP_ICON);

        m_textCanvas.setPosAndSize(ICON_WIDTH, 0, width - ICON_WIDTH, height);
        (void)m_textCanvas.addWidget(m_textWidget);

        /* Use just the first found sensor for temperature. */
        if (true == sensorDataProv.find(sensorIdx, channelIdx, ISensorChannel::TYPE_TEMPERATURE_DEGREE_CELSIUS, ISensorChannel::DATA_TYPE_FLOAT32))
        {
            m_temperatureSensorCh = sensorDataProv.getSensor(sensorIdx)->getChannel(channelIdx);
        }

        /* Use just the first found sensor for humidity. */
        if (true == sensorDataProv.find(sensorIdx, channelIdx, ISensorChannel::TYPE_HUMIDITY_PERCENT, ISensorChannel::DATA_TYPE_FLOAT32))
        {
            m_humiditySensorCh = sensorDataProv.getSensor(sensorIdx)->getChannel(channelIdx);
        }

        m_isInitialized = true;
    }

    return;
}

void TempHumidPlugin::stop()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    /* Nothing to do. */

    return;
}

void TempHumidPlugin::process() 
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    /* Read only if update period not reached or sensor has never been read. */
    if ((false == m_sensorUpdateTimer.isTimerRunning()) ||
        (true == m_sensorUpdateTimer.isTimeout()))
    {
        if (nullptr != m_temperatureSensorCh)
        {
            if (ISensorChannel::DATA_TYPE_FLOAT32 == m_temperatureSensorCh->getDataType())
            {
                SensorChannelFloat32*   channel     = static_cast<SensorChannelFloat32*>(m_temperatureSensorCh);
                float                   temperature = channel->getValue();

                if (!isnan(temperature))
                {
                    m_temp = temperature;

                    LOG_INFO("Temperature: %0.1f °C", m_temp);
                }
            }
        }

        if (nullptr != m_humiditySensorCh)
        {
            if (ISensorChannel::DATA_TYPE_FLOAT32 == m_humiditySensorCh->getDataType())
            {
                SensorChannelFloat32*   channel     = static_cast<SensorChannelFloat32*>(m_humiditySensorCh);
                float                   humidity    = channel->getValue();

                if (!isnan(humidity))
                {
                    m_humid = humidity;

                    LOG_INFO("Humidity: %3.1f %%", m_humid);
                }
            }
        }

        m_sensorUpdateTimer.start(SENSOR_UPDATE_PERIOD);
    }
}

void TempHumidPlugin::active(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    /* Set time to show page - either 10s or slot_time / 4
     * read here because otherwise we do not get config changes during runtime in slot_time.
     */
    if (nullptr != m_slotInterf) {
        m_pageTime = m_slotInterf->getDuration() / 4U;
    }

    gfx.fillScreen(ColorDef::BLACK);
    m_iconCanvas.update(gfx);
    m_textCanvas.update(gfx);

    return;
}

void TempHumidPlugin::inactive()
{
    /* Nothing to do. */
    return;
}

void TempHumidPlugin::update(YAGfx& gfx)
{
    bool                        showPage = false;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if (false == m_timer.isTimerRunning())
    {
        m_timer.start(m_pageTime);
        showPage = true;
    }
    else if (true == m_timer.isTimeout())
    {
        /* Switch to next page */
        ++m_page;
        m_page %= PAGE_MAX;

        showPage = true;
        m_timer.restart();
    }
    else
    {
        /* Do nothing */;
    }

    if (true == showPage)
    {
        /* Clear display */
        gfx.fillScreen(ColorDef::BLACK);
        
        m_iconCanvas.update(gfx);
        m_textCanvas.update(gfx);

        switch(m_page)
        {
        case TEMPERATURE:
            (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_TEMP_ICON);

            if (nullptr == m_temperatureSensorCh)
            {
                m_textWidget.setFormatStr("\\calign-");
            }
            else
            {
                char    valueReducedPrecison[6] = { 0 };    /* Holds a value in lower precision for display. */
                String  text;

                /* Generate temperature string with reduced precision and add unit °C. */
                (void)snprintf(valueReducedPrecison, sizeof(valueReducedPrecison), (m_temp < -9.9f) ? "%.0f" : "%.1f" , m_temp);
                text  = "\\calign";
                text += valueReducedPrecison;
                text += ISensorChannel::channelTypeToUnit(m_temperatureSensorCh->getType());

                m_textWidget.setFormatStr(text);
            }
            break;

        case HUMIDITY:
            (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_HUMID_ICON);

            if (nullptr == m_humiditySensorCh)
            {
                m_textWidget.setFormatStr("\\calign-");
            }
            else
            {
                char    valueReducedPrecison[4] = { 0 };    /* Holds a value in lower precision for display. */
                String  text;

                (void)snprintf(valueReducedPrecison, sizeof(valueReducedPrecison), "%3f", m_humid);
                text  = "\\calign";
                text += valueReducedPrecison;
                text += ISensorChannel::channelTypeToUnit(m_humiditySensorCh->getType());
                
                m_textWidget.setFormatStr(text);
            }
            break;

        default:
            break;
        }
    }

    return;
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

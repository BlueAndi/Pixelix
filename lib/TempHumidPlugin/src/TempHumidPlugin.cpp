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
 * @brief  Reads temperature and humidity from a sensor and displays it
 * @author Flavio Curti <fcu-github@no-way.org>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "TempHumidPlugin.h"

#include <FileSystem.h>
#include <SensorDataProvider.h>
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

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void TempHumidPlugin::setSlot(const ISlotPlugin* slotInterf)
{
    m_slotInterf = slotInterf;
}

void TempHumidPlugin::start(uint16_t width, uint16_t height)
{
    uint8_t                     sensorIdx       = 0U;
    uint8_t                     channelIdx      = 0U;
    MutexGuard<MutexRecursive>  guard(m_mutex);
    SensorDataProvider&         sensorDataProv  = SensorDataProvider::getInstance();

    m_view.init(width, height);

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
}

void TempHumidPlugin::stop()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    /* Nothing to do. */
}

void TempHumidPlugin::process(bool isConnected) 
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    PLUGIN_NOT_USED(isConnected);

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
                    m_temperature = temperature;

                    LOG_INFO("Temperature: %0.1f °C", m_temperature);
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
                    m_humidity = humidity;

                    LOG_INFO("Humidity: %3.0f %%", m_humidity);
                }
            }
        }

        m_sensorUpdateTimer.start(SENSOR_UPDATE_PERIOD);
    }

    /* Set time to show page - either 10s or slot_time / 4
     * read here because otherwise we do not get config changes during runtime in slot_time.
     */
    if (nullptr != m_slotInterf) {
        m_pageTime = m_slotInterf->getDuration() / 4U;

        if (DEFAULT_PAGE_TIME > m_pageTime)
        {
            m_pageTime = DEFAULT_PAGE_TIME;
        }
    }
}

void TempHumidPlugin::active(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.update(gfx);
}

void TempHumidPlugin::inactive()
{
    /* Nothing to do. */
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
        switch(m_page)
        {
        case TEMPERATURE:
            handleTemperature();
            break;

        case HUMIDITY:
            handleHumidity();
            break;

        default:
            break;
        }
    }

    m_view.update(gfx);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void TempHumidPlugin::handleTemperature()
{
    m_view.loadIconByType(_TempHumidPlugin::View::ICON_TEMPERATURE);

    if (nullptr == m_temperatureSensorCh)
    {
        m_view.setFormatText("{hc}-");
    }
    else
    {
        char    valueReducedPrecison[10] = { 0 };   /* Holds a value in lower precision for display. */
        String  text;

        /* Generate temperature string with reduced precision and add unit °C. */
        (void)snprintf(valueReducedPrecison, sizeof(valueReducedPrecison), (m_temperature < -9.9F) ? "%.0f" : "%.1f" , m_temperature);
        text  = "{hc}";
        text += valueReducedPrecison;
        text += ISensorChannel::channelTypeToUnit(m_temperatureSensorCh->getType());

        m_view.setFormatText(text);
    }
}

void TempHumidPlugin::handleHumidity()
{
    m_view.loadIconByType(_TempHumidPlugin::View::ICON_HUMIDITY);

    if (nullptr == m_humiditySensorCh)
    {
        m_view.setFormatText("{hc}-");
    }
    else
    {
        char    valueReducedPrecison[10] = { 0 };   /* Holds a value in lower precision for display. */
        String  text;

        (void)snprintf(valueReducedPrecison, sizeof(valueReducedPrecison), "%3.0f", m_humidity);
        text  = "{hc}";
        text += valueReducedPrecison;
        text += ISensorChannel::channelTypeToUnit(m_humiditySensorCh->getType());
        
        m_view.setFormatText(text);
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

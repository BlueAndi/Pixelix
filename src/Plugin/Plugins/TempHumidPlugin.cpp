/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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

#include <Board.h>
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
    lock();

    if (nullptr == m_iconCanvas)
    {
        m_iconCanvas = new Canvas(ICON_WIDTH, ICON_HEIGHT, 0, 0);

        if (nullptr != m_iconCanvas)
        {
            (void)m_iconCanvas->addWidget(m_bitmapWidget);

            /* Load icon from filesystem. */
            (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_TEMP_ICON);
        }
    }

    if (nullptr == m_textCanvas)
    {
        m_textCanvas = new Canvas(width - ICON_WIDTH, width, ICON_WIDTH, 0);

        if (nullptr != m_textCanvas)
        {
            (void)m_textCanvas->addWidget(m_textWidget);
        }
    }

    m_dht.setup(Board::Pin::dhtInPinNo, DHTTYPE);

    unlock();

    return;
}

void TempHumidPlugin::stop()
{
    lock();

    if (nullptr != m_iconCanvas)
    {
        delete m_iconCanvas;
        m_iconCanvas = nullptr;
    }

    if (nullptr != m_textCanvas)
    {
        delete m_textCanvas;
        m_textCanvas = nullptr;
    }

    unlock();

    return;
}

void TempHumidPlugin::process() 
{
    lock();

    /* Read only if update period not reached or sensor has never been read. */
    if ((false == m_sensorUpdateTimer.isTimerRunning()) ||
        (true == m_sensorUpdateTimer.isTimeout()))
    {
        float   humidity    = m_dht.getHumidity();
        float   temperature = m_dht.getTemperature();

        /* Only accept if both values could be read. */
        if ( (!isnan(humidity)) && (!isnan(temperature)) ) 
        {
            m_humid = humidity;
            m_temp  = temperature;

            LOG_INFO("Got new temp. h: %f, t: %f", m_humid, m_temp);

            m_sensorUpdateTimer.start(SENSOR_UPDATE_PERIOD);
        }
    }

    unlock();
}

void TempHumidPlugin::active(YAGfx& gfx)
{
    lock();

    /* Set time to show page - either 10s or slot_time / 4
     * read here because otherwise we do not get config changes during runtime in slot_time.
     */
    if (nullptr != m_slotInterf) {
        m_pageTime = m_slotInterf->getDuration() / 4U;
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

void TempHumidPlugin::inactive()
{
    /* Nothing to do. */
    return;
}

void TempHumidPlugin::update(YAGfx& gfx)
{
    bool showPage                   = false;
    char valueReducedPrecison[6]    = { 0 };    /* Holds a value in lower precision for display. */

    lock();

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

        if (nullptr != m_iconCanvas)
        {
            m_iconCanvas->update(gfx);
        }

        if (nullptr != m_textCanvas) 
        {
            m_textCanvas->update(gfx);
        }

        switch(m_page)
        {
        case TEMPERATURE:
            (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_TEMP_ICON);
            /* Generate temperature string with reduced precision and add unit °C. */
            (void)snprintf(valueReducedPrecison, sizeof(valueReducedPrecison), (m_temp < -9.9f) ? "%.0f" : "%.1f" , m_temp);
            m_text  = "\\calign";
            m_text += valueReducedPrecison;
            m_text += "\x8E";
            m_text += "C";
            m_textWidget.setFormatStr(m_text);
            break;

        case HUMIDITY:
            (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_HUMID_ICON);
            (void)snprintf(valueReducedPrecison, sizeof(valueReducedPrecison), "%3f", m_humid);
            m_text = valueReducedPrecison;
            m_text += "%";
            m_textWidget.setFormatStr(m_text);
            break;

        default:
            break;
        }
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

void TempHumidPlugin::lock() const
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreTakeRecursive(m_xMutex, portMAX_DELAY);
    }

    return;
}

void TempHumidPlugin::unlock() const
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

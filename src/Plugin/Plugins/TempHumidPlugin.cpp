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
#include <Color.h>
#include <DHTesp.h>
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

void TempHumidPlugin::active(IGfx& gfx)
{
    lock();

    /* 
    set time to show page - either 10s or slot_time / 4
    read here because otherwise we do not get config changes during runtime in slot_time 
    */

    if (nullptr != m_slotInterf) {
        m_pageTime = m_slotInterf->getDuration() / 4U;
    }

    gfx.fillScreen(ColorDef::BLACK);

    if (nullptr == m_iconCanvas)
    {
        m_iconCanvas = new Canvas(ICON_WIDTH, ICON_HEIGHT, 0, 0);

        if (nullptr != m_iconCanvas)
        {
            (void)m_iconCanvas->addWidget(m_bitmapWidget);

            /* Load icon from filesystem. */
            (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_TEMP_ICON);

            m_iconCanvas->update(gfx);
        }
    }
    else
    {
        m_iconCanvas->update(gfx);
    }

    if (nullptr == m_textCanvas)
    {
        m_textCanvas = new Canvas(gfx.getWidth() - ICON_WIDTH, gfx.getHeight(), ICON_WIDTH, 0);

        if (nullptr != m_textCanvas)
        {
            (void)m_textCanvas->addWidget(m_textWidget);

            m_textCanvas->update(gfx);
        }
    }
    else
    {
        m_textCanvas->update(gfx);
    }

    unlock();

    return;
}

void TempHumidPlugin::update(IGfx& gfx)
{
    bool showPage    = false;
    char valueReducedPrecison[6]      =  { 0 };  /* holds a value in lower precision for display */

    if (false == m_timer.isTimerRunning())
    {
        m_timer.start(m_pageTime);
        showPage = true;
    }
    else if (true == m_timer.isTimeout())
    {
        /* switch to next page */
        ++m_page;
        m_page %= PAGE_MAX;

        showPage = true;
        m_timer.restart();
    }
    else
    {
        /* do nothing */;
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

    return;
}

void TempHumidPlugin::process() 
{
    unsigned long msSinceInit=millis();
    float humidity;
    float temperature;

    /* read only if update_period not reached or sensor has never been read */
    if ( ( msSinceInit > ( m_last + SENSOR_UPDATE_PERIOD ) )  || ( 0U == m_last ) )
    {
            humidity = m_dht.getTemperature();
            temperature = m_dht.getHumidity();
            /* only accept if both values could be read */
            if ( !isnan(humidity) && !isnan(temperature) ) 
            {
                m_humid = humidity;
                m_temp  = temperature;
                m_last = msSinceInit;
                LOG_INFO("got new temp %lu h: %f, t: %f", m_last, m_humid, m_temp);
            }
    }
}

void TempHumidPlugin::start()
{
    m_dht.setup(Board::Pin::dhtInPinNo, DHTTYPE);
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

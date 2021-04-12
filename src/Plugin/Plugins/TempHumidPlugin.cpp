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
/* Initialize image path for standard icon. */
const char* TempHumidPlugin::IMAGE_PATH_TEMP_ICON      = "/images/temp.bmp";

/* Initialize image path for "stop" icon. */
const char* TempHumidPlugin::IMAGE_PATH_HUMID_ICON     = "/images/drop.bmp";

/******************************************************************************
 * Public Methods
 *****************************************************************************/
void TempHumidPlugin::active(IGfx& gfx)
{
    lock();

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
    const uint32_t  PERIOD      = 10000U; // show value page for 10 seconds
    bool            showPage    = false;
    char            tmp[6]      = "";     // hold the display value

    if (false == m_timer.isTimerRunning())
    {
        m_timer.start(PERIOD);
        showPage = true;
    }
    else if (true == m_timer.isTimeout())
    {
        ++m_page;
        m_page %= 2U;

        showPage = true;
        m_timer.restart();
    }
    else
    {
        ;
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
        case 0U: // screen 0 ist temperatur
            m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_TEMP_ICON);
            snprintf(tmp, sizeof(tmp), "%3f", m_temp);
            m_text = tmp;
            m_text += "°C";
            LOG_INFO("p0: new string %s", m_text);
            m_textWidget.setFormatStr(m_text);
            break;

        case 1U: // screen 1 ist Humidity
            m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_HUMID_ICON);
            snprintf(tmp, sizeof(tmp), "%3f", m_humid);
            m_text = tmp;
            m_text += "%";
            LOG_INFO("p1: new string %s", m_text);
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
    unsigned long m=millis();
    float h;
    float t;

   /* read only if update_period not reached or sensor has never been read */
   if ( m_last + SENSOR_UPDATE_PERIOD < m  || m_last == 0)
   {
        h = m_dht.getTemperature();
        t = m_dht.getHumidity();
        // only accept if both values could be read
        if (!isnan(h) && !isnan(t)) {
            m_humid = h;
            m_temp  = t;
            m_last = m;
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

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
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
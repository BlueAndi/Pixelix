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
 * @brief  WiFi status plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WifiStatusPlugin.h"
#include "WiFiUtil.h"
#include "WiFiSTA.h"

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

void WifiStatusPlugin::active(IGfx& gfx)
{
    gfx.fillScreen(ColorDef::BLACK);

    if (nullptr == m_dsp)
    {
        m_dsp           = new Canvas(gfx.getWidth(), WIFI_ICON_HEIGHT, 0, 0);
        m_iconCanvas    = new Canvas(WIFI_ICON_WIDTH, WIFI_ICON_HEIGHT, 0, 0, true);
        m_textCanvas    = new Canvas(gfx.getWidth() - WIFI_ICON_WIDTH - 1U, WIFI_ICON_HEIGHT, WIFI_ICON_WIDTH + 1, 0);

        if ((nullptr == m_dsp) ||
            (nullptr == m_iconCanvas) ||
            (nullptr == m_textCanvas))
        {
            if (nullptr != m_dsp)
            {
                delete m_dsp;
                m_dsp = nullptr;
            }

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
        }
        else
        {
            (void)m_dsp->addWidget(*m_iconCanvas);
            (void)m_dsp->addWidget(*m_textCanvas);

            (void)m_iconCanvas->addWidget(m_alertWidget);
            (void)m_textCanvas->addWidget(m_textWidget);

            m_alertWidget.move(0, 1);
            m_alertWidget.setFormatStr("");
            m_alertWidget.setTextColor(ColorDef::ORANGE);

            m_textWidget.move(0, 1);
            m_textWidget.setFormatStr("\\calignWiFi");
        }
    }

    /* Force update of the status information */
    m_timer.start(0U);

    return;
}

void WifiStatusPlugin::inactive()
{
    m_timer.stop();

    return;
}

void WifiStatusPlugin::update(IGfx& gfx)
{
    if (nullptr != m_dsp)
    {
        if ((true == m_timer.isTimerRunning()) &&
            (true == m_timer.isTimeout()))
        {
            int8_t      rssi                = -100; // dbm
            uint8_t     quality             = 0U;   // percent
            wl_status_t connectionStatus    = WiFi.status();

            /* Only in station mode it makes sense to retrieve the RSSI.
             * Otherwise keep it -100 dbm.
             */
            if (WIFI_MODE_STA == WiFi.getMode())
            {
                rssi = WiFi.RSSI();
            }

            quality = WiFiUtil::getSignalQuality(rssi);

            gfx.fillScreen(ColorDef::BLACK);

            if (WL_CONNECTED != connectionStatus)
            {
                if (false == m_toggle)
                {
                    m_alertWidget.setFormatStr("");
                    m_toggle = true;
                }
                else
                {
                    m_alertWidget.setFormatStr("\\calign!");
                    m_toggle = false;
                }
            }
            else
            {
                m_alertWidget.setFormatStr("");
                m_toggle = true;
            }

            updateWifiStatus(quality);
            m_dsp->update(gfx);

            /* Restart period */
            m_timer.start(PERIOD);
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

void WifiStatusPlugin::updateWifiStatus(uint8_t quality)
{
    uint8_t index = 0U;

    m_iconCanvas->fillScreen(ColorDef::BLACK);

    /* Draw signal strength bar steps:
     *          ##
     *       ## ##
     *    ## ## ##
     * ## ## ## ##
     */
    for(index = 0U; index < WIFI_BARS; ++index)
    {
        uint8_t     qualityRangeMin = (100U / WIFI_BARS) * index;
        int16_t     height          = WIFI_BAR_HEIGHT * (index + 1U);
        int16_t     x               = (WIFI_BAR_WIDTH * index) + (index * WIFI_BAR_SPACE_WIDTH);
        int16_t     y               = (WIFI_BARS - index - 1) * WIFI_BAR_HEIGHT;
        Color       color           = ColorDef::BLACK;

        if (qualityRangeMin < quality)
        {
            color = ColorDef::GREEN;
        }
        else
        {
            color = ColorDef::GRAY;
        }

        m_iconCanvas->fillRect(x, y, WIFI_BAR_WIDTH, height, color);
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

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
 * @brief  WiFi status plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WifiStatusPlugin.h"
#include "WiFiUtil.h"
#include "WiFi.h"

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

void WifiStatusPlugin::start(uint16_t width, uint16_t height)
{
    PLUGIN_NOT_USED(height);

    m_iconCanvas.setPosAndSize(0, 0, WIFI_ICON_WIDTH, WIFI_ICON_HEIGHT);
    (void)m_iconCanvas.addWidget(m_alertWidget);

    m_textCanvas.setPosAndSize(WIFI_ICON_WIDTH + 1, 0, width - WIFI_ICON_WIDTH - 1U, WIFI_ICON_HEIGHT);
    (void)m_textCanvas.addWidget(m_textWidget);

    m_alertWidget.move(0, 1);
    m_alertWidget.setFormatStr("");
    m_alertWidget.setTextColor(ColorDef::ORANGE);

    m_textWidget.move(0, 1);
    m_textWidget.setFormatStr("\\calignWiFi");
}

void WifiStatusPlugin::stop()
{
    /* Nothing to do. */
}

void WifiStatusPlugin::active(YAGfx& gfx)
{
    gfx.fillScreen(ColorDef::BLACK);

    /* Force update of the status information */
    m_timer.start(0U);
}

void WifiStatusPlugin::inactive()
{
    m_timer.stop();
}

void WifiStatusPlugin::update(YAGfx& gfx)
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

        gfx.fillScreen(ColorDef::BLACK);
        updateWifiStatus(gfx, quality);
        m_iconCanvas.update(gfx);
        m_textCanvas.update(gfx);

        /* Restart period */
        m_timer.start(PERIOD);
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void WifiStatusPlugin::updateWifiStatus(YAGfx& gfx, uint8_t quality)
{
    uint8_t index = 0U;

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

        gfx.fillRect(x, y, WIFI_BAR_WIDTH, height, color);
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

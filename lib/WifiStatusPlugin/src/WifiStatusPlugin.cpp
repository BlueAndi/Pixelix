/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
    m_view.init(width, height);
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

        /* Disconnected? */
        if (WL_CONNECTED != connectionStatus)
        {
            if (false == m_toggle)
            {
                m_view.setFormatText("");
                m_toggle = true;
            }
            else
            {
                m_view.setFormatText("{#FFA500}{hc}!");
                m_toggle = false;
            }
        }
        else
        {
            m_view.setFormatText("{hc}WiFi");
            m_toggle = true;
        }

        m_view.updateWifiStatus(quality);
        m_view.update(gfx);

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

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

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
 * @brief  System state: Connecting
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ConnectingState.h"
#include "SysMsg.h"
#include "Services.h"
#include "SensorDataProvider.h"

#include "IdleState.h"
#include "ConnectedState.h"
#include "ErrorState.h"

#include <WiFi.h>
#include <Logging.h>
#include <Util.h>
#include <SettingsService.h>

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

void ConnectingState::entry(StateMachine& sm)
{
    SettingsService& settings = SettingsService::getInstance();

    /* Are remote wifi network informations available? */
    if (true == settings.open(true))
    {
        m_wifiSSID          = settings.getWifiSSID().getValue();
        m_wifiPassphrase    = settings.getWifiPassphrase().getValue();
        m_isQuiet           = settings.getQuietMode().getValue();

        settings.close();
    }
    else
    {
        m_isQuiet = settings.getQuietMode().getDefault();
    }

    /* No remote wifi network informations available? */
    if ((0 == m_wifiSSID.length()) ||
        (0 == m_wifiPassphrase.length()))
    {
        String infoStr = "Keep button pressed and reboot. Set SSID/password via webserver.";

        LOG_INFO(infoStr);
        SysMsg::getInstance().show(infoStr);

        sm.setState(IdleState::getInstance());
    }

    /* Disable retry mechanism. */
    m_retryTimer.stop();

    /* Disable automatic reconnect, so we are able to handle the
     * reconnect behaviour by ourself.
     */
    if (false == WiFi.setAutoReconnect(false))
    {
        String errorStr = "Set autom. reconnect failed.";

        /* Fatal error */
        LOG_FATAL(errorStr);
        SysMsg::getInstance().show(errorStr);

        sm.setState(ErrorState::getInstance());
    }
}

void ConnectingState::process(StateMachine& sm)
{
    /* No retry mechanism is running? */
    if (false == m_retryTimer.isTimerRunning())
    {
        const uint32_t  DURATION_NON_SCROLLING  = 2000U; /* ms */
        const uint32_t  SCROLLING_REPEAT_NUM    = 1U;
        wl_status_t status      = WL_IDLE_STATUS;
        String      infoStr     = "Connecting to ";

        infoStr += m_wifiSSID;
        infoStr += ".";

        LOG_INFO(infoStr);

        if (false == m_isQuiet)
        {
            SysMsg::getInstance().show(infoStr, DURATION_NON_SCROLLING, SCROLLING_REPEAT_NUM);
        }

        /* Remote wifi network informations are available, try to establish a connection. */
        status = WiFi.begin(m_wifiSSID.c_str(), m_wifiPassphrase.c_str());

        /* Connection establishment pending? */
        if (WL_CONNECTED != status)
        {
            /* Wait a little bit, until retry. */
            m_retryTimer.start(RETRY_DELAY);
        }
        /* Connected */
        else
        {
            /* Disable retry mechanism. */
            m_retryTimer.stop();

            sm.setState(ConnectedState::getInstance());
        }
    }
    /* Retry mechanism is active. */
    else
    {
        /* Retry delay timeout? */
        if (true == m_retryTimer.isTimeout())
        {
            /* Disable retry mechanism. */
            m_retryTimer.stop();
        }
        /* Connection successful established? */
        else if (true == WiFi.isConnected())
        {
            /* Disable retry mechanism. */
            m_retryTimer.stop();

            sm.setState(ConnectedState::getInstance());
        }
        else
        {
            /* Wait ... */
            ;
        }
    }

    Services::processAll();
    SensorDataProvider::getInstance().process();
}

void ConnectingState::exit(StateMachine& sm)
{
    UTIL_NOT_USED(sm);

    /* Nothing to do. */
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

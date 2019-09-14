/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
#include "Settings.h"
#include "DisplayMgr.h"
#include "IdleState.h"
#include "ConnectedState.h"

#include <WiFi.h>
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

/* Connecting state instance */
ConnectingState ConnectingState::m_instance;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void ConnectingState::entry(StateMachine& sm)
{
    /* Are remote wifi network informations available? */
    if (true == Settings::getInstance().open(true))
    {
        m_wifiSSID          = Settings::getInstance().getWifiSSID();
        m_wifiPassphrase    = Settings::getInstance().getWifiPassphrase();

        Settings::getInstance().close();
    }

    /* No remote wifi network informations available? */
    if ((0 == m_wifiSSID.length()) ||
        (0 == m_wifiPassphrase.length()))
    {
        String infoStr = "Keep button pressed and reboot. Set SSID/password via webserer.";

        LOG_INFO(infoStr);
        DisplayMgr::getInstance().showSysMsg(infoStr);

        sm.setState(IdleState::getInstance());
    }

    /* Disable retry mechanism. */
    m_retryTimer.stop();

    /* Disable automatic reconnect, so we are able to handle the
     * reconnect behaviour by ourself.
     */
    WiFi.setAutoReconnect(false);

    return;
}

void ConnectingState::process(StateMachine& sm)
{
    /* No retry mechanism is running? */
    if (false == m_retryTimer.isTimerRunning())
    {
        wl_status_t status      = WL_IDLE_STATUS;
        String      infoStr     = "Connecting to ";
        String      errorStr    = "Connection failed.";

        infoStr += m_wifiSSID;

        LOG_INFO(infoStr);
        DisplayMgr::getInstance().showSysMsg(infoStr);

        /* Remote wifi network informations are available, try to establish a connection. */
        status = WiFi.begin(m_wifiSSID.c_str(), m_wifiPassphrase.c_str());

        /* Connection establishment failed? */
        if (WL_CONNECTED != status)
        {
            LOG_ERROR(errorStr);
            DisplayMgr::getInstance().showSysMsg(errorStr);

            /* Wait a little bit, until retry. */
            m_retryTimer.start(RETRY_DELAY);

            /* TODO How to determine whats wrong? SSID or password? */
        }
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
            m_retryTimer.stop();
        }
    }

    return;
}

void ConnectingState::exit(StateMachine& sm)
{
    /* Nothing to do. */
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

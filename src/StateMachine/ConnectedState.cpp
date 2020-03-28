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
 * @brief  System state: Connected
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ConnectedState.h"
#include "SysMsg.h"
#include "UpdateMgr.h"
#include "MyWebServer.h"
#include "Settings.h"
#include "WebConfig.h"
#include "ClockDrv.h"

#include "ConnectingState.h"
#include "RestartState.h"
#include "ErrorState.h"

#include <Arduino.h>
#include <WiFi.h>
#include <Logging.h>
#include <ESPmDNS.h>

#include "AsyncHttpClient.h"

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

/* Connected state instance */
ConnectedState  ConnectedState::m_instance;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void ConnectedState::entry(StateMachine& sm)
{
    String infoStr = "Hostname: ";
    String hostname;
    String infoStringIp = "IP: ";

    LOG_INFO("Connected.");

    /* Get hostname. */
    if (false == Settings::getInstance().open(true))
    {
        LOG_WARNING("Use default hostname.");
        hostname = Settings::getInstance().getHostname().getDefault();
    }
    else
    {
        hostname = Settings::getInstance().getHostname().getValue();
        Settings::getInstance().close();
    }

    /* Set hostname. Note, wifi must be connected somehow. */
    if (false == WiFi.setHostname(hostname.c_str()))
    {
        String errorStr = "Can't set AP hostname.";

        /* Fatal error */
        LOG_FATAL(errorStr);
        SysMsg::getInstance().show(errorStr);

        sm.setState(ErrorState::getInstance());
    }
    /* Enable mDNS */
    else if (false == MDNS.begin(hostname.c_str()))
    {
        String errorStr = "Failed to setup mDNS.";

        /* Fatal error */
        LOG_FATAL(errorStr);
        SysMsg::getInstance().show(errorStr);

        sm.setState(ErrorState::getInstance());
    }
    else
    {
        /* Start the ClockDriver */
        ClockDrv::getInstance().init();

        /* Start webserver after a wifi connection is established.
         * If its done earlier, it will cause an exception.
         */
        MyWebServer::begin();

        /* Start over-the-air update server. */
        UpdateMgr::getInstance().begin();

        /* Add MDNS services */
        MDNS.enableArduino(WebConfig::ARDUINO_OTA_PORT, true); /* This typically set by ArduinoOTA, but is disabled there. */
        MDNS.addService("http", "tcp", WebConfig::WEBSERVER_PORT);

        /* Show hostname and IP. */
        infoStr += WiFi.getHostname(); /* Don't believe its the same as set before. */
        infoStr += " IP: ";
        infoStr += WiFi.localIP().toString();
        SysMsg::getInstance().show(infoStr, infoStr.length() * 600U);

        LOG_INFO(infoStr);

        {
            static AsyncHttpClient  client;

            if (false == client.begin("http://www.google.de"))
            {
                LOG_ERROR("AsyncHttpClient::begin() failed.");
            }
            else if (false == client.GET())
            {
                LOG_ERROR("AsyncHttpClient::sendRequest() failed.");
            }
            else
            {
                ;
            }
        }
    }

    return;
}

void ConnectedState::process(StateMachine& sm)
{
    /* Handle update, there may be one in the background. */
    UpdateMgr::getInstance().process();

    /* Restart requested by update manager? This may happen after a successful received
     * new firmware or filesystem binary.
     */
    if (true == UpdateMgr::getInstance().isRestartRequested())
    {
        sm.setState(RestartState::getInstance());
    }
    /* Connection lost? */
    else if (false == WiFi.isConnected())
    {
        LOG_INFO("Connection lost.");

        sm.setState(ConnectingState::getInstance());
    }

    return;
}

void ConnectedState::exit(StateMachine& sm)
{
    UTIL_NOT_USED(sm);

    /* Stop mDNS */
    MDNS.end();

    /* Stop webserver */
    MyWebServer::end();

    /* Stop over-the-air update server */
    UpdateMgr::getInstance().end();

    /* Disconnect all connections */
    (void)WiFi.disconnect();

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

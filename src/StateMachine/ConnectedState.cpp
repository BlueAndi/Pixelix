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
#include "DisplayMgr.h"
#include "Services.h"
#include "SensorDataProvider.h"
#include "PluginMgr.h"

#include "ConnectingState.h"
#include "RestartState.h"
#include "ErrorState.h"
#include "HttpStatus.h"

#include <Arduino.h>
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

void ConnectedState::entry(StateMachine& sm)
{
    SettingsService&    settings        = SettingsService::getInstance();
    String              infoStr         = "Hostname: ";
    String              hostname;
    String              infoStringIp    = "IP: ";
    String              notifyURL       = "-";
    bool                isQuiet         = false;

    LOG_INFO("Connected.");

    /* Get some settings. */
    if (false == settings.open(true))
    {
        LOG_WARNING("Use default hostname.");

        hostname    = settings.getHostname().getDefault();
        notifyURL   = settings.getNotifyURL().getDefault();
        isQuiet     = settings.getQuietMode().getDefault();
    }
    else
    {
        hostname    = settings.getHostname().getValue();
        notifyURL   = settings.getNotifyURL().getValue();
        isQuiet     = settings.getQuietMode().getValue();

        settings.close();
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
    else
    {
        const uint32_t  DURATION_NON_SCROLLING  = 4000U; /* ms */
        const uint32_t  SCROLLING_REPEAT_NUM    = 2U;

        /* Notify about successful network connection. */
        DisplayMgr::getInstance().setNetworkStatus(true);

        /* Show hostname and IP. */
        infoStr += WiFi.getHostname(); /* Don't believe its the same as set before. */
        infoStr += " IP: ";
        infoStr += WiFi.localIP().toString();

        LOG_INFO(infoStr);

        if (false == isQuiet)
        {
            SysMsg::getInstance().show(infoStr, DURATION_NON_SCROLLING, SCROLLING_REPEAT_NUM);
        }

        pushUrl(notifyURL);
    }
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

    Services::processAll();
    SensorDataProvider::getInstance().process();
}

void ConnectedState::exit(StateMachine& sm)
{
    UTIL_NOT_USED(sm);

    /* User requested (power off / restart after update) to disconnect? */
    if (true == WiFi.isConnected())
    {
        /* Purge sensor topics (MQTT) */
        SensorDataProvider::getInstance().end();

        /* Unregister all plugins, which will purge all of their topics (MQTT). */
        PluginMgr::getInstance().unregisterAllPluginTopics();

        /* Stop all services now to allow them having graceful disconnection from
         * servers until the wifi will be disconnected.
         */
        Services::stopAll();

        /* Disconnect wifi connection. */
        (void)WiFi.disconnect();
    }

    /* Notify about no network connection. */
    DisplayMgr::getInstance().setNetworkStatus(false);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void ConnectedState::initHttpClient()
{
    m_client.regOnResponse([](const HttpResponse& rsp){
        uint16_t statusCode = rsp.getStatusCode();

        if (HttpStatus::STATUS_CODE_OK == statusCode)
        {
            LOG_INFO("Online state reported.");
        }

    });

    m_client.regOnError([]() {
        LOG_WARNING("Connection error happened.");
   });
}

void ConnectedState::pushUrl(const String& pushUrl)
{
    /* If a push URL is set, notify about the online status. */
    if (false == pushUrl.isEmpty())
    {
        String      url         = pushUrl;
        const char* GET_CMD     = "get ";
        const char* POST_CMD    = "post ";
        bool        isGet       = true;

        /* URL prefix might indicate the kind of request. */
        url.toLowerCase();
        if (0U != url.startsWith(GET_CMD))
        {
            url = url.substring(strlen(GET_CMD));
            isGet = true;
        }
        else if (0U != url.startsWith(POST_CMD))
        {
            url = url.substring(strlen(POST_CMD));
            isGet = false;
        }
        else
        {
            ;
        }

        if (true == m_client.begin(url))
        {
            if (false == m_client.GET())
            {
                LOG_WARNING("GET %s failed.", url.c_str());
            }
            else
            {
                LOG_INFO("Notification triggered.");
            }
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

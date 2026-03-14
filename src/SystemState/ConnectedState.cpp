/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   ConnectedState.cpp
 * @brief  System state: Connected
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ConnectedState.h"
#include "SysMsg.h"
#include "RestartMgr.h"
#include "MyWebServer.h"
#include "DisplayMgr.h"
#include "Services.h"
#include "SensorDataProvider.h"
#include "MyWebServer.h"

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
    SettingsService& settings               = SettingsService::getInstance();
    String           infoStr                = "Hostname: ";
    String           infoStringIp           = " IP: ";
    String           notifyURL              = "-";
    bool             isQuiet                = false;
    const uint32_t   DURATION_NON_SCROLLING = 4000U; /* ms */
    const uint32_t   SCROLLING_REPEAT_NUM   = 2U;

    LOG_INFO("Connected.");

    /* Get some settings. */
    if (false == settings.open(true))
    {
        LOG_WARNING("Using default hostname.");

        notifyURL = settings.getNotifyURL().getDefault();
        isQuiet   = settings.getQuietMode().getDefault();
    }
    else
    {
        notifyURL = settings.getNotifyURL().getValue();
        isQuiet   = settings.getQuietMode().getValue();

        settings.close();
    }

    /* Notify about successful network connection. */
    DisplayMgr::getInstance().setNetworkStatus(true);

    /* Show hostname and IP. */
    infoStr += WiFi.getHostname();
    infoStr += infoStringIp;
    infoStr += WiFi.localIP().toString();

    LOG_INFO(infoStr);

    if (false == isQuiet)
    {
        SysMsg::getInstance().show(infoStr, DURATION_NON_SCROLLING, SCROLLING_REPEAT_NUM);
    }

    pushUrl(notifyURL);
}

void ConnectedState::process(StateMachine& sm)
{
    /* Handle webserver. */
    MyWebServer::process();

    /* Connection lost? */
    if (false == WiFi.isConnected())
    {
        LOG_INFO("Connection lost.");

        /* Notify about no network connection. */
        DisplayMgr::getInstance().setNetworkStatus(false);

        sm.setState(ConnectingState::getInstance());
    }

    Services::processAll();
    SensorDataProvider::getInstance().process();

    handlePushUrlResponse();
}

void ConnectedState::exit(StateMachine& sm)
{
    UTIL_NOT_USED(sm);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void ConnectedState::pushUrl(const String& pushUrl)
{
    /* If a push URL is set, notify about the online status. */
    if ((false == pushUrl.isEmpty()) &&
        (INVALID_HTTP_JOB_ID == m_pushJobId))
    {
        HttpService& httpService = HttpService::getInstance();
        String       url         = pushUrl;
        const char*  GET_CMD     = "get ";
        const char*  POST_CMD    = "post ";
        bool         isGet       = true;

        /* URL prefix might indicate the kind of request. */
        url.toLowerCase();
        if (true == url.startsWith(GET_CMD))
        {
            url   = url.substring(strlen(GET_CMD));
            isGet = true;
        }
        else if (true == url.startsWith(POST_CMD))
        {
            url   = url.substring(strlen(POST_CMD));
            isGet = false;
        }
        else
        {
            ;
        }

        if (false == isGet)
        {
            m_pushJobId = httpService.post(url.c_str(), nullptr, 0U);

            if (INVALID_HTTP_JOB_ID == m_pushJobId)
            {
                LOG_WARNING("POST %s failed.", url.c_str());
            }
        }
        else
        {
            m_pushJobId = httpService.get(url.c_str());

            if (INVALID_HTTP_JOB_ID == m_pushJobId)
            {
                LOG_WARNING("GET %s failed.", url.c_str());
            }
        }
    }
}

void ConnectedState::handlePushUrlResponse()
{
    if (INVALID_HTTP_JOB_ID != m_pushJobId)
    {
        HttpService& httpService = HttpService::getInstance();
        HttpRsp      response;

        if (true == httpService.getResponse(m_pushJobId, response))
        {
            if (HTTP_CODE_OK == response.statusCode)
            {
                LOG_INFO("Push URL successful.");
            }
            else
            {
                LOG_WARNING("Push URL failed, HTTP status code: %u", response.statusCode);
            }

            m_pushJobId = INVALID_HTTP_JOB_ID;
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

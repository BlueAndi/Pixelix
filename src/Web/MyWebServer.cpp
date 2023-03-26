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
 * @brief  Web server
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MyWebServer.h"
#include "WebConfig.h"
#include "CaptivePortal.h"
#include "Pages.h"
#include "RestApi.h"
#include "WebSocket.h"

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

static void error(AsyncWebServerRequest* request);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Web server */
static AsyncWebServer   gWebServer(WebConfig::WEBSERVER_PORT);

/******************************************************************************
 * Public Methods
 *****************************************************************************/

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

void MyWebServer::init(bool initCaptivePortal)
{
    if (false == initCaptivePortal)
    {
        /* Register all web pages */
        Pages::init(gWebServer);
        RestApi::init(gWebServer);

        gWebServer.onNotFound(error);

        /* Register websocket */
        WebSocketSrv::getInstance().init(gWebServer);
    }
    else
    {
        CaptivePortal::init(gWebServer);
    }
}

void MyWebServer::begin()
{
    /* Start webserver */
    gWebServer.begin();
}

void MyWebServer::end()
{
    /* Stop webserver */
    gWebServer.end();
}

AsyncWebServer& MyWebServer::getInstance()
{
    return gWebServer;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Common error handler used in case a requested path was not found.
 *
 * @param[in] request   Web request
 */
static void error(AsyncWebServerRequest* request)
{
    if (nullptr == request)
    {
        return;
    }

    /* REST request? */
    if (request->url().startsWith(RestApi::BASE_URI))
    {
        RestApi::error(request);
    }
    else
    {
        Pages::error(request);
    }

}

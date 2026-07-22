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
 * @file   MyWebServer.cpp
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

#include <MemUtil.h>
#include <Logging.h>
#include <HttpStatus.h>

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
static void rejectIfLowMemory(AsyncWebServerRequest* request, ArMiddlewareNext next);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/**
 * The web server instance.
 *
 * @return Its not a function, it's a variable! Doxygen just gets confused here.
 */
static AsyncWebServer gWebServer(WebConfig::WEBSERVER_PORT);

/** Is captive portal enabled? */
static bool gIsCaptivePortalEnabled            = false;

/**
 * Hard limit for the largest allocatable internal heap block. Below this a web
 * request is always rejected with HTTP 503 to avoid an out-of-memory situation
 * on boards without PSRAM.
 * The largest free block (not the total free heap) is used, because the heap is
 * fragmented and this is what predicts whether the next allocation succeeds.
 */
static const size_t MIN_FREE_BLOCK_FOR_REQUEST = 12U * 1024U;

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
        /* Reject requests with HTTP 503 if the internal heap is too low. */
        gWebServer.addMiddleware(rejectIfLowMemory);

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

    gIsCaptivePortalEnabled = initCaptivePortal;
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

void MyWebServer::process()
{
    if (false == gIsCaptivePortalEnabled)
    {
        WebSocketSrv::getInstance().process();
    }
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
    if (true == request->url().startsWith(RestApi::BASE_URI))
    {
        RestApi::error(request);
    }
    else
    {
        Pages::error(request);
    }
}

/**
 * Middleware which rejects a request with HTTP 503 if the internal heap is too
 * low. It runs before any handler allocates memory and asks the client to retry
 * later instead of risking an out-of-memory crash. The largest free block
 * naturally drops when an outgoing request (incl. its memory hungry TLS
 * handshake) is in flight, so this also covers that case without disturbing
 * concurrent requests while enough heap is available.
 *
 * @param[in] request   Web request
 * @param[in] next      Callback to continue the middleware chain
 */
static void rejectIfLowMemory(AsyncWebServerRequest* request, ArMiddlewareNext next)
{
    size_t largestFreeBlock = MemUtil::getLargestFreeBlockSize();

    if (MIN_FREE_BLOCK_FOR_REQUEST <= largestFreeBlock)
    {
        next();
    }
    else
    {
        AsyncWebServerResponse* response = request->beginResponse(
            HttpStatus::STATUS_CODE_SERVICE_UNAVAILABLE,
            "application/json",
            "{\"status\":\"error\",\"error\":{\"msg\":\"Service temporarily unavailable, please retry.\"}}");

        if (nullptr == response)
        {
            /* Not even a small response could be allocated. */
            request->send(HttpStatus::STATUS_CODE_SERVICE_UNAVAILABLE);
        }
        else
        {
            response->addHeader("Retry-After", "2");
            request->send(response);
        }

        LOG_WARNING("Request rejected (HTTP 503), largest free heap block: %u byte.", largestFreeBlock);
    }
}

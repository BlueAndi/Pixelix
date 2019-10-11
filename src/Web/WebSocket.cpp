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
 * @brief  Websocket server
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WebSocket.h"
#include "WebConfig.h"

#include <Logging.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Get number of array elements. */
#define ARRAY_NUM(__arr)    (sizeof(__arr) / sizeof((__arr)[0]))

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Websocket */
static AsyncWebSocket   gWebSocket(WebConfig::WEBSOCKET_PATH);

/******************************************************************************
 * Public Methods
 *****************************************************************************/

static void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

void WebSocket::init(AsyncWebServer& srv)
{
    /* Register websocket event handler */
    gWebSocket.onEvent(onEvent);

    /* Register websocket on webserver */
    srv.addHandler(&gWebSocket);

    return;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Websocket event handler.
 * 
 * @param[in] server    Websocket server
 * @param[in] client    Weboscket client
 * @param[in] type      Websocket event type
 * @param[in] arg       Websocket argument
 * @param[in] data      Websocket data
 * @param[in] len       Websocket data length in bytes
 */
static void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len)
{
    uint16_t    errorId = 0;
    const char* msg     = "-";

    if ((NULL == server) ||
        (NULL == client))
    {
        return;
    }

    switch(type)
    {
    /* Client connected */
    case WS_EVT_CONNECT:
        LOG_INFO("ws[%s][%u] client connected.", server->url(), client->id());
        client->ping();
        break;
        
    /* Client disconnected */
    case WS_EVT_DISCONNECT:
        LOG_INFO("ws[%s][%u] client disconnected.", server->url(), client->id());
        break;
        
    /* Pong received */
    case WS_EVT_PONG:

        if ((NULL != data) &&
            (0 < len))
        {
            msg = reinterpret_cast<char*>(data);
        }

        LOG_INFO("ws[%s][%u] pong: %s", server->url(), client->id(), msg);
        break;
        
    /* Remote error */
    case WS_EVT_ERROR:

        if (NULL != arg)
        {
            errorId = *static_cast<uint16_t*>(arg);
        }

        if ((NULL != data) &&
            (0 < len))
        {
            msg = reinterpret_cast<char*>(data);
        }

        LOG_INFO("ws[%s][%u] error %u: %s", server->url(), client->id(), errorId, msg);
        break;
        
    /* Data */
    case WS_EVT_DATA:
        /* Not supported yet. */
        break;

    default:
        break;
    }

    return;
}
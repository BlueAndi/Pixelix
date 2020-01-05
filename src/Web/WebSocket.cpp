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
 * @brief  Websocket server
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WebSocket.h"
#include "WsCmdGetDisp.h"

#include <Logging.h>
#include <Util.h>

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

/* Initialize the websocket server instance. */
WebSocketSrv    WebSocketSrv::m_instance;

/** Websocket get display command */
static WsCmdGetDisp gWsCmdGetDisp;

/** Websocket command list */
static WsCmd*       gWsCommands[] =
{
    &gWsCmdGetDisp
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void WebSocketSrv::init(AsyncWebServer& srv)
{
    /* Register websocket event handler */
    m_webSocket.onEvent(onEvent);

    /* HTTP Authenticate before switch to Websocket protocol */
    m_webSocket.setAuthentication(WebConfig::WEB_LOGIN_USER, WebConfig::WEB_LOGIN_PASSWORD);

    /* Register websocket on webserver */
    srv.addHandler(&m_webSocket);

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void WebSocketSrv::onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len)
{
    if ((NULL == server) ||
        (NULL == client))
    {
        return;
    }

    switch(type)
    {
    /* Client connected */
    case WS_EVT_CONNECT:
        m_instance.onConnect(server, client, reinterpret_cast<AsyncWebServerRequest*>(arg));
        break;
        
    /* Client disconnected */
    case WS_EVT_DISCONNECT:
        m_instance.onDisconnect(server, client);
        break;
        
    /* Pong received */
    case WS_EVT_PONG:
        m_instance.onPong(server, client, data, len);
        break;
        
    /* Remote error */
    case WS_EVT_ERROR:
        m_instance.onError(server, client, *reinterpret_cast<uint16_t*>(arg), reinterpret_cast<const char*>(data), len);
        break;
        
    /* Data */
    case WS_EVT_DATA:
        m_instance.onData(server, client, reinterpret_cast<AwsFrameInfo*>(arg), data, len);
        break;

    default:
        break;
    }

    return;
}

void WebSocketSrv::onConnect(AsyncWebSocket* server, AsyncWebSocketClient* client, AsyncWebServerRequest* request)
{
    UTIL_NOT_USED(request);

    LOG_INFO("ws[%s][%u] Client connected.", server->url(), client->id());
    return;
}

void WebSocketSrv::onDisconnect(AsyncWebSocket* server, AsyncWebSocketClient* client)
{
    LOG_INFO("ws[%s][%u] Client disconnected.", server->url(), client->id());
    return;
}

void WebSocketSrv::onPong(AsyncWebSocket* server, AsyncWebSocketClient* client, uint8_t* data, size_t len)
{
    if ((NULL == data) ||
        (0 == len))
    {
        LOG_INFO("ws[%s][%u] Pong: -", server->url(), client->id());
    }
    else
    {
        LOG_INFO("ws[%s][%u] Pong: %s", server->url(), client->id(), data);
    }

    return;
}

void WebSocketSrv::onError(AsyncWebSocket* server, AsyncWebSocketClient* client, uint16_t reasonCode, const char* reasonStr, size_t reasonStrLen)
{
    if ((NULL == reasonStr) ||
        (0 == reasonStrLen))
    {
        LOG_INFO("ws[%s][%u] Error %u: -", server->url(), client->id(), reasonCode);
    }
    else
    {
        LOG_INFO("ws[%s][%u] Error %u: %s", server->url(), client->id(), reasonCode, reasonStr);
    }

    return;
}

void WebSocketSrv::onData(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsFrameInfo* info, const uint8_t* data, size_t len)
{
    /* Frame info missing? */
    if (NULL == info)
    {
        LOG_ERROR("ws[%s][%u] Frame info is missing.", server->url(), client->id());
        server->close(client->id(), 0u, "Frame info is missing.");
    }
    /* No text frame? */
    else if (WS_TEXT != info->opcode)
    {
        LOG_ERROR("ws[%s][%u] Not supported message type received: %u", server->url(), client->id(), info->opcode);
        server->close(client->id(), 0u, "Not supported message type.");
    }
    /* Is the whole message in a single frame and we got all of it's data? */
    else if ((0 < info->final) &&
                (0 == info->index) &&
                (len == info->len ))
    {
        /* Empty text message? */
        if ((NULL == data) ||
            (0 == len))
        {
            LOG_WARNING("ws[%s][%u] Message: -", server->url(), client->id());
        }
        /* Handle text message */
        else
        {
            handleMsg(server, client, reinterpret_cast<const char*>(data), len);
        }
    }
    /* Message is comprised of multiple frames or the frame is split into multiple packets */
    else
    {
        LOG_ERROR("ws[%s][%u] Fragmented messages not supported.", server->url(), client->id());
        server->close(client->id(), 0u, "Not supported message type.");
    }

    return;
}

void WebSocketSrv::handleMsg(AsyncWebSocket* server, AsyncWebSocketClient* client, const char* msg, size_t msgLen)
{
    size_t      msgIndex    = 0u;
    uint8_t     index       = 0u;
    String      cmd;
    String      par;
    WsCmd*      wsCmd       = NULL;
    const char  DELIMITER   = ';';

    if ((NULL == server) ||
        (NULL == client) ||
        (NULL == msg) ||
        (0 == msgLen))
    {
        return;
    }

    /* Skip spaces and tabs in front. */
    while((msgLen > msgIndex) && ( (' ' == msg[msgIndex]) || ('\t' == msg[msgIndex]) ))
    {
        ++msgIndex;
    }

    /* Get command string */
    while((msgLen > msgIndex) && (DELIMITER != msg[msgIndex]))
    {
        cmd += msg[msgIndex];
        ++msgIndex;
    }

    /* Command string not empty? */
    if (0 < cmd.length())
    {
        /* Find command object */
        index = 0u;
        while((NULL == wsCmd) && (index < UTIL_ARRAY_NUM(gWsCommands)))
        {
            if (cmd == gWsCommands[index]->getCmd())
            {
                wsCmd = gWsCommands[index];
            }

            ++index;
        }

        /* Command not found? */
        if (NULL == wsCmd)
        {
            client->text("NACK;\"Command unknown.\"");
        }
        else
        {
            /* Determine parameters */
            if ((msgLen > msgIndex) &&
                (DELIMITER == msg[msgIndex]))
            {
                /* Overstep delimiter */
                ++msgIndex;

                while(msgLen > msgIndex)
                {
                    if (DELIMITER == msg[msgIndex])
                    {
                        wsCmd->setPar(par.c_str());
                        par.clear();
                    }
                    else
                    {
                        par += msg[msgIndex];
                    }

                    ++msgIndex;
                }

                wsCmd->setPar(par.c_str());
            }

            /* Execute command */
            wsCmd->execute(server, client);
        }
    }

    return;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

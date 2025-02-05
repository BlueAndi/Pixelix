/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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

#include "WsCmdAlias.h"
#include "WsCmdBrightness.h"
#include "WsCmdButton.h"
#include "WsCmdEffect.h"
#include "WsCmdGetDisp.h"
#include "WsCmdInstall.h"
#include "WsCmdIperf.h"
#include "WsCmdLog.h"
#include "WsCmdMove.h"
#include "WsCmdPlugins.h"
#include "WsCmdRestart.h"
#include "WsCmdSlotDuration.h"
#include "WsCmdSlot.h"
#include "WsCmdSlots.h"
#include "WsCmdUninstall.h"

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

/** Websocket get display command */
static WsCmdGetDisp         gWsCmdGetDisp;

/** Websocket slot command */
static WsCmdSlot            gWsCmdSlot;

/** Websocket slots command */
static WsCmdSlots           gWsCmdSlots;

/** Websocket plugins command */
static WsCmdPlugins         gWsCmdPlugins;

/** Websocket install command */
static WsCmdInstall         gWsCmdInstall;

/** Websocket uninstall command */
static WsCmdUninstall       gWsCmdUninstall;

/** Websocket reset command */
static WsCmdRestart         gWsCmdRestart;

/** Websocket get/set brightness command */
static WsCmdBrightness      gWsCmdBrightness;

/** Websocket log command */
static WsCmdLog             gWsCmdLog;

/** Websocket move command */
static WsCmdMove            gWsCmdMove;

/** Websocket slot duration command */
static WsCmdSlotDuration    gWsCmdSlotDuration;

#if CONFIG_FEATURE_IPERF == 1

/** Websocket iperf command */
static WsCmdIperf           gWsCmdIperf;

#endif /* CONFIG_FEATURE_IPERF == 1 */

/** Websocket control virtual button command */
static WsCmdButton          gWsCmdButton;

/** Websocket control fade effects */
static WsCmdEffect          gWsCmdEffect;

/** Websocket get/set plugin alias name command */
static WsCmdAlias           gWsCmdAlias;

/** Websocket command list */
static WsCmd*       gWsCommands[] =
{
    &gWsCmdGetDisp,
    &gWsCmdSlot,
    &gWsCmdSlots,
    &gWsCmdPlugins,
    &gWsCmdInstall,
    &gWsCmdUninstall,
    &gWsCmdRestart,
    &gWsCmdBrightness,
    &gWsCmdLog,
    &gWsCmdMove,
    &gWsCmdSlotDuration,
#if CONFIG_FEATURE_IPERF == 1
    &gWsCmdIperf,
#endif /* CONFIG_FEATURE_IPERF == 1 */
    &gWsCmdButton,
    &gWsCmdEffect,
    &gWsCmdAlias
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void WebSocketSrv::init(AsyncWebServer& srv)
{
    String              webLoginUser;
    String              webLoginPassword;
    SettingsService&    settings        = SettingsService::getInstance();

    if (false == settings.open(true))
    {
        webLoginUser        = settings.getWebLoginUser().getDefault();
        webLoginPassword    = settings.getWebLoginPassword().getDefault();
    }
    else
    {
        webLoginUser        = settings.getWebLoginUser().getValue();
        webLoginPassword    = settings.getWebLoginPassword().getValue();

        settings.close();
    }

    /* Setup the websocket message input queue. */
    (void)m_msgQueue.create(MAX_WEBSOCKET_MSGS);

    /* Register websocket event handler */
    m_webSocket.onEvent(
        [this](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len)
        {
            this->onEvent(server, client, type, arg, data, len);
        }
    );

    /* HTTP Authenticate before switch to Websocket protocol */
    m_webSocket.setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());

    /* Register websocket on webserver */
    srv.addHandler(&m_webSocket);
}

void WebSocketSrv::process()
{
    WebSocketMsg* msg = nullptr;

    /* Handle all messages in the input queue. */
    while(true == m_msgQueue.receive(&msg, 0U))
    {
        if (nullptr != msg)
        {
            if (nullptr != msg->cmd)
            {
                LOG_DEBUG("Websocket command: %s", msg->cmd->getCmd());

                /* Parameter available? */
                if (false == msg->parameters.isEmpty())
                {
                    int     beginIdx    = 0;
                    int     endIdx      = msg->parameters.indexOf(DELIMITER);
                    String  parStr;

                    while(0 <= endIdx)
                    {
                        parStr = msg->parameters.substring(beginIdx, endIdx);

                        LOG_DEBUG("Websocket parameter: %s", parStr.c_str());
                        msg->cmd->setPar(parStr.c_str());

                        beginIdx    = endIdx + 1U; /* Overstep delimiter */
                        endIdx      = msg->parameters.indexOf(DELIMITER, beginIdx);
                    }

                    parStr = msg->parameters.substring(beginIdx);
                    LOG_DEBUG("Websocket parameter: %s", parStr.c_str());
                    msg->cmd->setPar(parStr.c_str());
                }

                msg->cmd->execute(&m_webSocket, msg->clientId);
            }

            delete msg;
            msg = nullptr;
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void WebSocketSrv::onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len)
{
    if ((nullptr == server) ||
        (nullptr == client))
    {
        return;
    }

    switch(type)
    {
    /* Client connected */
    case WS_EVT_CONNECT:
        onConnect(server, client, static_cast<AsyncWebServerRequest*>(arg));
        break;

    /* Client disconnected */
    case WS_EVT_DISCONNECT:
        onDisconnect(server, client);
        break;

    /* Pong received */
    case WS_EVT_PONG:
        onPong(server, client, data, len);
        break;

    /* Remote error */
    case WS_EVT_ERROR:
        onError(server, client, *static_cast<uint16_t*>(arg), reinterpret_cast<const char*>(data), len);
        break;

    /* Data */
    case WS_EVT_DATA:
        onData(server, client, static_cast<AwsFrameInfo*>(arg), data, len);
        break;

    default:
        break;
    }
}

void WebSocketSrv::onConnect(AsyncWebSocket* server, AsyncWebSocketClient* client, AsyncWebServerRequest* request)
{
    UTIL_NOT_USED(request);

    LOG_INFO("ws[%s][%u] Client connected.", server->url(), client->id());
}

void WebSocketSrv::onDisconnect(AsyncWebSocket* server, AsyncWebSocketClient* client)
{
    LOG_INFO("ws[%s][%u] Client disconnected.", server->url(), client->id());
}

void WebSocketSrv::onPong(AsyncWebSocket* server, AsyncWebSocketClient* client, uint8_t* data, size_t len)
{
    if ((nullptr == data) ||
        (0 == len))
    {
        LOG_INFO("ws[%s][%u] Pong: -", server->url(), client->id());
    }
    else
    {
        LOG_INFO("ws[%s][%u] Pong: %s", server->url(), client->id(), data);
    }
}

void WebSocketSrv::onError(AsyncWebSocket* server, AsyncWebSocketClient* client, uint16_t reasonCode, const char* reasonStr, size_t reasonStrLen)
{
    if ((nullptr == reasonStr) ||
        (0 == reasonStrLen))
    {
        LOG_INFO("ws[%s][%u] Error %u: -", server->url(), client->id(), reasonCode);
    }
    else
    {
        LOG_INFO("ws[%s][%u] Error %u: %s", server->url(), client->id(), reasonCode, reasonStr);
    }
}

void WebSocketSrv::onData(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsFrameInfo* info, const uint8_t* data, size_t len)
{
    /* Frame info missing? */
    if (nullptr == info)
    {
        LOG_ERROR("ws[%s][%u] Frame info is missing.", server->url(), client->id());
        server->close(client->id(), 0U, "Frame info is missing.");
    }
    /* No text frame? */
    else if (WS_TEXT != info->opcode)
    {
        LOG_ERROR("ws[%s][%u] Not supported message type received: %u", server->url(), client->id(), info->opcode);
        server->close(client->id(), 0U, "Not supported message type.");
    }
    /* Is the whole message in a single frame and we got all of it's data? */
    else if ((0U < info->final) &&
             (0U == info->index) &&
             (len == info->len ))
    {
        /* Empty text message? */
        if ((nullptr == data) ||
            (0U == len))
        {
            LOG_WARNING("ws[%s][%u] Message: -", server->url(), client->id());
        }
        /* Handle text message */
        else
        {
            const void* vData   = data;
            const char* cData   = static_cast<const char*>(vData);

            handleMsg(server, client, cData, len);
        }
    }
    /* Message is comprised of multiple frames or the frame is split into multiple packets */
    else
    {
        LOG_ERROR("ws[%s][%u] Fragmented messages not supported.", server->url(), client->id());
        server->close(client->id(), 0U, "Not supported message type.");
    }
}

void WebSocketSrv::handleMsg(AsyncWebSocket* server, AsyncWebSocketClient* client, const char* msg, size_t msgLen)
{
    size_t      msgIndex    = 0U;
    const char* cmd         = nullptr;
    size_t      cmdLength   = 0U;
    WsCmd*      wsCmd       = nullptr;

    if ((nullptr == server) ||
        (nullptr == client) ||
        (nullptr == msg) ||
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
    cmd = &msg[msgIndex];
    while((msgLen > msgIndex) && (DELIMITER != msg[msgIndex]))
    {
        ++cmdLength;
        ++msgIndex;
    }

    /* Command string not empty? */
    if (0 < cmdLength)
    {
        uint8_t index = 0U;

        /* Find command object */
        while((nullptr == wsCmd) && (UTIL_ARRAY_NUM(gWsCommands) > index))
        {
            /* Note, cmd is NOT terminated! */
            if (0 == strncmp(gWsCommands[index]->getCmd(), cmd, cmdLength))
            {
                wsCmd = gWsCommands[index];
            }

            ++index;
        }

        /* Command not found? */
        if (nullptr == wsCmd)
        {
            server->text(client->id(), "NACK;\"Command unknown.\"");
        }
        else
        {
            WebSocketMsg* wsMsg = new(std::nothrow) WebSocketMsg;

            /* Overstep delimiter in case there are parameters. */
            if ((DELIMITER == msg[msgIndex]) &&
                (msgLen > msgIndex))
            {
                ++msgIndex;
            }

            if (nullptr != wsMsg)
            {
                wsMsg->cmd          = wsCmd;
                wsMsg->clientId     = client->id();

                if (0U < (msgLen - msgIndex))
                {
                    wsMsg->parameters = String(&msg[msgIndex], msgLen - msgIndex);
                }

                if (false == m_msgQueue.sendToBack(wsMsg, QUEUE_WAIT_TIME * portTICK_PERIOD_MS))
                {
                    LOG_WARNING("Lost websocket message, because queue full.");

                    delete wsMsg;
                }
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

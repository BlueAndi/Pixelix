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
 * @brief  Abstract websocket command
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WsCmd.h"

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

/* Delimiter of websocket parameters */
const char*    WsCmd::DELIMITER = ";";

/* Positive response code */
const char*    WsCmd::ACK       = "ACK";

/* Negative response code. */
const char*    WsCmd::NACK      = "NACK";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

void WsCmd::sendPositiveResponse(AsyncWebSocket* server, AsyncWebSocketClient* client, const char* msg)
{
    if ((nullptr != server) &&
        (nullptr != client))
    {
        String rsp = ACK;

        if ((nullptr != msg) &&
            ('\0' != msg[0U]))
        {
            rsp += DELIMITER;
            rsp += msg;
        }

        server->text(client->id(), rsp);
    }
}

void WsCmd::sendPositiveResponse(AsyncWebSocket* server, AsyncWebSocketClient* client)
{
    sendPositiveResponse(server, client, nullptr);
}

void WsCmd::sendNegativeResponse(AsyncWebSocket* server, AsyncWebSocketClient* client, const char* msg)
{
    if ((nullptr != server) &&
        (nullptr != client))
    {
        String rsp = NACK;

        rsp += DELIMITER;

        if ((nullptr != msg) &&
            ('\0' != msg[0U]))
        {
            rsp += msg;
        }
        else
        {
            rsp += "\"Unknown.\"";
        }

        server->text(client->id(), rsp);
    }
}

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

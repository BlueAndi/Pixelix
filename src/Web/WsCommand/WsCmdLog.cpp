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
 * @brief  Websocket command to get/set logging
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WsCmdLog.h"
#include "WebSocket.h"

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

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void WsCmdLog::execute(AsyncWebSocket* server, AsyncWebSocketClient* client)
{
    if ((nullptr == server) ||
        (nullptr == client))
    {
        return;
    }

    /* Any error happended? */
    if (true == m_isError)
    {
        sendNegativeResponse(server, client, "\"Parameter invalid.\"");
    }
    else
    {
        String      msg;
        LogSink*    selectedSink    = nullptr;

        /* Set logging on/off? */
        if (0 < m_cnt)
        {
            if (false == m_isLoggingOn)
            {
                (void)Logging::getInstance().selectSink("Serial");
            }
            else
            {
                (void)Logging::getInstance().selectSink("Websocket");
            }
        }

        selectedSink = Logging::getInstance().getSelectedSink();

        preparePositiveResponse(msg);

        if ((nullptr == selectedSink) ||
            (selectedSink->getName() != "Websocket"))
        {
            msg += "0";
        }
        else
        {
            msg += "1";
        }

        sendResponse(server, client, msg);
    }

    m_cnt       = 0U;
    m_isError   = false;
}

void WsCmdLog::setPar(const char* par)
{
    if (0U == m_cnt)
    {
        if (0 == strcmp(par, "0"))
        {
            m_isLoggingOn = false;
        }
        else if (0 == strcmp(par, "1"))
        {
            m_isLoggingOn = true;
        }
        else
        {
            m_isError = true;
        }

        ++m_cnt;
    }
    else
    {
        m_isError = true;
    }
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

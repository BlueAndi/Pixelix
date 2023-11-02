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
 * @brief  Websocket command to start/stop iperf
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WsCmdIperf.h"

#include <Logging.h>
#include <Util.h>

#if CONFIG_FEATURE_IPERF == 1

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

void WsCmdIperf::execute(AsyncWebSocket* server, AsyncWebSocketClient* client)
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
    /* Get iperf status? */
    else if (CMD_STATUS == m_cmd)
    {
        String msg;

        preparePositiveResponse(msg);

        if (false == m_isIperfRunning)
        {
            msg += "0";
        }
        else
        {
            msg += "1";
        }
        
        sendResponse(server, client, msg);
    }
    /* Start iperf? */
    else if (CMD_START == m_cmd)
    {
        if (ESP_OK != iperf_start(&m_cfg))
        {
            sendNegativeResponse(server, client, "\"Failed to start.\"");
        }
        else
        {
            String msg;
            
            preparePositiveResponse(msg);

            msg += "1";

            LOG_INFO("iperf started: mode = %s-%s sip = %u.%u.%u.%u:%u, interval = %us, time = %us",
                (m_cfg.flag & IPERF_FLAG_TCP) ? "tcp" : "udp",
                (m_cfg.flag & IPERF_FLAG_SERVER) ? "server" : "client",
                m_cfg.sip & 0xffU, (m_cfg.sip >> 8) & 0xffU, (m_cfg.sip >> 16) & 0xffU, (m_cfg.sip >>24) & 0xffU, m_cfg.sport,
                m_cfg.interval, m_cfg.time);

            sendResponse(server, client, msg);
        }
    }
    /* Stop iperf? */
    else if (CMD_STOP == m_cmd)
    {
        if (ESP_OK != iperf_stop())
        {
            sendNegativeResponse(server, client, "\"Failed to stop.\"");
        }
        else
        {
            String msg;
            
            preparePositiveResponse(msg);
            
            msg += "0";

            LOG_INFO("iperf stopped.");

            sendResponse(server, client, msg);
        }
    }
    else
    {
        sendNegativeResponse(server, client, "\"Parameter invalid.\"");        
    }

    m_isError   = false;
    m_parCnt    = 0U;
    m_cmd       = CMD_STATUS;

    setCfgDefault();
}

void WsCmdIperf::setPar(const char* par)
{
    if (0U == m_parCnt)
    {
        if (0 == strcmp(par, "START"))
        {
            m_cmd = CMD_START;
        }
        else if (0 == strcmp(par, "STOP"))
        {
            m_cmd = CMD_STOP;
        }
        else
        {
            m_isError = true;
        }
    }
    else if (CMD_START == m_cmd)
    {
        if (1U == m_parCnt)
        {
            if (0 == strcmp(par, "DEFAULT"))
            {
                m_cfg.flag = IPERF_FLAG_SERVER | IPERF_FLAG_TCP;
            }
            else if (0 == strcmp(par, "TCP"))
            {
                m_cfg.flag = IPERF_FLAG_SERVER | IPERF_FLAG_TCP;;
            }
            else if (0 == strcmp(par, "UDP"))
            {
                m_cfg.flag = IPERF_FLAG_SERVER | IPERF_FLAG_UDP;
            }
            else
            {
                m_isError = true;
            }
        }
        else if (2U == m_parCnt)
        {
            if (0 == strcmp(par, "DEFAULT"))
            {
                m_cfg.interval = IPERF_DEFAULT_INTERVAL;
            }
            else
            {
                bool status = Util::strToUInt32(String(par), m_cfg.interval);

                if (false == status)
                {
                    m_isError = true;
                }
            }
        }
        else if (3U == m_parCnt)
        {
            if (0 == strcmp(par, "DEFAULT"))
            {
                m_cfg.time = IPERF_DEFAULT_TIME;
            }
            else
            {
                bool status = Util::strToUInt32(String(par), m_cfg.time);

                if (false == status)
                {
                    m_isError = true;
                }
            }
        }
        else
        {
            m_isError = true;
        }
    }
    else
    {
        m_isError = true;
    }

    ++m_parCnt;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void WsCmdIperf::setCfgDefault()
{
    /* Default configuration */
    m_cfg.flag      = IPERF_FLAG_SERVER | IPERF_FLAG_TCP;
    m_cfg.sip       = WiFi.localIP();
    m_cfg.sport     = IPERF_DEFAULT_PORT;
    m_cfg.dip       = 0U;
    m_cfg.dport     = IPERF_DEFAULT_PORT;
    m_cfg.interval  = IPERF_DEFAULT_INTERVAL;
    m_cfg.time      = IPERF_DEFAULT_TIME;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

#endif /* CONFIG_FEATURE_IPERF == 1 */

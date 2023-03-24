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
 *
 * @addtogroup web
 *
 * @{
 */

#ifndef WSCMDIPERF_H
#define WSCMDIPERF_H

#if CONFIG_FEATURE_IPERF == 1

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WsCmd.h"

extern "C"
{
    #include "iperf.h"
}

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Websocket command to to start/stop iperf
 */
class WsCmdIperf: public WsCmd
{
public:

    /**
     * Constructs the websocket command.
     */
    WsCmdIperf() :
        WsCmd("IPERF"),
        m_isError(false),
        m_parCnt(0U),
        m_cmd(CMD_STATUS),
        m_cfg(),
        m_isIperfRunning(false)
    {
        setCfgDefault();
    }

    /**
     * Destroys websocket command.
     */
    ~WsCmdIperf()
    {
    }

    /**
     * Execute command.
     *
     * @param[in] server    Websocket server
     * @param[in] client    Websocket client
     */
    void execute(AsyncWebSocket* server, AsyncWebSocketClient* client) final;

    /**
     * Set command parameter. Call this for each parameter, until executing it.
     *
     * @param[in] par   Parameter string
     */
    void setPar(const char* par) final;

private:

    /** iperf commands */
    enum Cmd
    {
        CMD_STATUS = 0, /**< Get iperf status */
        CMD_START,      /**< Start iperf */
        CMD_STOP        /**< Stop iperf */
    };

    bool        m_isError;          /**< Any error happened during parameter reception? */
    uint8_t     m_parCnt;           /**< Number of received parameters */
    Cmd         m_cmd;              /**< iperf command */
    iperf_cfg_t m_cfg;              /**< iperf configuration */
    bool        m_isIperfRunning;   /**< Is iperf running or not? */

    WsCmdIperf(const WsCmdIperf& cmd);
    WsCmdIperf& operator=(const WsCmdIperf& cmd);

    /**
     * Set iperf default configuration.
     */
    void setCfgDefault();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* CONFIG_FEATURE_IPERF == 1 */

#endif  /* WSCMDIPERF_H */

/** @} */
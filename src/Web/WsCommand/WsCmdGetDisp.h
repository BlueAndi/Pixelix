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
 * @brief  Websocket command get display content
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup WEB
 *
 * @{
 */

#ifndef WSCMDGETDISP_H
#define WSCMDGETDISP_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WsCmd.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Websocket command get display content
 */
class WsCmdGetDisp: public WsCmd
{
public:

    /**
     * Constructs a websocket get display command.
     */
    WsCmdGetDisp() :
        WsCmd("GETDISP"),
        m_isError(false)
    {
    }

    /**
     * Destroys websocket command.
     */
    ~WsCmdGetDisp()
    {
    }

    /**
     * Execute command.
     * 
     * @param[in] server    Websocket server
     * @param[in] clientId  Websocket client ID
     */
    void execute(AsyncWebSocket* server, uint32_t clientId) final;

    /**
     * Set command parameter. Call this for each parameter, until executing it.
     * 
     * @param[in] par   Parameter string
     */
    void setPar(const char* par) final;

private:

    bool    m_isError;  /**< Any error happened during parameter reception? */

    /**
     * RGB data calculation states.
     * 
     * The RGB data is send in a "compressed" format using a repeat counter.
     * The algorithm to calculate the format is implemented as a small state
     * machine that creates sequences of repeated colors from the frame buffer.
     */
    enum GetDispState
    {
        STATE_GETDISP_COLLECT = 0, /**< Collect sequences of same color. */
        STATE_GETDISP_SEND,        /**< Send current color sequence.     */
        STATE_GETDISP_FINISH       /**< Terminate state machine.         */
    };

    WsCmdGetDisp(const WsCmdGetDisp& cmd);
    WsCmdGetDisp& operator=(const WsCmdGetDisp& cmd);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* WSCMDGETDISP_H */

/** @} */
/* MIT License
 *
 * Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>
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
 * 
 * @addtogroup web
 *
 * @{
 */

#ifndef __WSCMD_H__
#define __WSCMD_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <ESPAsyncWebServer.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Abstract websocket command
 */
class WsCmd
{
public:

    /**
     * Constructs a websocket command.
     * 
     * @param[in] cmd   Command string
     */
    WsCmd(const char* cmd) :
        m_cmd(cmd)
    {
    }

    /**
     * Destroys websocket command.
     */
    ~WsCmd()
    {
    }

    /**
     * Get command string.
     * 
     * @return Command string
     */
    const String& getCmd() const
    {
        return m_cmd;
    }

    /**
     * Execute command.
     * 
     * @param[in] server    Websocket server
     * @param[in] client    Websocket client
     */
    virtual void execute(AsyncWebSocket* server, AsyncWebSocketClient* client) = 0;

    /**
     * Set command parameter. Call this for each parameter, until executing it.
     * 
     * @param[in] par   Parameter string
     */
    virtual void setPar(const char* par) = 0;

private:

    String  m_cmd;  /**< Command */

    WsCmd();
    WsCmd(const WsCmd& cmd);
    WsCmd& operator=(const WsCmd& cmd);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __WSCMD_H__ */

/** @} */
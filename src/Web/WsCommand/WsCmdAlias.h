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
 * @brief  Websocket command to get/set plugin alias name
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup WEB
 *
 * @{
 */

#ifndef WSCMDALIAS_H
#define WSCMDALIAS_H

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
 * Websocket command to get/set plugin alias name
 */
class WsCmdAlias: public WsCmd
{
public:

    /**
     * Constructs the websocket command.
     */
    WsCmdAlias() :
        WsCmd("ALIAS"),
        m_isError(false),
        m_parCnt(0U),
        m_pluginUid(0),
        m_alias()
    {
    }

    /**
     * Destroys websocket command.
     */
    ~WsCmdAlias()
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

    bool        m_isError;      /**< Any error happened during parameter reception? */
    uint8_t     m_parCnt;       /**< Received number of parameters */
    uint16_t    m_pluginUid;    /**< Plugin UID */
    String      m_alias;        /**< Plugin alias name */

    WsCmdAlias(const WsCmdAlias& cmd);
    WsCmdAlias& operator=(const WsCmdAlias& cmd);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* WSCMDALIAS_H */

/** @} */
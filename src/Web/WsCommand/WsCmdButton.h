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
 * @brief  Websocket command to control the virtual user button
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup web
 *
 * @{
 */

#ifndef WSCMDBUTTON_H
#define WSCMDBUTTON_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WsCmd.h"
#include "ButtonActions.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Websocket command to control the virtual user button.
 */
class WsCmdButton: public WsCmd, public ButtonActions
{
public:

    /**
     * Constructs the websocket command.
     */
    WsCmdButton() :
        WsCmd("BUTTON"),
        ButtonActions(),
        m_isError(false),
        m_actionId(BUTTON_ACTION_ID_ACTIVATE_NEXT_SLOT) /* Default */
    {
    }

    /**
     * Destroys websocket command.
     */
    ~WsCmdButton()
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

    bool            m_isError;  /**< Any error happened during parameter reception? */
    ButtonActionId  m_actionId; /**< Id of the action, which to execute. */

    WsCmdButton(const WsCmdButton& cmd);
    WsCmdButton& operator=(const WsCmdButton& cmd);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* WSCMDBUTTON_H */

/** @} */
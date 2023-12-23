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
 * 
 * @addtogroup web
 *
 * @{
 */

#ifndef WSCMD_H
#define WSCMD_H

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
    const char* getCmd() const
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

protected:

    /** Delimiter of websocket parameters */
    static const char*    DELIMITER;

    /** Positive response code */
    static const char*    ACK;

    /** Negative response code. */
    static const char*    NACK;

    /**
     * Prepare a positive response message.
     * The last added element will always be a delimiter.
     * 
     * @param[out] msg  Message to prepare
     */
    void preparePositiveResponse(String& msg)
    {
        msg = ACK;
        msg += DELIMITER;
    }

    /**
     * Prepare a negative response message.
     * The last added element will always be a delimiter.
     * 
     * @param[out] msg  Message to prepare
     */
    void prepareNegativeResponse(String& msg)
    {
        msg = NACK;
        msg += DELIMITER;
    }

    /**
     * Send a response to the client.
     * 
     * @param[in] server    Websocket server which is used to send a message to the client.
     * @param[in] client    The client the message belongs to.
     * @param[in] msg       The response messsage.
     */
    void sendResponse(AsyncWebSocket* server, AsyncWebSocketClient* client, const String& msg)
    {
        server->text(client->id(), msg);
    }

    /**
     * Send positive response to the client.
     * 
     * @param[in] server    Websocket server which is used to send a message to the client.
     * @param[in] client    The client the message belongs to.
     * @param[in] msg       The negative response messsage.
     */
    void sendPositiveResponse(AsyncWebSocket* server, AsyncWebSocketClient* client, const char* msg);

    /**
     * Send negative response to the client.
     * 
     * @param[in] server    Websocket server which is used to send a message to the client.
     * @param[in] client    The client the message belongs to.
     * @param[in] msg       The negative response messsage.
     */
    void sendPositiveResponse(AsyncWebSocket* server, AsyncWebSocketClient* client);

    /**
     * Send negative response to the client.
     * 
     * @param[in] server    Websocket server which is used to send a message to the client.
     * @param[in] client    The client the message belongs to.
     * @param[in] msg       The negative response messsage.
     */
    void sendNegativeResponse(AsyncWebSocket* server, AsyncWebSocketClient* client, const char* msg);

private:

    const char* m_cmd;  /**< Command */

    WsCmd();
    WsCmd(const WsCmd& cmd);
    WsCmd& operator=(const WsCmd& cmd);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* WSCMD_H */

/** @} */
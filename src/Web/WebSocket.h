/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Websocket
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup web
 *
 * @{
 */

#ifndef __WEBSOCKET_H__
#define __WEBSOCKET_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <ESPAsyncWebServer.h>
#include <stdint.h>
#include <Print.h>

#include "WebConfig.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Websocket server
 */
class WebSocketSrv : public Print
{
public:

    /**
     * Get websocket server instance.
     * 
     * @return Websocket server instance
     */
    static WebSocketSrv& getInstance(void)
    {
        return m_instance;
    }

    /**
     * Initialize websocket server and register it on the webserver.
     * 
     * @param[in] srv   Web server
     */
    void init(AsyncWebServer& srv);

private:

    static WebSocketSrv m_instance;     /**< Websocket instance */

    AsyncWebSocket      m_webSocket;    /**< Websocket */

    /**
     * Constructs the websocket server.
     */
    WebSocketSrv() :
        m_webSocket(WebConfig::WEBSOCKET_PATH)
    {
    }

    /**
     * Destroys the websocket server.
     */
    ~WebSocketSrv()
    {
    }

    /**
     * Websocket event handler.
     * 
     * @param[in] server    Websocket server
     * @param[in] client    Weboscket client
     * @param[in] type      Websocket event type
     * @param[in] arg       Websocket argument
     * @param[in] data      Websocket data
     * @param[in] len       Websocket data length in bytes
     */
    static void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);

    /**
     * Write single data byte to all clients.
     * 
     * @param[in] data  Data byte
     * 
     * @return Number of written bytes.
     */
    size_t write(uint8_t data)
    {
        m_webSocket.textAll(&data, 1);
        return 1;
    }

    /**
     * Write data to all clients.
     * 
     * @param[in] buffer    Data buffer
     * @param[in] size      Data buffer size
     * 
     * @return Number of written bytes.
     */
    size_t write(const uint8_t *buffer, size_t size)
    {
        m_webSocket.textAll(const_cast<uint8_t*>(buffer), size);
        return size;
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __WEBSOCKET_H__ */

/** @} */
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
 * @brief  Websocket log sink
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup utilities
 *
 * @{
 */

#ifndef LOG_SINK_WEBSOCKET_H
#define LOG_SINK_WEBSOCKET_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Logging.h"
#include "WebSocket.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Websocket log sink.
 */
class LogSinkWebsocket : public LogSink
{
public:

    /**
     * Constructs a empty printer log sink.
     */
    LogSinkWebsocket() :
        m_name(),
        m_output(nullptr)
    {

    }

    /**
     * Constructs a websocket log sink.
     *
     * @param[in] name      Name of the sink
     * @param[in] output    Websocket
     */
    LogSinkWebsocket(const String& name, WebSocketSrv* output) :
        m_name(name),
        m_output(output)
    {
    }

    /**
     * Destroys the websocket log sink.
     */
    ~LogSinkWebsocket()
    {
    }

    /**
     * Get websocket.
     *
     * @return Websocket
     */
    WebSocketSrv* getWebsocket()
    {
        return m_output;
    }

    /**
     * Set websocket.
     *
     * @param[in] ws WebSocketSrv
     */
    void setWebsocket(WebSocketSrv* ws)
    {
        m_output = ws;
    }

    /**
     * Set sink name.
     *
     * @param[in] name Name of the sink
     */
    void setName(const String& name)
    {
        m_name = name;
    }

    /**
     * Get sink name.
     *
     * @return Name of the sink.
     */
    const String& getName() const final
    {
        return m_name;
    }

    /**
     * Send a log message to this sink.
     *
     * @param[in] msg   Log message
     */
    void send(const Logging::Msg& msg) final;

private:

    String          m_name;     /**< Name of the sink */
    WebSocketSrv*   m_output;   /**< Log sink output */

    LogSinkWebsocket(const LogSinkWebsocket& sink);
    LogSinkWebsocket& operator=(const LogSinkWebsocket& sink);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* LOG_SINK_WEBSOCKET_H */

/** @} */
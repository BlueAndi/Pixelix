/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   AsyncTCP.h
 * @brief  Asynchronous TCP for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * Counterpart of the AsyncTCP library, which the webserver is based on. It
 * provides the very same interface, but on top of sockets instead of lwIP.
 *
 * Like the target, every callback is called from one single thread. The
 * webserver is written for that guarantee and would corrupt its internal state
 * if the callbacks of different clients would run concurrently.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef ASYNC_TCP_H
#define ASYNC_TCP_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <functional>

/* Note, Arduino.h is deliberately not included here. It defines the F() macro
 * and a boolean type, which collide with the Windows SDK headers that winsock
 * pulls in. See SocketCompat.hpp, which must be included before Arduino.h.
 */
#include <IPAddress.h>
#include <lwip/tcpbase.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Copy the data, which shall be written. */
#define ASYNC_WRITE_FLAG_COPY (0x01U)

/** Send the data more, which shall be written. */
#define ASYNC_WRITE_FLAG_MORE (0x02U)

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

class AsyncClient;

/** Handler which is called after a connection is established or closed. */
typedef std::function<void(void*, AsyncClient*)> AcConnectHandler;

/** Handler which is called after data is acknowledged by the peer. */
typedef std::function<void(void*, AsyncClient*, size_t len, uint32_t time)> AcAckHandler;

/** Handler which is called in case of an error. */
typedef std::function<void(void*, AsyncClient*, int8_t error)> AcErrorHandler;

/** Handler which is called after data is received. */
typedef std::function<void(void*, AsyncClient*, void* data, size_t len)> AcDataHandler;

/** Handler which is called after a timeout. */
typedef std::function<void(void*, AsyncClient*, uint32_t time)> AcTimeoutHandler;

/**
 * A single asynchronous TCP connection.
 */
class AsyncClient
{
public:

    /**
     * Constructs a client for the given socket.
     *
     * @param[in] sock  Socket of an accepted connection.
     */
    AsyncClient(int sock = -1);

    /**
     * Destroys the client.
     */
    ~AsyncClient();

    /**
     * Add data to the transmit buffer, but don't send it yet.
     *
     * @param[in] data      Data buffer
     * @param[in] size      Data buffer size in byte
     * @param[in] apiFlags  Write flags
     *
     * @return Number of accepted bytes.
     */
    size_t add(const char* data, size_t size, uint8_t apiFlags = ASYNC_WRITE_FLAG_COPY);

    /**
     * Send the data in the transmit buffer.
     *
     * @return If successful send, it will return true otherwise false.
     */
    bool send();

    /**
     * Add data to the transmit buffer and send it.
     *
     * @param[in] data      Data buffer
     * @param[in] size      Data buffer size in byte
     * @param[in] apiFlags  Write flags
     *
     * @return Number of written bytes.
     */
    size_t write(const char* data, size_t size, uint8_t apiFlags = ASYNC_WRITE_FLAG_COPY);

    /**
     * Add a string to the transmit buffer and send it.
     *
     * @param[in] data  Zero terminated string
     *
     * @return Number of written bytes.
     */
    size_t write(const char* data);

    /**
     * Get the free space of the transmit buffer.
     *
     * @return Free space in byte
     */
    size_t space() const;

    /**
     * Can data be sent?
     *
     * @return If data can be sent, it will return true otherwise false.
     */
    bool canSend() const;

    /**
     * Close the connection.
     */
    void close();

    /**
     * Close the connection.
     *
     * @param[in] now   Close it immediately or not.
     */
    void close(bool now)
    {
        (void)now;

        close();
    }

    /**
     * Close the connection immediately.
     */
    void stop()
    {
        close();
    }

    /**
     * Abort the connection.
     *
     * @return Always -1, like the AsyncTCP counterpart.
     */
    int8_t abort();

    /**
     * Can the client be released?
     *
     * @return If the client can be released, it will return true otherwise false.
     */
    bool free() const;

    /**
     * Is the connection established?
     *
     * @return If the connection is established, it will return true otherwise false.
     */
    bool connected() const;

    /**
     * Is the connection closed?
     *
     * @return If the connection is closed, it will return true otherwise false.
     */
    bool disconnected() const;

    /**
     * Get the TCP state of the connection.
     *
     * @return TCP state
     */
    uint8_t state() const;

    /**
     * Get the TCP state of the connection as string.
     *
     * @return TCP state
     */
    const char* stateToString() const;

    /**
     * Enable or disable the nagle algorithm.
     *
     * @param[in] noDelay   Disable the nagle algorithm or not.
     */
    void setNoDelay(bool noDelay);

    /**
     * Is the nagle algorithm disabled?
     *
     * @return If the nagle algorithm is disabled, it will return true otherwise false.
     */
    bool getNoDelay() const;

    /**
     * Set the timeout for receiving data.
     *
     * @param[in] timeout   Timeout in s.
     */
    void setRxTimeout(uint32_t timeout);

    /**
     * Get the timeout for receiving data.
     *
     * @return Timeout in s.
     */
    uint32_t getRxTimeout() const;

    /**
     * Acknowledge the received data later.
     */
    void ackLater()
    {
        m_isAckPending = true;
    }

    /**
     * Acknowledge the given number of received bytes.
     *
     * @param[in] len   Number of bytes.
     *
     * @return Number of acknowledged bytes.
     */
    size_t ack(size_t len)
    {
        m_isAckPending = false;

        return len;
    }

    /**
     * Get the IP address of the peer.
     *
     * @return IP address
     */
    IPAddress remoteIP() const;

    /**
     * Get the port of the peer.
     *
     * @return Port number
     */
    uint16_t remotePort() const;

    /**
     * Get the local IP address.
     *
     * @return IP address
     */
    IPAddress localIP() const;

    /**
     * Get the local port.
     *
     * @return Port number
     */
    uint16_t localPort() const;

    /**
     * Set the handler which is called after the connection is established.
     *
     * @param[in] cb    The handler.
     * @param[in] arg   User specific argument.
     */
    void onConnect(AcConnectHandler cb, void* arg = nullptr);

    /**
     * Set the handler which is called after the connection is closed.
     *
     * @param[in] cb    The handler.
     * @param[in] arg   User specific argument.
     */
    void onDisconnect(AcConnectHandler cb, void* arg = nullptr);

    /**
     * Set the handler which is called after data is acknowledged.
     *
     * @param[in] cb    The handler.
     * @param[in] arg   User specific argument.
     */
    void onAck(AcAckHandler cb, void* arg = nullptr);

    /**
     * Set the handler which is called in case of an error.
     *
     * @param[in] cb    The handler.
     * @param[in] arg   User specific argument.
     */
    void onError(AcErrorHandler cb, void* arg = nullptr);

    /**
     * Set the handler which is called after data is received.
     *
     * @param[in] cb    The handler.
     * @param[in] arg   User specific argument.
     */
    void onData(AcDataHandler cb, void* arg = nullptr);

    /**
     * Set the handler which is called after a timeout.
     *
     * @param[in] cb    The handler.
     * @param[in] arg   User specific argument.
     */
    void onTimeout(AcTimeoutHandler cb, void* arg = nullptr);

    /**
     * Set the handler which is called periodically.
     *
     * @param[in] cb    The handler.
     * @param[in] arg   User specific argument.
     */
    void onPoll(AcConnectHandler cb, void* arg = nullptr);

    /**
     * Process the connection. Called by the event loop only.
     *
     * @param[in] isReadable    Is data available?
     * @param[in] isWritable    Can data be written?
     */
    void process(bool isReadable, bool isWritable);

    /**
     * Notify the poll handler. Called by the event loop only.
     */
    void notifyPoll();

    /**
     * Notify the ack handler about the data, which was sent since the last
     * notification. Called by the event loop only.
     */
    void notifyAck();

    /**
     * Is there data sent, which is not acknowledged yet? Called by the event
     * loop only.
     *
     * @return If a notification is pending, it will return true otherwise false.
     */
    bool hasPendingAck() const;

    /**
     * Get the socket. Called by the event loop only.
     *
     * @return Socket
     */
    int getSocket() const
    {
        return m_socket;
    }

    /**
     * Is there data pending to be sent? Called by the event loop only.
     *
     * @return If data is pending, it will return true otherwise false.
     */
    bool hasPendingTxData() const;

private:

    /** Max. size of the transmit buffer in byte. */
    static const size_t TX_BUFFER_SIZE = 5744U;

    /** Max. size of a single receive in byte. */
    static const size_t RX_BUFFER_SIZE = 1436U;

    int                 m_socket;                   /**< Socket of the connection. */
    bool                m_isNoDelay;                /**< Is the nagle algorithm disabled? */
    bool                m_isAckPending;             /**< Shall the received data be acknowledged later? */
    uint32_t            m_rxTimeout;                /**< Timeout for receiving data in s. */
    uint32_t            m_lastRxTime;               /**< Timestamp of the last received data in ms. */
    char                m_txBuffer[TX_BUFFER_SIZE]; /**< Transmit buffer. */
    size_t              m_txBufferLength;           /**< Number of bytes in the transmit buffer. */
    size_t              m_pendingAck;               /**< Number of sent bytes, which are not notified to the ack handler yet. */

    AcConnectHandler    m_connectHandler;    /**< Handler for a established connection. */
    void*               m_connectArg;        /**< User argument of the connect handler. */
    AcConnectHandler    m_disconnectHandler; /**< Handler for a closed connection. */
    void*               m_disconnectArg;     /**< User argument of the disconnect handler. */
    AcAckHandler        m_ackHandler;        /**< Handler for acknowledged data. */
    void*               m_ackArg;            /**< User argument of the ack handler. */
    AcErrorHandler      m_errorHandler;      /**< Handler for an error. */
    void*               m_errorArg;          /**< User argument of the error handler. */
    AcDataHandler       m_dataHandler;       /**< Handler for received data. */
    void*               m_dataArg;           /**< User argument of the data handler. */
    AcTimeoutHandler    m_timeoutHandler;    /**< Handler for a timeout. */
    void*               m_timeoutArg;        /**< User argument of the timeout handler. */
    AcConnectHandler    m_pollHandler;       /**< Handler which is called periodically. */
    void*               m_pollArg;           /**< User argument of the poll handler. */

    AsyncClient(const AsyncClient& client);
    AsyncClient& operator=(const AsyncClient& client);

    /**
     * Close the socket and notify the disconnect handler.
     */
    void handleDisconnect();
};

/**
 * A server which accepts asynchronous TCP connections.
 */
class AsyncServer
{
public:

    /**
     * Constructs the server.
     *
     * @param[in] port  Port number, the server listens on.
     */
    AsyncServer(uint16_t port);

    /**
     * Constructs the server.
     *
     * @param[in] addr  IP address, the server binds to.
     * @param[in] port  Port number, the server listens on.
     */
    AsyncServer(const IPAddress& addr, uint16_t port);

    /**
     * Destroys the server.
     */
    ~AsyncServer();

    /**
     * Set the handler which is called after a client connected.
     *
     * @param[in] cb    The handler.
     * @param[in] arg   User specific argument.
     */
    void onClient(AcConnectHandler cb, void* arg);

    /**
     * Start listening.
     */
    void begin();

    /**
     * Stop listening.
     */
    void end();

    /**
     * Enable or disable the nagle algorithm for every accepted client.
     *
     * @param[in] noDelay   Disable the nagle algorithm or not.
     */
    void setNoDelay(bool noDelay);

    /**
     * Is the nagle algorithm disabled?
     *
     * @return If the nagle algorithm is disabled, it will return true otherwise false.
     */
    bool getNoDelay() const;

    /**
     * Get the state of the listening socket.
     *
     * @return TCP state
     */
    uint8_t status() const;

    /**
     * Accept a pending connection. Called by the event loop only.
     */
    void acceptClient();

    /**
     * Get the listening socket. Called by the event loop only.
     *
     * @return Socket
     */
    int getSocket() const
    {
        return m_socket;
    }

private:

    uint16_t         m_port;           /**< Port number, the server listens on. */
    IPAddress        m_addr;           /**< IP address, the server binds to. */
    int              m_socket;         /**< Listening socket. */
    bool             m_isNoDelay;      /**< Is the nagle algorithm disabled? */
    AcConnectHandler m_connectHandler; /**< Handler for an accepted client. */
    void*            m_connectArg;     /**< User argument of the connect handler. */

    AsyncServer(const AsyncServer& server);
    AsyncServer& operator=(const AsyncServer& server);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* ASYNC_TCP_H */

/** @} */

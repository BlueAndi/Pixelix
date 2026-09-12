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
 * @file   AsyncTCP.cpp
 * @brief  Asynchronous TCP for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
/* The socket headers must come first. On Windows they pull in the Windows SDK,
 * which collides with the F() macro and the boolean type of Arduino.h.
 */
#include <SocketCompat.hpp>

#include "AsyncTCP.h"
#include "AsyncTcpLoop.h"

#include <Arduino.h>
#include <Logging.h>
#include <string.h>
#include <new>

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

static IPAddress toIPAddress(uint32_t netOrderAddr);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

AsyncClient::AsyncClient(int sock) :
    m_socket(sock),
    m_isNoDelay(false),
    m_isAckPending(false),
    m_rxTimeout(0U),
    m_lastRxTime(millis()),
    m_txBuffer{ 0 },
    m_txBufferLength(0U),
    m_pendingAck(0U),
    m_connectHandler(nullptr),
    m_connectArg(nullptr),
    m_disconnectHandler(nullptr),
    m_disconnectArg(nullptr),
    m_ackHandler(nullptr),
    m_ackArg(nullptr),
    m_errorHandler(nullptr),
    m_errorArg(nullptr),
    m_dataHandler(nullptr),
    m_dataArg(nullptr),
    m_timeoutHandler(nullptr),
    m_timeoutArg(nullptr),
    m_pollHandler(nullptr),
    m_pollArg(nullptr)
{
}

AsyncClient::~AsyncClient()
{
    AsyncTcpLoop::getInstance().unregisterClient(this);

    if (-1 != m_socket)
    {
        SocketCompat::closeSocket(static_cast<SocketCompat::Socket>(m_socket));
        m_socket = -1;
    }
}

size_t AsyncClient::add(const char* data, size_t size, uint8_t apiFlags)
{
    size_t accepted = 0U;

    /* The data is always copied, the transmit buffer is owned by the client. */
    (void)apiFlags;

    if ((nullptr != data) &&
        (-1 != m_socket))
    {
        accepted = space();

        if (size < accepted)
        {
            accepted = size;
        }

        if (0U < accepted)
        {
            memcpy(&m_txBuffer[m_txBufferLength], data, accepted);
            m_txBufferLength += accepted;
        }
    }

    return accepted;
}

bool AsyncClient::send()
{
    bool isSuccessful = false;

    if ((-1 != m_socket) &&
        (0U < m_txBufferLength))
    {
        int written = SocketCompat::sendData(static_cast<SocketCompat::Socket>(m_socket), m_txBuffer, m_txBufferLength);

        if (0 < written)
        {
            size_t sent = static_cast<size_t>(written);

            /* Keep the rest, the event loop sends it as soon as the socket is
             * writable again.
             */
            if (sent < m_txBufferLength)
            {
                memmove(m_txBuffer, &m_txBuffer[sent], m_txBufferLength - sent);
            }

            m_txBufferLength -= sent;
            isSuccessful      = true;

            /* On the target the ack is notified by the TCP/IP task, never from
             * inside a write. The webserver relies on that, because its
             * response state machine writes first and updates its state
             * afterwards. Therefore the notification is deferred to the event
             * loop, see notifyAck().
             */
            m_pendingAck     += sent;
        }
        else if (true == SocketCompat::wouldBlock())
        {
            /* Try again in the next cycle. */
            isSuccessful = true;
        }
        else
        {
            handleDisconnect();
        }
    }
    else if (-1 != m_socket)
    {
        /* Nothing to send is not an error. */
        isSuccessful = true;
    }
    else
    {
        /* Guard: no connection. */
    }

    return isSuccessful;
}

size_t AsyncClient::write(const char* data, size_t size, uint8_t apiFlags)
{
    size_t written = add(data, size, apiFlags);

    if (0U < written)
    {
        (void)send();
    }

    return written;
}

size_t AsyncClient::write(const char* data)
{
    size_t written = 0U;

    if (nullptr != data)
    {
        written = write(data, strlen(data));
    }

    return written;
}

size_t AsyncClient::space() const
{
    return TX_BUFFER_SIZE - m_txBufferLength;
}

bool AsyncClient::canSend() const
{
    return (0U < space());
}

void AsyncClient::close()
{
    handleDisconnect();
}

int8_t AsyncClient::abort()
{
    handleDisconnect();

    return -1;
}

bool AsyncClient::free() const
{
    return (-1 == m_socket);
}

bool AsyncClient::connected() const
{
    return (-1 != m_socket);
}

bool AsyncClient::disconnected() const
{
    return (-1 == m_socket);
}

uint8_t AsyncClient::state() const
{
    return (-1 == m_socket) ? static_cast<uint8_t>(CLOSED) : static_cast<uint8_t>(ESTABLISHED);
}

const char* AsyncClient::stateToString() const
{
    return (-1 == m_socket) ? "CLOSED" : "ESTABLISHED";
}

void AsyncClient::setNoDelay(bool noDelay)
{
    m_isNoDelay = noDelay;

    if (-1 != m_socket)
    {
        SocketCompat::setNoDelay(static_cast<SocketCompat::Socket>(m_socket), noDelay);
    }
}

bool AsyncClient::getNoDelay() const
{
    return m_isNoDelay;
}

void AsyncClient::setRxTimeout(uint32_t timeout)
{
    m_rxTimeout = timeout;
}

uint32_t AsyncClient::getRxTimeout() const
{
    return m_rxTimeout;
}

IPAddress AsyncClient::remoteIP() const
{
    struct sockaddr_in addr;
    socklen_t          addrLen = sizeof(addr);
    IPAddress          result;

    if ((-1 != m_socket) &&
        (0 == getpeername(static_cast<SocketCompat::Socket>(m_socket), reinterpret_cast<struct sockaddr*>(&addr), &addrLen)))
    {
        result = toIPAddress(addr.sin_addr.s_addr);
    }

    return result;
}

uint16_t AsyncClient::remotePort() const
{
    struct sockaddr_in addr;
    socklen_t          addrLen = sizeof(addr);
    uint16_t           port    = 0U;

    if ((-1 != m_socket) &&
        (0 == getpeername(static_cast<SocketCompat::Socket>(m_socket), reinterpret_cast<struct sockaddr*>(&addr), &addrLen)))
    {
        port = ntohs(addr.sin_port);
    }

    return port;
}

IPAddress AsyncClient::localIP() const
{
    struct sockaddr_in addr;
    socklen_t          addrLen = sizeof(addr);
    IPAddress          result;

    if ((-1 != m_socket) &&
        (0 == getsockname(static_cast<SocketCompat::Socket>(m_socket), reinterpret_cast<struct sockaddr*>(&addr), &addrLen)))
    {
        result = toIPAddress(addr.sin_addr.s_addr);
    }

    return result;
}

uint16_t AsyncClient::localPort() const
{
    struct sockaddr_in addr;
    socklen_t          addrLen = sizeof(addr);
    uint16_t           port    = 0U;

    if ((-1 != m_socket) &&
        (0 == getsockname(static_cast<SocketCompat::Socket>(m_socket), reinterpret_cast<struct sockaddr*>(&addr), &addrLen)))
    {
        port = ntohs(addr.sin_port);
    }

    return port;
}

void AsyncClient::onConnect(AcConnectHandler cb, void* arg)
{
    m_connectHandler = cb;
    m_connectArg     = arg;
}

void AsyncClient::onDisconnect(AcConnectHandler cb, void* arg)
{
    m_disconnectHandler = cb;
    m_disconnectArg     = arg;
}

void AsyncClient::onAck(AcAckHandler cb, void* arg)
{
    m_ackHandler = cb;
    m_ackArg     = arg;
}

void AsyncClient::onError(AcErrorHandler cb, void* arg)
{
    m_errorHandler = cb;
    m_errorArg     = arg;
}

void AsyncClient::onData(AcDataHandler cb, void* arg)
{
    m_dataHandler = cb;
    m_dataArg     = arg;
}

void AsyncClient::onTimeout(AcTimeoutHandler cb, void* arg)
{
    m_timeoutHandler = cb;
    m_timeoutArg     = arg;
}

void AsyncClient::onPoll(AcConnectHandler cb, void* arg)
{
    m_pollHandler = cb;
    m_pollArg     = arg;
}

void AsyncClient::process(bool isReadable, bool isWritable)
{
    if (-1 == m_socket)
    {
        /* Guard: no connection. */
    }
    else
    {
        if (true == isReadable)
        {
            char rxBuffer[RX_BUFFER_SIZE];
            int  received = SocketCompat::receiveData(static_cast<SocketCompat::Socket>(m_socket), rxBuffer, sizeof(rxBuffer));

            if (0 < received)
            {
                m_lastRxTime = millis();

                if (nullptr != m_dataHandler)
                {
                    m_dataHandler(m_dataArg, this, rxBuffer, static_cast<size_t>(received));
                }
            }
            /* The peer closed the connection. */
            else if (0 == received)
            {
                handleDisconnect();
            }
            else if (false == SocketCompat::wouldBlock())
            {
                handleDisconnect();
            }
            else
            {
                /* Nothing available, try again later. */
                ;
            }
        }

        if ((-1 != m_socket) &&
            (true == isWritable))
        {
            (void)send();
        }
    }
}

void AsyncClient::notifyPoll()
{
    if (-1 != m_socket)
    {
        /* The webserver closes a connection which is idle for too long. */
        if ((0U < m_rxTimeout) &&
            ((m_rxTimeout * 1000U) < (millis() - m_lastRxTime)))
        {
            if (nullptr != m_timeoutHandler)
            {
                m_timeoutHandler(m_timeoutArg, this, millis() - m_lastRxTime);
            }
        }

        if ((-1 != m_socket) &&
            (nullptr != m_pollHandler))
        {
            m_pollHandler(m_pollArg, this);
        }
    }
}

void AsyncClient::notifyAck()
{
    if (0U < m_pendingAck)
    {
        size_t acked = m_pendingAck;

        /* Cleared before the notification, because the handler may write again
         * and therefore cause the next pending acknowledge.
         */
        m_pendingAck = 0U;

        if ((-1 != m_socket) &&
            (nullptr != m_ackHandler))
        {
            m_ackHandler(m_ackArg, this, acked, millis());
        }
    }
}

bool AsyncClient::hasPendingAck() const
{
    return (0U < m_pendingAck);
}

bool AsyncClient::hasPendingTxData() const
{
    return (0U < m_txBufferLength);
}

AsyncServer::AsyncServer(uint16_t port) :
    m_port(port),
    m_addr(),
    m_socket(-1),
    m_isNoDelay(false),
    m_connectHandler(nullptr),
    m_connectArg(nullptr)
{
}

AsyncServer::AsyncServer(const IPAddress& addr, uint16_t port) :
    m_port(port),
    m_addr(addr),
    m_socket(-1),
    m_isNoDelay(false),
    m_connectHandler(nullptr),
    m_connectArg(nullptr)
{
}

AsyncServer::~AsyncServer()
{
    end();
}

void AsyncServer::onClient(AcConnectHandler cb, void* arg)
{
    m_connectHandler = cb;
    m_connectArg     = arg;
}

void AsyncServer::begin()
{
    if (-1 != m_socket)
    {
        /* Guard: already listening. */
    }
    else if (false == SocketCompat::init())
    {
        LOG_ERROR("Socket API not available.");
    }
    else
    {
        SocketCompat::Socket sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (SocketCompat::INVALID_SOCKET_HANDLE == sock)
        {
            LOG_ERROR("Failed to create the server socket.");
        }
        else
        {
            struct sockaddr_in addr;

            SocketCompat::setReuseAddr(sock);

            memset(&addr, 0, sizeof(addr));
            addr.sin_family      = AF_INET;
            addr.sin_port        = htons(m_port);
            addr.sin_addr.s_addr = htonl(INADDR_ANY);

            if (0 != bind(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)))
            {
                LOG_ERROR("Failed to bind the server socket to port %u.", m_port);
                SocketCompat::closeSocket(sock);
            }
            else if (0 != listen(sock, SOMAXCONN))
            {
                LOG_ERROR("Failed to listen on port %u.", m_port);
                SocketCompat::closeSocket(sock);
            }
            else if (false == SocketCompat::setNonBlocking(sock))
            {
                LOG_ERROR("Failed to set the server socket non-blocking.");
                SocketCompat::closeSocket(sock);
            }
            else
            {
                m_socket = static_cast<int>(sock);

                AsyncTcpLoop::getInstance().registerServer(this);

                LOG_INFO("Webserver is listening on port %u.", m_port);
            }
        }
    }
}

void AsyncServer::end()
{
    if (-1 != m_socket)
    {
        AsyncTcpLoop::getInstance().unregisterServer(this);

        SocketCompat::closeSocket(static_cast<SocketCompat::Socket>(m_socket));
        m_socket = -1;
    }
}

void AsyncServer::setNoDelay(bool noDelay)
{
    m_isNoDelay = noDelay;
}

bool AsyncServer::getNoDelay() const
{
    return m_isNoDelay;
}

uint8_t AsyncServer::status() const
{
    return (-1 == m_socket) ? static_cast<uint8_t>(CLOSED) : static_cast<uint8_t>(LISTEN);
}

void AsyncServer::acceptClient()
{
    if (-1 != m_socket)
    {
        SocketCompat::Socket sock = accept(static_cast<SocketCompat::Socket>(m_socket), nullptr, nullptr);

        if (SocketCompat::INVALID_SOCKET_HANDLE != sock)
        {
            if (false == SocketCompat::setNonBlocking(sock))
            {
                SocketCompat::closeSocket(sock);
            }
            else
            {
                /* The webserver takes the ownership of the client and deletes
                 * it as soon as the connection is closed.
                 */
                AsyncClient* client = new (std::nothrow) AsyncClient(static_cast<int>(sock));

                if (nullptr == client)
                {
                    SocketCompat::closeSocket(sock);
                }
                else
                {
                    client->setNoDelay(m_isNoDelay);

                    AsyncTcpLoop::getInstance().registerClient(client);

                    if (nullptr != m_connectHandler)
                    {
                        m_connectHandler(m_connectArg, client);
                    }
                }
            }
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void AsyncClient::handleDisconnect()
{
    if (-1 != m_socket)
    {
        AsyncTcpLoop::getInstance().unregisterClient(this);

        SocketCompat::closeSocket(static_cast<SocketCompat::Socket>(m_socket));
        m_socket         = -1;
        m_txBufferLength = 0U;

        if (nullptr != m_disconnectHandler)
        {
            m_disconnectHandler(m_disconnectArg, this);
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Convert a IPv4 address in network byte order to a IPAddress.
 *
 * @param[in] netOrderAddr  IPv4 address in network byte order.
 *
 * @return IP address
 */
static IPAddress toIPAddress(uint32_t netOrderAddr)
{
    uint32_t hostOrderAddr = ntohl(netOrderAddr);

    return IPAddress(hostOrderAddr);
}

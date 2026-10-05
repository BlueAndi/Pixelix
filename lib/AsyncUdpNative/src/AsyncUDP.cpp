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
 * @file   AsyncUDP.cpp
 * @brief  Asynchronous UDP for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
/* The socket headers must come first. On Windows they pull in the Windows SDK,
 * which collides with the F() macro and the boolean type of Arduino.h.
 */
#include <SocketCompat.hpp>

#include "AsyncUDP.h"
#include "AsyncUdpLoop.h"

#include <Arduino.h>
#include <Logging.h>
#include <string.h>

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

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

size_t AsyncUDPMessage::write(const uint8_t* data, size_t size)
{
    size_t written = 0U;

    if ((nullptr != data) &&
        (0U < size))
    {
        m_data.insert(m_data.end(), data, data + size);
        written = size;
    }

    return written;
}

size_t AsyncUDPPacket::write(const uint8_t* data, size_t size)
{
    size_t sent = 0U;

    if (nullptr != m_udp)
    {
        sent = m_udp->writeTo(data, size, m_remoteIP, m_remotePort);
    }

    return sent;
}

size_t AsyncUDPPacket::send(AsyncUDPMessage& message)
{
    return write(message.data(), message.length());
}

AsyncUDP::AsyncUDP() :
    m_socket(-1),
    m_localPort(0U),
    m_mutex(),
    m_handler(nullptr),
    m_handlerArg(nullptr),
    m_isSenderKnown(false),
    m_senderIP(),
    m_senderPort(0U),
    m_rxBuffer{ 0U }
{
    /* The event loop shall be destroyed after every statically allocated UDP
     * socket, therefore it must be constructed first.
     */
    (void)AsyncUdpLoop::getInstance();
}

AsyncUDP::~AsyncUDP()
{
    close();
}

bool AsyncUDP::listen(uint16_t port)
{
    return listen(IPAddress(), port);
}

bool AsyncUDP::listen(const IPAddress& addr, uint16_t port)
{
    bool isSuccessful = false;

    if (-1 != m_socket)
    {
        LOG_WARNING("UDP socket is already listening.");
    }
    else if (false == SocketCompat::init())
    {
        LOG_ERROR("Socket API not available.");
    }
    else
    {
        SocketCompat::Socket sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if (SocketCompat::INVALID_SOCKET_HANDLE == sock)
        {
            LOG_ERROR("Failed to create the UDP socket.");
        }
        else
        {
            struct sockaddr_in sockAddr;
            socklen_t          sockAddrLen = static_cast<socklen_t>(sizeof(sockAddr));

            memset(&sockAddr, 0, sizeof(sockAddr));
            sockAddr.sin_family      = AF_INET;
            sockAddr.sin_port        = htons(port);
            sockAddr.sin_addr.s_addr = htonl(static_cast<uint32_t>(addr));

            if (0 != bind(sock, reinterpret_cast<struct sockaddr*>(&sockAddr), sizeof(sockAddr)))
            {
                LOG_ERROR("Failed to bind the UDP socket to port %u.", port);
                SocketCompat::closeSocket(sock);
            }
            else if (false == SocketCompat::setNonBlocking(sock))
            {
                LOG_ERROR("Failed to set the UDP socket non-blocking.");
                SocketCompat::closeSocket(sock);
            }
            else
            {
                SocketCompat::setBroadcast(sock);
                SocketCompat::disableUdpConnReset(sock);

                /* If port 0 was given, the OS selected one. */
                if (0 == getsockname(sock, reinterpret_cast<struct sockaddr*>(&sockAddr), &sockAddrLen))
                {
                    port = ntohs(sockAddr.sin_port);
                }

                {
                    std::lock_guard<std::mutex> guard(m_mutex);

                    m_localPort     = port;
                    m_isSenderKnown = false;
                }

                m_socket = static_cast<int>(sock);

                AsyncUdpLoop::getInstance().registerSocket(this);

                LOG_INFO("UDP socket is listening on port %u.", port);

                isSuccessful = true;
            }
        }
    }

    return isSuccessful;
}

void AsyncUDP::close()
{
    int sock = m_socket.exchange(-1);

    if (-1 != sock)
    {
        /* It returns after a running packet handler finished, so the socket
         * is not closed while it is used.
         */
        AsyncUdpLoop::getInstance().unregisterSocket(this);

        SocketCompat::closeSocket(static_cast<SocketCompat::Socket>(sock));
    }
}

bool AsyncUDP::connected() const
{
    return (-1 != m_socket);
}

void AsyncUDP::onPacket(AuPacketHandlerFunctionWithArg cb, void* arg)
{
    std::lock_guard<std::mutex> guard(m_mutex);

    m_handler    = cb;
    m_handlerArg = arg;
}

void AsyncUDP::onPacket(AuPacketHandlerFunction cb)
{
    onPacket([cb](void* arg, AsyncUDPPacket& packet) {
        (void)arg;

        if (nullptr != cb)
        {
            cb(packet);
        }
    },
        nullptr);
}

size_t AsyncUDP::writeTo(const uint8_t* data, size_t size, const IPAddress& addr, uint16_t port)
{
    size_t sent = 0U;
    int    sock = m_socket;

    if ((nullptr == data) ||
        (0U == size) ||
        (-1 == sock))
    {
        /* Nothing to send or not listening. */
    }
    else
    {
        struct sockaddr_in sockAddr;
        int                result = 0;

        memset(&sockAddr, 0, sizeof(sockAddr));
        sockAddr.sin_family      = AF_INET;
        sockAddr.sin_port        = htons(port);
        sockAddr.sin_addr.s_addr = htonl(static_cast<uint32_t>(addr));

        result                   = static_cast<int>(sendto(static_cast<SocketCompat::Socket>(sock), reinterpret_cast<const char*>(data), static_cast<int>(size), 0, reinterpret_cast<struct sockaddr*>(&sockAddr), sizeof(sockAddr)));

        if (0 < result)
        {
            sent = static_cast<size_t>(result);
        }
        else
        {
            LOG_WARNING("Failed to send UDP packet to %s:%u.", addr.toString().c_str(), port);
        }
    }

    return sent;
}

size_t AsyncUDP::sendTo(AsyncUDPMessage& message, const IPAddress& addr, uint16_t port)
{
    return writeTo(message.data(), message.length(), addr, port);
}

size_t AsyncUDP::send(AsyncUDPMessage& message)
{
    size_t    sent          = 0U;
    bool      isSenderKnown = false;
    IPAddress senderIP;
    uint16_t  senderPort = 0U;

    {
        std::lock_guard<std::mutex> guard(m_mutex);

        isSenderKnown = m_isSenderKnown;
        senderIP      = m_senderIP;
        senderPort    = m_senderPort;
    }

    if (true == isSenderKnown)
    {
        sent = sendTo(message, senderIP, senderPort);
    }

    return sent;
}

size_t AsyncUDP::broadcastTo(const uint8_t* data, size_t size, uint16_t port)
{
    return writeTo(data, size, IPAddress(static_cast<uint32_t>(0xFFFFFFFFUL)), port);
}

size_t AsyncUDP::broadcast(const uint8_t* data, size_t size)
{
    uint16_t localPort = 0U;

    {
        std::lock_guard<std::mutex> guard(m_mutex);

        localPort = m_localPort;
    }

    return broadcastTo(data, size, localPort);
}

size_t AsyncUDP::broadcast(AsyncUDPMessage& message)
{
    return broadcast(message.data(), message.length());
}

void AsyncUDP::process()
{
    size_t count  = 0U;
    bool   isDone = false;

    while ((false == isDone) && (MAX_PACKETS_PER_CYCLE > count))
    {
        int                sock = m_socket;
        int                size = -1;
        struct sockaddr_in from;
        socklen_t          fromLen = static_cast<socklen_t>(sizeof(from));

        ++count;

        memset(&from, 0, sizeof(from));

        if (-1 != sock)
        {
            size = static_cast<int>(recvfrom(static_cast<SocketCompat::Socket>(sock), reinterpret_cast<char*>(m_rxBuffer), static_cast<int>(RX_BUFFER_SIZE), 0, reinterpret_cast<struct sockaddr*>(&from), &fromLen));
        }

        /* Nothing more to receive or the socket is closed. A zero length
         * datagram is valid for UDP.
         */
        if (0 > size)
        {
            isDone = true;
        }
        else
        {
            AuPacketHandlerFunctionWithArg handler;
            void*                          handlerArg = nullptr;
            uint16_t                       localPort  = 0U;
            IPAddress                      remoteIP(static_cast<uint32_t>(ntohl(from.sin_addr.s_addr)));
            uint16_t                       remotePort = ntohs(from.sin_port);

            {
                std::lock_guard<std::mutex> guard(m_mutex);

                handler         = m_handler;
                handlerArg      = m_handlerArg;
                localPort       = m_localPort;
                m_isSenderKnown = true;
                m_senderIP      = remoteIP;
                m_senderPort    = remotePort;
            }

            if (nullptr != handler)
            {
                AsyncUDPPacket packet(this, m_rxBuffer, static_cast<size_t>(size), remoteIP, remotePort, IPAddress(), localPort);

                handler(handlerArg, packet);
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

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

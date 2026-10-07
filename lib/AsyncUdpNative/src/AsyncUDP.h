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
 * @file   AsyncUDP.h
 * @brief  Asynchronous UDP for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * Counterpart of the ESP32 Async UDP library. It provides the subset of the
 * interface, which is used by the application, but on top of sockets instead
 * of lwIP.
 *
 * Like the target, every packet handler is called from one single thread, see
 * AsyncUdpLoop. Sending is allowed from any thread.
 *
 * Differences to the target:
 * - AsyncUDP::send() and AsyncUDPPacket::send() reply to the sender of the
 *   last received packet, because there is no connect().
 * - AsyncUDPPacket::isBroadcast() and AsyncUDPPacket::isMulticast() are always
 *   false.
 * - A received datagram is limited to RX_BUFFER_SIZE byte. A larger one is
 *   dropped on Windows and truncated on Linux, while lwIP would provide it
 *   completely. In both cases a warning is logged.
 * - AsyncUDPPacket::localIP() is always 0.0.0.0, because the socket is not
 *   bound to a dedicated network interface.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef ASYNC_UDP_H
#define ASYNC_UDP_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <atomic>
#include <functional>
#include <mutex>
#include <vector>

/* Note, Arduino.h is deliberately not included here. It defines the F() macro
 * and a boolean type, which collide with the Windows SDK headers that winsock
 * pulls in. See SocketCompat.hpp, which must be included before Arduino.h.
 */
#include <IPAddress.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

class AsyncUDP;
class AsyncUDPPacket;

/** Handler which is called after a packet is received. */
typedef std::function<void(AsyncUDPPacket& packet)> AuPacketHandlerFunction;

/** Handler which is called after a packet is received, with a user argument. */
typedef std::function<void(void* arg, AsyncUDPPacket& packet)> AuPacketHandlerFunctionWithArg;

/**
 * A message, which shall be sent.
 */
class AsyncUDPMessage
{
public:

    /**
     * Constructs an empty message.
     */
    AsyncUDPMessage() :
        m_data()
    {
    }

    /**
     * Destroys the message.
     */
    ~AsyncUDPMessage()
    {
    }

    /**
     * Append data to the message.
     *
     * @param[in] data  Data buffer
     * @param[in] size  Data buffer size in byte
     *
     * @return Number of appended bytes.
     */
    size_t write(const uint8_t* data, size_t size);

    /**
     * Get the message size.
     *
     * @return Message size in byte
     */
    size_t length() const
    {
        return m_data.size();
    }

    /**
     * Get the message data.
     *
     * @return Message data
     */
    uint8_t* data()
    {
        return m_data.data();
    }

private:

    std::vector<uint8_t> m_data; /**< The message data. */
};

/**
 * A received packet.
 * The data is only valid inside the packet handler.
 */
class AsyncUDPPacket
{
public:

    /**
     * Constructs a packet.
     *
     * @param[in] udp           The UDP socket, which received the packet.
     * @param[in] data          Data buffer
     * @param[in] size          Data buffer size in byte
     * @param[in] remoteIP      IP address of the sender.
     * @param[in] remotePort    Port of the sender.
     * @param[in] localIP       Local IP address.
     * @param[in] localPort     Local port.
     */
    AsyncUDPPacket(AsyncUDP* udp, uint8_t* data, size_t size, const IPAddress& remoteIP, uint16_t remotePort, const IPAddress& localIP, uint16_t localPort) :
        m_udp(udp),
        m_data(data),
        m_size(size),
        m_remoteIP(remoteIP),
        m_remotePort(remotePort),
        m_localIP(localIP),
        m_localPort(localPort)
    {
    }

    /**
     * Destroys the packet.
     */
    ~AsyncUDPPacket()
    {
    }

    /**
     * Get the packet data.
     *
     * @return Packet data
     */
    uint8_t* data()
    {
        return m_data;
    }

    /**
     * Get the packet size.
     *
     * @return Packet size in byte
     */
    size_t length() const
    {
        return m_size;
    }

    /**
     * Get the IP address of the sender.
     *
     * @return IP address
     */
    IPAddress remoteIP() const
    {
        return m_remoteIP;
    }

    /**
     * Get the port of the sender.
     *
     * @return Port number
     */
    uint16_t remotePort() const
    {
        return m_remotePort;
    }

    /**
     * Get the local IP address.
     *
     * @return IP address
     */
    IPAddress localIP() const
    {
        return m_localIP;
    }

    /**
     * Get the local port.
     *
     * @return Port number
     */
    uint16_t localPort() const
    {
        return m_localPort;
    }

    /**
     * Was the packet sent as broadcast?
     *
     * @return Always false in the native environment.
     */
    bool isBroadcast() const
    {
        return false;
    }

    /**
     * Was the packet sent as multicast?
     *
     * @return Always false in the native environment.
     */
    bool isMulticast() const
    {
        return false;
    }

    /**
     * Reply to the sender.
     *
     * @param[in] data  Data buffer
     * @param[in] size  Data buffer size in byte
     *
     * @return Number of sent bytes.
     */
    size_t write(const uint8_t* data, size_t size);

    /**
     * Reply to the sender.
     *
     * @param[in] message   The message.
     *
     * @return Number of sent bytes.
     */
    size_t send(AsyncUDPMessage& message);

private:

    AsyncUDP* m_udp;        /**< The UDP socket, which received the packet. */
    uint8_t*  m_data;       /**< Data buffer */
    size_t    m_size;       /**< Data buffer size in byte */
    IPAddress m_remoteIP;   /**< IP address of the sender. */
    uint16_t  m_remotePort; /**< Port of the sender. */
    IPAddress m_localIP;    /**< Local IP address. */
    uint16_t  m_localPort;  /**< Local port. */
};

/**
 * Asynchronous UDP socket.
 */
class AsyncUDP
{
public:

    /**
     * Constructs the UDP socket.
     */
    AsyncUDP();

    /**
     * Destroys the UDP socket.
     */
    ~AsyncUDP();

    /**
     * Start listening on all interfaces.
     *
     * @param[in] port  Port number
     *
     * @return If successful, it will return true otherwise false.
     */
    bool listen(uint16_t port);

    /**
     * Start listening.
     *
     * @param[in] addr  IP address, the socket binds to.
     * @param[in] port  Port number
     *
     * @return If successful, it will return true otherwise false.
     */
    bool listen(const IPAddress& addr, uint16_t port);

    /**
     * Stop listening.
     * It waits until a running packet handler returned.
     */
    void close();

    /**
     * Is the socket listening?
     *
     * @return If listening, it will return true otherwise false.
     */
    bool connected() const;

    /**
     * Set the handler which is called after a packet is received.
     *
     * @param[in] cb    The handler.
     * @param[in] arg   User specific argument.
     */
    void onPacket(AuPacketHandlerFunctionWithArg cb, void* arg = nullptr);

    /**
     * Set the handler which is called after a packet is received.
     *
     * @param[in] cb    The handler.
     */
    void onPacket(AuPacketHandlerFunction cb);

    /**
     * Send data to the given destination.
     *
     * @param[in] data  Data buffer
     * @param[in] size  Data buffer size in byte
     * @param[in] addr  Destination IP address
     * @param[in] port  Destination port
     *
     * @return Number of sent bytes, 0 in case of an error.
     */
    size_t writeTo(const uint8_t* data, size_t size, const IPAddress& addr, uint16_t port);

    /**
     * Send a message to the given destination.
     *
     * @param[in] message   The message.
     * @param[in] addr      Destination IP address
     * @param[in] port      Destination port
     *
     * @return Number of sent bytes, 0 in case of an error.
     */
    size_t sendTo(AsyncUDPMessage& message, const IPAddress& addr, uint16_t port);

    /**
     * Send a message to the sender of the last received packet.
     *
     * @param[in] message   The message.
     *
     * @return Number of sent bytes, 0 in case of an error.
     */
    size_t send(AsyncUDPMessage& message);

    /**
     * Broadcast data to the given port.
     *
     * @param[in] data  Data buffer
     * @param[in] size  Data buffer size in byte
     * @param[in] port  Destination port
     *
     * @return Number of sent bytes, 0 in case of an error.
     */
    size_t broadcastTo(const uint8_t* data, size_t size, uint16_t port);

    /**
     * Broadcast data to the local port.
     *
     * @param[in] data  Data buffer
     * @param[in] size  Data buffer size in byte
     *
     * @return Number of sent bytes, 0 in case of an error.
     */
    size_t broadcast(const uint8_t* data, size_t size);

    /**
     * Broadcast a message to the local port.
     *
     * @param[in] message   The message.
     *
     * @return Number of sent bytes, 0 in case of an error.
     */
    size_t broadcast(AsyncUDPMessage& message);

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
     * Receive the pending packets and notify the packet handler.
     * Called by the event loop only.
     */
    void process();

private:

    /** Max. size of a received packet in byte. */
    static const size_t RX_BUFFER_SIZE                   = 1500U;

    /** Max. number of packets, which are handled per event loop cycle. */
    static const size_t            MAX_PACKETS_PER_CYCLE = 32U;

    std::atomic<int>               m_socket;                   /**< Socket */
    uint16_t                       m_localPort;                /**< Local port, protected by m_mutex. */
    mutable std::mutex             m_mutex;                    /**< Protects the handler, the local port and the last sender. */
    AuPacketHandlerFunctionWithArg m_handler;                  /**< Packet handler, protected by m_mutex. */
    void*                          m_handlerArg;               /**< User argument of the packet handler, protected by m_mutex. */
    bool                           m_isSenderKnown;            /**< Was a packet received? Protected by m_mutex. */
    IPAddress                      m_senderIP;                 /**< IP address of the last sender, protected by m_mutex. */
    uint16_t                       m_senderPort;               /**< Port of the last sender, protected by m_mutex. */
    uint8_t                        m_rxBuffer[RX_BUFFER_SIZE]; /**< Receive buffer, used by the event loop only. */

    AsyncUDP(const AsyncUDP& udp);
    AsyncUDP& operator=(const AsyncUDP& udp);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* ASYNC_UDP_H */

/** @} */

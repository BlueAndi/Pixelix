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
 * @file   TestAsyncUdpNative.cpp
 * @brief  Test the native AsyncUDP with a loopback.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
/* The socket headers must come first, see AsyncUDP.cpp. */
#include <SocketCompat.hpp>

#include <unity.h>
#include <AsyncUDP.h>
#include <Util.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/** Collects the packets, which are received by the AsyncUDP. */
class PacketCollector
{
public:

    PacketCollector() :
        m_mutex(),
        m_count(0U),
        m_data(),
        m_remotePort(0U),
        m_replyText()
    {
    }

    std::mutex       m_mutex;      /**< Protects the members. */
    std::atomic<int> m_count;      /**< Number of received packets. */
    std::string      m_data;       /**< Data of the last packet. */
    uint16_t         m_remotePort; /**< Remote port of the last packet. */
    std::string      m_replyText;  /**< If not empty, the packet is answered with it. */
};

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static void testReceive();
static void testReplyToSender();
static void testSendToLastSender();
static void testBroadcast();
static void testRelisten();

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Port of the UDP server under test. */
static const uint16_t TEST_PORT    = 45048U;

/** Max. time to wait for an event in ms. */
static const uint32_t TEST_TIMEOUT = 2000U;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/**
 * Main entry point
 *
 * @param[in] argc  Number of command line arguments
 * @param[in] argv  Command line arguments
 */
extern int main(int argc, char** argv)
{
    UTIL_NOT_USED(argc);
    UTIL_NOT_USED(argv);

    UNITY_BEGIN();

    RUN_TEST(testReceive);
    RUN_TEST(testReplyToSender);
    RUN_TEST(testSendToLastSender);
    RUN_TEST(testBroadcast);
    RUN_TEST(testRelisten);

    return UNITY_END();
}

/**
 * Setup a test. This function will be called before every test by unity.
 */
extern void setUp(void)
{
    TEST_ASSERT_TRUE(SocketCompat::init());
}

/**
 * Clean up test. This function will be called after every test by unity.
 */
extern void tearDown(void)
{
    /* Not used. */
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Create a UDP client socket, which is bound to a free local port.
 *
 * @return Socket
 */
static SocketCompat::Socket createClient()
{
    SocketCompat::Socket sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in   addr;

    TEST_ASSERT_NOT_EQUAL(SocketCompat::INVALID_SOCKET_HANDLE, sock);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(0);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    TEST_ASSERT_EQUAL_INT(0, bind(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)));

    return sock;
}

/**
 * Send a datagram to the UDP server under test.
 *
 * @param[in] sock  The client socket.
 * @param[in] text  The text to send.
 */
static void sendToServer(SocketCompat::Socket sock, const std::string& text)
{
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(TEST_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(text.size()), static_cast<int>(sendto(sock, text.c_str(), static_cast<int>(text.size()), 0, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr))));
}

/**
 * Receive a datagram on the client socket.
 *
 * @param[in]  sock The client socket.
 * @param[out] text The received text.
 *
 * @return If received, it will return true otherwise false.
 */
static bool receiveFromServer(SocketCompat::Socket sock, std::string& text)
{
    bool isReceived = false;

    if (true == SocketCompat::waitFor(sock, TEST_TIMEOUT, false))
    {
        char buffer[128U];
        int  size = static_cast<int>(recvfrom(sock, buffer, sizeof(buffer), 0, nullptr, nullptr));

        if (0 <= size)
        {
            text       = std::string(buffer, static_cast<size_t>(size));
            isReceived = true;
        }
    }

    return isReceived;
}

/**
 * Wait until the given number of packets are received.
 *
 * @param[in] collector The packet collector.
 * @param[in] count     Expected number of packets.
 *
 * @return If received, it will return true otherwise false.
 */
static bool waitForPackets(const PacketCollector& collector, int count)
{
    uint32_t waited = 0U;

    while ((count > collector.m_count) && (TEST_TIMEOUT > waited))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        waited += 5U;
    }

    return (count <= collector.m_count);
}

/**
 * Register the collector as packet handler.
 *
 * @param[in] udp       The UDP server.
 * @param[in] collector The packet collector.
 */
static void registerCollector(AsyncUDP& udp, PacketCollector& collector)
{
    udp.onPacket([](void* arg, AsyncUDPPacket& packet) {
        PacketCollector* tthis = static_cast<PacketCollector*>(arg);

        if (nullptr != tthis)
        {
            std::string replyText;

            {
                std::lock_guard<std::mutex> guard(tthis->m_mutex);

                tthis->m_data       = std::string(reinterpret_cast<const char*>(packet.data()), packet.length());
                tthis->m_remotePort = packet.remotePort();
                replyText           = tthis->m_replyText;
            }

            if (false == replyText.empty())
            {
                AsyncUDPMessage message;

                (void)message.write(reinterpret_cast<const uint8_t*>(replyText.c_str()), replyText.size());
                (void)packet.send(message);
            }

            ++tthis->m_count;
        }
    },
        &collector);
}

/**
 * Test the reception of a datagram.
 */
static void testReceive()
{
    PacketCollector      collector; /* Must outlive the UDP server, which calls its handler. */
    AsyncUDP             udp;
    SocketCompat::Socket client = createClient();

    TEST_ASSERT_TRUE(udp.listen(TEST_PORT));
    TEST_ASSERT_TRUE(udp.connected());
    registerCollector(udp, collector);

    sendToServer(client, "hello");

    TEST_ASSERT_TRUE(waitForPackets(collector, 1));

    {
        std::lock_guard<std::mutex> guard(collector.m_mutex);

        TEST_ASSERT_EQUAL_STRING("hello", collector.m_data.c_str());
        TEST_ASSERT_NOT_EQUAL(0U, collector.m_remotePort);
    }

    udp.close();
    TEST_ASSERT_FALSE(udp.connected());

    SocketCompat::closeSocket(client);
}

/**
 * Test the reply by the packet.
 */
static void testReplyToSender()
{
    PacketCollector      collector;
    AsyncUDP             udp;
    SocketCompat::Socket client = createClient();
    std::string          answer;

    collector.m_replyText = "pong";

    TEST_ASSERT_TRUE(udp.listen(TEST_PORT));
    registerCollector(udp, collector);

    sendToServer(client, "ping");

    TEST_ASSERT_TRUE(receiveFromServer(client, answer));
    TEST_ASSERT_EQUAL_STRING("pong", answer.c_str());

    SocketCompat::closeSocket(client);
}

/**
 * Test the message sending to the sender of the last received packet.
 */
static void testSendToLastSender()
{
    PacketCollector      collector;
    AsyncUDP             udp;
    SocketCompat::Socket client = createClient();
    AsyncUDPMessage      message;
    std::string          answer;
    const char*          TEXT = "reply";

    TEST_ASSERT_TRUE(udp.listen(TEST_PORT));
    registerCollector(udp, collector);

    /* Nobody has sent something yet. */
    TEST_ASSERT_EQUAL_UINT32(5U, message.write(reinterpret_cast<const uint8_t*>(TEXT), 5U));
    TEST_ASSERT_EQUAL_UINT32(0U, udp.send(message));

    sendToServer(client, "hello");
    TEST_ASSERT_TRUE(waitForPackets(collector, 1));

    TEST_ASSERT_EQUAL_UINT32(5U, udp.send(message));
    TEST_ASSERT_TRUE(receiveFromServer(client, answer));
    TEST_ASSERT_EQUAL_STRING("reply", answer.c_str());

    SocketCompat::closeSocket(client);
}

/**
 * Test that a broadcast doesn't fail, if the socket is listening.
 * The destination is the local port, the datagram may be received by itself.
 */
static void testBroadcast()
{
    AsyncUDP        udp;
    AsyncUDPMessage message;
    const char*     TEXT = "bcast";

    TEST_ASSERT_EQUAL_UINT32(5U, message.write(reinterpret_cast<const uint8_t*>(TEXT), 5U));

    /* Not listening */
    TEST_ASSERT_EQUAL_UINT32(0U, udp.broadcast(message));

    TEST_ASSERT_TRUE(udp.listen(TEST_PORT));

    /* A host without any network interface can't send a broadcast. */
    size_t sent = udp.broadcast(message);

    TEST_ASSERT_TRUE((0U == sent) || (5U == sent));
}

/**
 * Test that the socket can be closed and reopened.
 */
static void testRelisten()
{
    AsyncUDP udp;

    TEST_ASSERT_TRUE(udp.listen(TEST_PORT));
    TEST_ASSERT_FALSE(udp.listen(TEST_PORT)); /* Already listening. */

    udp.close();
    TEST_ASSERT_FALSE(udp.connected());

    TEST_ASSERT_TRUE(udp.listen(TEST_PORT));
    TEST_ASSERT_TRUE(udp.connected());
}

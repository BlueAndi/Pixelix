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
 * @file   AsyncTcpLoop.cpp
 * @brief  Event loop of the native asynchronous TCP
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
/* The socket headers must come first, see AsyncTCP.cpp. */
#include <SocketCompat.hpp>

#include "AsyncTcpLoop.h"
#include "AsyncTCP.h"

#include <Arduino.h>
#include <algorithm>
#include <chrono>

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

void AsyncTcpLoop::registerServer(AsyncServer* server)
{
    if (nullptr != server)
    {
        {
            std::lock_guard<std::mutex> guard(m_mutex);

            if (m_servers.end() == std::find(m_servers.begin(), m_servers.end(), server))
            {
                m_servers.push_back(server);
            }
        }

        start();
    }
}

void AsyncTcpLoop::unregisterServer(AsyncServer* server)
{
    std::lock_guard<std::mutex>         guard(m_mutex);
    std::vector<AsyncServer*>::iterator it = std::find(m_servers.begin(), m_servers.end(), server);

    if (m_servers.end() != it)
    {
        (void)m_servers.erase(it);
    }
}

void AsyncTcpLoop::registerClient(AsyncClient* client)
{
    if (nullptr != client)
    {
        std::lock_guard<std::mutex> guard(m_mutex);

        if (m_clients.end() == std::find(m_clients.begin(), m_clients.end(), client))
        {
            m_clients.push_back(client);
        }
    }
}

void AsyncTcpLoop::unregisterClient(AsyncClient* client)
{
    std::lock_guard<std::mutex>         guard(m_mutex);
    std::vector<AsyncClient*>::iterator it = std::find(m_clients.begin(), m_clients.end(), client);

    if (m_clients.end() != it)
    {
        (void)m_clients.erase(it);
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void AsyncTcpLoop::start()
{
    std::lock_guard<std::mutex> guard(m_mutex);

    if (false == m_isRunning)
    {
        m_isRunning = true;
        m_lastPoll  = millis();
        m_thread    = std::thread(&AsyncTcpLoop::process, this);
    }
}

void AsyncTcpLoop::stop()
{
    bool isJoinable = false;

    {
        std::lock_guard<std::mutex> guard(m_mutex);

        m_isRunning = false;
        isJoinable  = m_thread.joinable();
    }

    if (true == isJoinable)
    {
        m_thread.join();
    }
}

void AsyncTcpLoop::process()
{
    while (true == m_isRunning)
    {
        fd_set                    readSet;
        fd_set                    writeSet;
        SocketCompat::Socket      maxSocket = 0;
        struct timeval            timeout;
        std::vector<AsyncServer*> servers;
        std::vector<AsyncClient*> clients;
        bool                      isPollDue    = false;
        bool                      isAckPending = false;

        /* Work on a copy, because a callback may register or unregister a
         * client and would modify the list while it is iterated.
         */
        {
            std::lock_guard<std::mutex> guard(m_mutex);

            servers = m_servers;
            clients = m_clients;
        }

        FD_ZERO(&readSet);
        FD_ZERO(&writeSet);

        for (AsyncServer* server : servers)
        {
            SocketCompat::Socket sock = static_cast<SocketCompat::Socket>(server->getSocket());

            if (SocketCompat::INVALID_SOCKET_HANDLE != sock)
            {
                FD_SET(sock, &readSet);

                if (maxSocket < sock)
                {
                    maxSocket = sock;
                }
            }
        }

        for (AsyncClient* client : clients)
        {
            SocketCompat::Socket sock = static_cast<SocketCompat::Socket>(client->getSocket());

            if (SocketCompat::INVALID_SOCKET_HANDLE != sock)
            {
                FD_SET(sock, &readSet);

                /* Only watch for writability if there is something to send,
                 * otherwise select() would return immediately all the time.
                 */
                if (true == client->hasPendingTxData())
                {
                    FD_SET(sock, &writeSet);
                }

                if (true == client->hasPendingAck())
                {
                    isAckPending = true;
                }

                if (maxSocket < sock)
                {
                    maxSocket = sock;
                }
            }
        }

        timeout.tv_sec = 0;

        /* A pending acknowledge is notified below in this cycle. Waiting for a
         * socket event first would just delay the next chunk of a response.
         */
        if (true == isAckPending)
        {
            timeout.tv_usec = 0L;
        }
        else
        {
            timeout.tv_usec = static_cast<long>(SELECT_PERIOD) * 1000L;
        }

        (void)select(static_cast<int>(maxSocket) + 1, &readSet, &writeSet, nullptr, &timeout);

        /* A pending connection is accepted first, so a new client is served in
         * the same cycle.
         */
        for (AsyncServer* server : servers)
        {
            SocketCompat::Socket sock = static_cast<SocketCompat::Socket>(server->getSocket());

            if ((SocketCompat::INVALID_SOCKET_HANDLE != sock) &&
                (0 != FD_ISSET(sock, &readSet)))
            {
                server->acceptClient();
            }
        }

        for (AsyncClient* client : clients)
        {
            SocketCompat::Socket sock = static_cast<SocketCompat::Socket>(client->getSocket());

            if (SocketCompat::INVALID_SOCKET_HANDLE != sock)
            {
                bool isReadable = (0 != FD_ISSET(sock, &readSet));
                bool isWritable = (0 != FD_ISSET(sock, &writeSet));

                if ((true == isReadable) ||
                    (true == isWritable))
                {
                    client->process(isReadable, isWritable);
                }
            }
        }

        /* The webserver expects a periodic poll, e.g. to send the next chunk of
         * a response or to detect a timeout.
         */
        if (POLL_PERIOD <= (millis() - m_lastPoll))
        {
            isPollDue  = true;
            m_lastPoll = millis();
        }

        /* The list is read again, because a client may have been closed by the
         * processing above.
         */
        {
            std::lock_guard<std::mutex> guard(m_mutex);

            clients = m_clients;
        }

        /* The acknowledge is notified here and not inside the write, like on
         * the target where the TCP/IP task notifies it. See
         * AsyncClient::notifyAck().
         */
        for (AsyncClient* client : clients)
        {
            client->notifyAck();
        }

        if (true == isPollDue)
        {
            /* The list is read again, because the acknowledge notification may
             * have closed a client.
             */
            {
                std::lock_guard<std::mutex> guard(m_mutex);

                clients = m_clients;
            }

            for (AsyncClient* client : clients)
            {
                client->notifyPoll();
            }
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

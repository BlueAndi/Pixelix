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
 * @file   AsyncUdpLoop.cpp
 * @brief  Event loop of the native asynchronous UDP
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
/* The socket headers must come first, see AsyncUDP.cpp. */
#include <SocketCompat.hpp>

#include "AsyncUdpLoop.h"
#include "AsyncUDP.h"

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

/** Is the current thread the loop thread? */
static thread_local bool isLoopThread = false;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void AsyncUdpLoop::registerSocket(AsyncUDP* udp)
{
    if (nullptr != udp)
    {
        {
            std::lock_guard<std::mutex> guard(m_mutex);

            if (m_sockets.end() == std::find(m_sockets.begin(), m_sockets.end(), udp))
            {
                m_sockets.push_back(udp);
            }
        }

        start();
    }
}

void AsyncUdpLoop::unregisterSocket(AsyncUDP* udp)
{
    if (nullptr != udp)
    {
        {
            /* Lock order: dispatch mutex first, like in process(). */
            std::lock_guard<std::recursive_mutex> dispatchGuard(m_dispatchMutex);
            std::lock_guard<std::mutex>           guard(m_mutex);
            std::vector<AsyncUDP*>::iterator      it = std::find(m_sockets.begin(), m_sockets.end(), udp);

            if (m_sockets.end() != it)
            {
                (void)m_sockets.erase(it);
            }
        }

        /* In the loop thread no select() is in progress. */
        if ((false == isLoopThread) &&
            (true == m_isRunning))
        {
            waitForCycle();
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void AsyncUdpLoop::start()
{
    std::lock_guard<std::mutex> guard(m_mutex);

    if (false == m_isRunning)
    {
        m_isRunning = true;
        m_thread    = std::thread(&AsyncUdpLoop::process, this);
    }
}

void AsyncUdpLoop::stop()
{
    bool isJoinable = false;

    {
        /* The thread handle is written by start() under the mutex, therefore
         * it must not be read without it.
         */
        std::lock_guard<std::mutex> guard(m_mutex);

        m_isRunning = false;
        isJoinable  = m_thread.joinable();
    }

    /* Join outside of the mutex, because the loop itself requires it. */
    if (true == isJoinable)
    {
        m_thread.join();
    }
}

bool AsyncUdpLoop::isRegistered(const AsyncUDP* udp)
{
    std::lock_guard<std::mutex> guard(m_mutex);

    return (m_sockets.end() != std::find(m_sockets.begin(), m_sockets.end(), udp));
}

void AsyncUdpLoop::notifyCycle()
{
    {
        std::lock_guard<std::mutex> guard(m_cycleMutex);

        ++m_cycle;
    }

    m_cycleCv.notify_all();
}

void AsyncUdpLoop::waitForCycle()
{
    std::unique_lock<std::mutex> lock(m_cycleMutex);
    uint32_t                     cycle = m_cycle;

    (void)m_cycleCv.wait_for(lock, std::chrono::milliseconds(static_cast<long long>(UNREGISTER_TIMEOUT)), [this, cycle]() { return (cycle != m_cycle); });
}

void AsyncUdpLoop::process()
{
    isLoopThread = true;

    while (true == m_isRunning)
    {
        fd_set                 readSet;
        SocketCompat::Socket   maxSocket = 0;
        size_t                 count     = 0U;
        struct timeval         timeout;
        std::vector<AsyncUDP*> sockets;

        FD_ZERO(&readSet);

        /* Work on a copy, because a handler may register or unregister a
         * socket and would modify the list while it is iterated. The dispatch
         * mutex keeps the sockets alive while they are accessed.
         */
        {
            std::lock_guard<std::recursive_mutex> dispatchGuard(m_dispatchMutex);

            {
                std::lock_guard<std::mutex> guard(m_mutex);

                sockets = m_sockets;
            }

            for (AsyncUDP* udp : sockets)
            {
                SocketCompat::Socket sock = static_cast<SocketCompat::Socket>(udp->getSocket());

                if (SocketCompat::INVALID_SOCKET_HANDLE != sock)
                {
                    FD_SET(sock, &readSet);
                    ++count;

                    if (maxSocket < sock)
                    {
                        maxSocket = sock;
                    }
                }
            }
        }

        /* On Windows select() fails immediately without any socket, which
         * would result in a busy loop.
         */
        if (0U == count)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long long>(SELECT_PERIOD)));

            notifyCycle();
        }
        else
        {
            timeout.tv_sec  = 0;
            timeout.tv_usec = static_cast<long>(SELECT_PERIOD) * 1000L;

            (void)select(static_cast<int>(maxSocket) + 1, &readSet, nullptr, nullptr, &timeout);

            notifyCycle();

            /* A socket may be unregistered by another thread during select().
             * The dispatch mutex ensures that it is not destroyed while its
             * handler is running.
             */
            std::lock_guard<std::recursive_mutex> dispatchGuard(m_dispatchMutex);

            for (AsyncUDP* udp : sockets)
            {
                if (true == isRegistered(udp))
                {
                    SocketCompat::Socket sock = static_cast<SocketCompat::Socket>(udp->getSocket());

                    if ((SocketCompat::INVALID_SOCKET_HANDLE != sock) &&
                        (0 != FD_ISSET(sock, &readSet)))
                    {
                        udp->process();
                    }
                }
            }
        }
    }

    /* No further cycle will be notified. A thread which unregisters a socket
     * right now shall not run into the timeout.
     */
    notifyCycle();
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

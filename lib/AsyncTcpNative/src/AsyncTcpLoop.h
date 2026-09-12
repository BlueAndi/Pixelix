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
 * @file   AsyncTcpLoop.h
 * @brief  Event loop of the native asynchronous TCP
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * The event loop owns one single thread, which watches every server and client
 * socket. Every callback of the AsyncTCP interface is called from this thread.
 *
 * That is essential: on the target all AsyncTCP callbacks run in one dedicated
 * task and the webserver is written for that guarantee. A thread per connection
 * would deliver the callbacks concurrently and corrupt its internal state.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef ASYNC_TCP_LOOP_H
#define ASYNC_TCP_LOOP_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <mutex>
#include <thread>
#include <vector>

class AsyncClient;
class AsyncServer;

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The event loop of the native asynchronous TCP.
 */
class AsyncTcpLoop
{
public:

    /**
     * Get the event loop instance.
     *
     * @return Event loop instance
     */
    static AsyncTcpLoop& getInstance()
    {
        static AsyncTcpLoop instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
    }

    /**
     * Register a server, whose listening socket shall be watched.
     * The thread is started with the first server.
     *
     * @param[in] server    The server.
     */
    void registerServer(AsyncServer* server);

    /**
     * Unregister a server.
     *
     * @param[in] server    The server.
     */
    void unregisterServer(AsyncServer* server);

    /**
     * Register a client, whose socket shall be watched.
     *
     * @param[in] client    The client.
     */
    void registerClient(AsyncClient* client);

    /**
     * Unregister a client.
     *
     * @param[in] client    The client.
     */
    void unregisterClient(AsyncClient* client);

private:

    /**
     * Period of the poll notification in ms.
     * The webserver drives the response chunking and its timeouts by it, like
     * the AsyncTCP counterpart does.
     */
    static const uint32_t POLL_PERIOD       = 125U;

    /** Max. time the loop waits for a socket event in ms. */
    static const uint32_t     SELECT_PERIOD = 25U;

    std::mutex                m_mutex;     /**< Protects the server and client lists. */
    std::vector<AsyncServer*> m_servers;   /**< The registered servers. */
    std::vector<AsyncClient*> m_clients;   /**< The registered clients. */
    std::thread               m_thread;    /**< The thread which runs the loop. */
    bool                      m_isRunning; /**< Is the loop running? */
    uint32_t                  m_lastPoll;  /**< Timestamp of the last poll notification in ms. */

    /**
     * Constructs the event loop.
     */
    AsyncTcpLoop() :
        m_mutex(),
        m_servers(),
        m_clients(),
        m_thread(),
        m_isRunning(false),
        m_lastPoll(0U)
    {
    }

    /**
     * Destroys the event loop.
     */
    ~AsyncTcpLoop()
    {
        stop();
    }

    AsyncTcpLoop(const AsyncTcpLoop& loop);
    AsyncTcpLoop& operator=(const AsyncTcpLoop& loop);

    /**
     * Start the thread, if not already running.
     */
    void start();

    /**
     * Stop the thread.
     */
    void stop();

    /**
     * The loop itself, which watches every socket.
     */
    void process();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* ASYNC_TCP_LOOP_H */

/** @} */

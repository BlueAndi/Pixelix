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
 * @file   AsyncUdpLoop.h
 * @brief  Event loop of the native asynchronous UDP
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * The event loop owns one single thread, which watches every UDP socket. Every
 * packet handler is called from this thread, like on the target.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef ASYNC_UDP_LOOP_H
#define ASYNC_UDP_LOOP_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

class AsyncUDP;

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The event loop of the native asynchronous UDP.
 */
class AsyncUdpLoop
{
public:

    /**
     * Get the event loop instance.
     *
     * @return Event loop instance
     */
    static AsyncUdpLoop& getInstance()
    {
        static AsyncUdpLoop instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
    }

    /**
     * Register a UDP socket, which shall be watched.
     * The thread is started with the first socket.
     *
     * @param[in] udp   The UDP socket.
     */
    void registerSocket(AsyncUDP* udp);

    /**
     * Unregister a UDP socket.
     * It waits until a running packet handler of any socket returned, so the
     * caller may destroy the socket afterwards. Calling it from inside a packet
     * handler is allowed.
     *
     * @param[in] udp   The UDP socket.
     */
    void unregisterSocket(AsyncUDP* udp);

private:

    /** Max. time the loop waits for a socket event in ms. */
    static const uint32_t  SELECT_PERIOD = 25U;

    std::mutex             m_mutex;         /**< Protects the socket list. */
    std::recursive_mutex   m_dispatchMutex; /**< Held while packet handlers are called. */
    std::vector<AsyncUDP*> m_sockets;       /**< The registered UDP sockets. */
    std::thread            m_thread;        /**< The thread which runs the loop. */
    std::atomic<bool>      m_isRunning;     /**< Is the loop running? */

    /**
     * Constructs the event loop.
     */
    AsyncUdpLoop() :
        m_mutex(),
        m_dispatchMutex(),
        m_sockets(),
        m_thread(),
        m_isRunning(false)
    {
    }

    /**
     * Destroys the event loop.
     */
    ~AsyncUdpLoop()
    {
        stop();
    }

    AsyncUdpLoop(const AsyncUdpLoop& loop);
    AsyncUdpLoop& operator=(const AsyncUdpLoop& loop);

    /**
     * Start the thread, if not already running.
     */
    void start();

    /**
     * Stop the thread.
     */
    void stop();

    /**
     * Is the given UDP socket registered?
     *
     * @param[in] udp   The UDP socket.
     *
     * @return If registered, it will return true otherwise false.
     */
    bool isRegistered(const AsyncUDP* udp);

    /**
     * The loop itself, which watches every socket.
     */
    void process();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* ASYNC_UDP_LOOP_H */

/** @} */

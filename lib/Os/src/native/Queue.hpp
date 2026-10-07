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
 * @file   Queue.hpp
 * @brief  Native queue wrapper
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * Native counterpart of the Os library queue wrapper. The interface is the
 * same, the implementation is based on the C++ standard library.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef QUEUE_HPP
#define QUEUE_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include "OsTypes.hpp"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Wrapper for the native queue.
 *
 * In difference to the freeRTOS counterpart the item is not copied byte wise,
 * but by its copy constructor. Therefore non-trivial item types are supported
 * as well.
 *
 * @tparam T    Item type
 */
template < typename T >
class Queue
{
public:

    /**
     * Create queue wrapper.
     */
    Queue() :
        m_mutex(),
        m_condVar(),
        m_items(),
        m_maxLength(0U),
        m_waiterCount(0U),
        m_isCreated(false)
    {
    }

    /**
     * Destroys queue wrapper.
     */
    ~Queue()
    {
        destroy();
    }

    /**
     * Create queue with given length.
     * If the queue is already created, it will fail.
     *
     * @param[in] length    Max. amount of items in the queue.
     *
     * @return If successful created, it will return true otherwise false.
     */
    bool create(size_t length)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        bool                        isSuccessful = false;

        if (true == m_isCreated)
        {
            /* Guard: the queue is already created. */
        }
        else if (0U == length)
        {
            /* Guard: a queue without any item is not supported. */
        }
        else
        {
            m_items.clear();
            m_maxLength  = length;
            m_isCreated  = true;
            isSuccessful = true;
        }

        return isSuccessful;
    }

    /**
     * Destroys the queue.
     * Blocked senders and receivers are released and will fail. It returns
     * after the last one left the queue.
     */
    void destroy()
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (true == m_isCreated)
        {
            m_isCreated = false;
            m_condVar.notify_all();

            /* Wait until all blocked senders and receivers left the queue. */
            m_condVar.wait(lock, [this]() { return (0U == m_waiterCount); });

            m_items.clear();
            m_maxLength = 0U;
        }
    }

    /**
     * Send item to the back of the queue.
     *
     * Note, set ticksToWait to portMAX_DELAY, will wait infinite.
     *
     * @param[in] itemToQueue   The item which to queue (by copy).
     * @param[in] ticksToWait   Ticks to wait until the item is queued.
     *
     * @return If the item is successful send to queue, it will return true otherwise false.
     */
    bool sendToBack(const T& itemToQueue, TickType_t ticksToWait)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        bool                         isSuccessful = false;

        if (false == waitForSpace(lock, ticksToWait))
        {
            /* Guard: the queue is not created or still full. */
        }
        else
        {
            m_items.push_back(itemToQueue);
            m_condVar.notify_all();

            isSuccessful = true;
        }

        return isSuccessful;
    }

    /**
     * Send item to the front of the queue.
     *
     * Note, set ticksToWait to portMAX_DELAY, will wait infinite.
     *
     * @param[in] itemToQueue   The item which to queue (by copy).
     * @param[in] ticksToWait   Ticks to wait until the item is queued.
     *
     * @return If the item is successful send to queue, it will return true otherwise false.
     */
    bool sendToFront(T& itemToQueue, TickType_t ticksToWait)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        bool                         isSuccessful = false;

        if (false == waitForSpace(lock, ticksToWait))
        {
            /* Guard: the queue is not created or still full. */
        }
        else
        {
            m_items.push_front(itemToQueue);
            m_condVar.notify_all();

            isSuccessful = true;
        }

        return isSuccessful;
    }

    /**
     * Receive item from the queue.
     *
     * Note, set ticksToWait to portMAX_DELAY, will wait infinite.
     *
     * @param[in] itemBuffer    The item buffer which the received item is copied to.
     * @param[in] ticksToWait   Ticks to wait until a item is received.
     *
     * @return If a item is successful received from queue, it will return true otherwise false.
     */
    bool receive(T* itemBuffer, TickType_t ticksToWait)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        bool                         isSuccessful = false;

        if (nullptr == itemBuffer)
        {
            /* Guard: invalid item buffer. */
        }
        else if (false == waitForItem(lock, ticksToWait))
        {
            /* Guard: the queue is not created or still empty. */
        }
        else
        {
            *itemBuffer = m_items.front();
            m_items.pop_front();
            m_condVar.notify_all();

            isSuccessful = true;
        }

        return isSuccessful;
    }

    /**
     * Peek item from Queue
     *
     * Note, set ticksToWait to portMAX_DELAY, will wait infinite.
     *
     * @param[in] itemBuffer The item buffer which the received item is copied to.
     * @param[in] ticksToWait Ticks to wait until a item is received.
     *
     * @return If a item is successful peeked from queue, it will return true otherwise false.
     */
    bool peek(T* itemBuffer, TickType_t ticksToWait)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        bool                         isSuccessful = false;

        if (nullptr == itemBuffer)
        {
            /* Guard: invalid item buffer. */
        }
        else if (false == waitForItem(lock, ticksToWait))
        {
            /* Guard: the queue is not created or still empty. */
        }
        else
        {
            *itemBuffer  = m_items.front();

            isSuccessful = true;
        }

        return isSuccessful;
    }

private:

    std::mutex              m_mutex;       /**< Protects the internal state. */
    std::condition_variable m_condVar;     /**< Signals a state change of the queue. */
    std::deque<T>           m_items;       /**< The queued items. */
    size_t                  m_maxLength;   /**< Max. amount of items in the queue. */
    uint32_t                m_waiterCount; /**< Number of blocked senders and receivers. */
    bool                    m_isCreated;   /**< Is the queue created? */

    Queue(const Queue& queue);
    Queue& operator=(const Queue& queue);

    /**
     * Wait until the queue is not full anymore, the queue is destroyed or the
     * timeout expired. The given lock must be locked and will be locked again
     * on return.
     *
     * @param[in] lock          Lock of the internal mutex.
     * @param[in] ticksToWait   Ticks to wait until space is available.
     *
     * @return If space for a item is available, it will return true otherwise false.
     */
    bool waitForSpace(std::unique_lock<std::mutex>& lock, TickType_t ticksToWait)
    {
        return waitFor(lock, ticksToWait, false);
    }

    /**
     * Wait until the queue is not empty anymore, the queue is destroyed or the
     * timeout expired. The given lock must be locked and will be locked again
     * on return.
     *
     * @param[in] lock          Lock of the internal mutex.
     * @param[in] ticksToWait   Ticks to wait until a item is available.
     *
     * @return If a item is available, it will return true otherwise false.
     */
    bool waitForItem(std::unique_lock<std::mutex>& lock, TickType_t ticksToWait)
    {
        return waitFor(lock, ticksToWait, true);
    }

    /**
     * Wait until the queue provides a item resp. space for a item, the queue is
     * destroyed or the timeout expired. The given lock must be locked and will
     * be locked again on return.
     *
     * @param[in] lock          Lock of the internal mutex.
     * @param[in] ticksToWait   Ticks to wait until the queue is ready.
     * @param[in] waitForItem   If true it waits for a item, otherwise for space for a item.
     *
     * @return If the queue is ready, it will return true otherwise false.
     */
    bool waitFor(std::unique_lock<std::mutex>& lock, TickType_t ticksToWait, bool waitForItem)
    {
        bool isReady = false;

        if (false == m_isCreated)
        {
            /* Guard: the queue is not created. */
        }
        else
        {
            /* The destroy() method waits until every blocked sender and
             * receiver left the queue.
             */
            ++m_waiterCount;

            if (portMAX_DELAY == ticksToWait)
            {
                m_condVar.wait(lock, [this, waitForItem]() { return isWaitFinished(waitForItem); });
            }
            else
            {
                const std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(pdTICKS_TO_MS(ticksToWait));

                (void)m_condVar.wait_until(lock, deadline, [this, waitForItem]() { return isWaitFinished(waitForItem); });
            }

            isReady = ((true == m_isCreated) && (true == isQueueReady(waitForItem)));

            --m_waiterCount;

            if (0U == m_waiterCount)
            {
                m_condVar.notify_all();
            }
        }

        return isReady;
    }

    /**
     * Is the wait for the queue finished?
     *
     * @param[in] waitForItem   If true it waits for a item, otherwise for space for a item.
     *
     * @return If the queue is ready or destroyed, it will return true otherwise false.
     */
    bool isWaitFinished(bool waitForItem) const
    {
        return ((false == m_isCreated) || (true == isQueueReady(waitForItem)));
    }

    /**
     * Does the queue provide a item resp. space for a item?
     *
     * @param[in] waitForItem   If true it checks for a item, otherwise for space for a item.
     *
     * @return If the queue is ready, it will return true otherwise false.
     */
    bool isQueueReady(bool waitForItem) const
    {
        bool isReady = false;

        if (true == waitForItem)
        {
            isReady = (false == m_items.empty());
        }
        else
        {
            isReady = (m_items.size() < m_maxLength);
        }

        return isReady;
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* QUEUE_HPP */

/** @} */

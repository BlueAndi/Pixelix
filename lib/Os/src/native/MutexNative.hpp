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
 * @file   MutexNative.hpp
 * @brief  Native mutex core, which emulates the freeRTOS mutex behaviour
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef MUTEX_NATIVE_HPP
#define MUTEX_NATIVE_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include "OsTypes.hpp"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The native mutex core, which emulates the freeRTOS mutex behaviour on top of
 * the C++ standard library. It keeps track of the owning thread, therefore
 * giving a mutex which is not owned will fail instead of causing undefined
 * behaviour.
 *
 * @note This is an internal helper of the native Os implementation. Use Mutex,
 *       MutexRecursive or CriticalSection instead.
 */
class MutexNative
{
public:

    /**
     * Creates the native mutex core.
     *
     * @param[in] isRecursive   If true, the owning thread may take the mutex more than once.
     */
    MutexNative(bool isRecursive) :
        m_mutex(),
        m_condVar(),
        m_owner(),
        m_isRecursive(isRecursive),
        m_count(0U)
    {
    }

    /**
     * Destroys the native mutex core.
     */
    ~MutexNative()
    {
    }

    /**
     * Take the mutex.
     * If blockTime is portMAX_DELAY, it will wait infinite.
     *
     * @note A non-recursive mutex, which is taken twice by the same thread, will
     *       block like the freeRTOS counterpart does.
     *
     * @param[in] blockTime Max. time in ticks, it shall wait for the mutex.
     *
     * @return If mutex is taken, it will return true otherwise false.
     */
    bool take(TickType_t blockTime)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        const std::thread::id        currentThreadId = std::this_thread::get_id();
        bool                         isSuccessful    = false;

        if ((0U < m_count) &&
            (currentThreadId == m_owner) &&
            (true == m_isRecursive))
        {
            ++m_count;
            isSuccessful = true;
        }
        else
        {
            if (portMAX_DELAY == blockTime)
            {
                m_condVar.wait(lock, [this]() { return (0U == m_count); });
                isSuccessful = true;
            }
            else
            {
                const std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(pdTICKS_TO_MS(blockTime));

                isSuccessful                                         = m_condVar.wait_until(lock, deadline, [this]() { return (0U == m_count); });
            }

            if (true == isSuccessful)
            {
                m_owner = currentThreadId;
                m_count = 1U;
            }
        }

        return isSuccessful;
    }

    /**
     * Give the mutex.
     * Only the owning thread is able to give the mutex back.
     *
     * @return If mutex is given, it will return true otherwise false.
     */
    bool give()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        bool                        isSuccessful = false;

        if (0U == m_count)
        {
            /* Guard: the mutex is not taken at all. */
        }
        else if (std::this_thread::get_id() != m_owner)
        {
            /* Guard: the mutex is owned by another thread. */
        }
        else
        {
            --m_count;

            if (0U == m_count)
            {
                m_owner = std::thread::id();

                /* Notify all, because a waiting thread may run into its timeout
                 * in parallel and would consume the notification without taking
                 * the mutex.
                 */
                m_condVar.notify_all();
            }

            isSuccessful = true;
        }

        return isSuccessful;
    }

private:

    std::mutex              m_mutex;       /**< Protects the internal state. */
    std::condition_variable m_condVar;     /**< Signals that the mutex is available. */
    std::thread::id         m_owner;       /**< Id of the thread which owns the mutex. */
    bool                    m_isRecursive; /**< Is the mutex recursive? */
    uint32_t                m_count;       /**< Number of times the owner took the mutex. */

    /* An instance shall not be copied. */
    MutexNative(const MutexNative& mutex);
    MutexNative& operator=(const MutexNative& mutex);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* MUTEX_NATIVE_HPP */

/** @} */

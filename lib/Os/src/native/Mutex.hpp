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
 * @file   Mutex.hpp
 * @brief  Native mutex wrapper
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * Native counterpart of the Os library mutex wrapper. The interface is the
 * same, the implementation is based on the C++ standard library.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef MUTEX_HPP
#define MUTEX_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <new>
#include "MutexNative.hpp"
#include "OsTypes.hpp"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Wrapper for the native mutex.
 */
class Mutex
{
public:

    /**
     * Create mutex wrapper.
     */
    Mutex() :
        m_mutex(nullptr)
    {
    }

    /**
     * Destroys mutex wrapper.
     */
    ~Mutex()
    {
        destroy();
    }

    /**
     * Create mutex.
     *
     * @return If successful created, it will return true otherwise false.
     */
    bool create()
    {
        bool isSuccessful = false;

        if (nullptr == m_mutex)
        {
            m_mutex = new (std::nothrow) MutexNative(false);

            if (nullptr != m_mutex)
            {
                isSuccessful = true;
            }
        }

        return isSuccessful;
    }

    /**
     * Destroys the mutex.
     */
    void destroy()
    {
        if (nullptr != m_mutex)
        {
            delete m_mutex;
            m_mutex = nullptr;
        }
    }

    /**
     * Is mutex allocated?
     *
     * @return If mutex is allocated it will return true otherwise false.
     */
    bool isAllocated() const
    {
        return (nullptr != m_mutex);
    }

    /**
     * Take the mutex.
     * If blockTime is portMAX_DELAY, it will wait infinite.
     *
     * @param[in] blockTime Max. time in ticks, it shall wait for the mutex.
     *
     * @return If mutex is taken, it will return true otherwise false.
     */
    bool take(TickType_t blockTime)
    {
        bool isSuccessful = false;

        if (nullptr != m_mutex)
        {
            isSuccessful = m_mutex->take(blockTime);
        }

        return isSuccessful;
    }

    /**
     * Give the mutex.
     *
     * @return If mutex is given, it will return true otherwise false.
     */
    bool give()
    {
        bool isSuccessful = false;

        if (nullptr != m_mutex)
        {
            isSuccessful = m_mutex->give();
        }

        return isSuccessful;
    }

private:

    MutexNative* m_mutex; /**< Mutex */

    Mutex(const Mutex& mutex);
    Mutex& operator=(const Mutex& mutex);
};

/**
 * Wrapper for the native recursive mutex.
 */
class MutexRecursive
{
public:

    /**
     * Create mutex wrapper.
     */
    MutexRecursive() :
        m_mutex(nullptr)
    {
    }

    /**
     * Destroys mutex wrapper.
     */
    ~MutexRecursive()
    {
        destroy();
    }

    /**
     * Create mutex.
     *
     * @return If successful created, it will return true otherwise false.
     */
    bool create()
    {
        bool isSuccessful = false;

        if (nullptr == m_mutex)
        {
            m_mutex = new (std::nothrow) MutexNative(true);

            if (nullptr != m_mutex)
            {
                isSuccessful = true;
            }
        }

        return isSuccessful;
    }

    /**
     * Destroys the mutex.
     */
    void destroy()
    {
        if (nullptr != m_mutex)
        {
            delete m_mutex;
            m_mutex = nullptr;
        }
    }

    /**
     * Is mutex allocated?
     *
     * @return If mutex is allocated it will return true otherwise false.
     */
    bool isAllocated() const
    {
        return (nullptr != m_mutex);
    }

    /**
     * Take the mutex.
     * If blockTime is portMAX_DELAY, it will wait infinite.
     *
     * @param[in] blockTime Max. time in ticks, it shall wait for the mutex.
     *
     * @return If mutex is taken, it will return true otherwise false.
     */
    bool take(TickType_t blockTime)
    {
        bool isSuccessful = false;

        if (nullptr != m_mutex)
        {
            isSuccessful = m_mutex->take(blockTime);
        }

        return isSuccessful;
    }

    /**
     * Give the mutex.
     *
     * @return If mutex is given, it will return true otherwise false.
     */
    bool give()
    {
        bool isSuccessful = false;

        if (nullptr != m_mutex)
        {
            isSuccessful = m_mutex->give();
        }

        return isSuccessful;
    }

private:

    MutexNative* m_mutex; /**< Mutex */

    MutexRecursive(const MutexRecursive& mutex);
    MutexRecursive& operator=(const MutexRecursive& mutex);
};

/**
 * The mutex guard takes the mutex at creation and gives it back during
 * destruction.
 *
 * @tparam T    Kind of mutex
 */
template < typename T >
class MutexGuard
{
public:

    /**
     * Creates the mutex guard and takes the mutex.
     * It will wait infinite, till it gets it.
     *
     * @param[in] mutex The guard uses this mutex for protection.
     */
    MutexGuard(T& mutex) :
        m_mutex(mutex)
    {
        (void)this->m_mutex.take(portMAX_DELAY);
    }

    /**
     * Destroys the mutex guard and gives the mutex back.
     */
    ~MutexGuard()
    {
        (void)this->m_mutex.give();
    }

private:

    T& m_mutex; /**< Mutex used for the guard. */

    MutexGuard();
    MutexGuard(const MutexGuard& guard);
    MutexGuard& operator=(const MutexGuard& guard);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* MUTEX_HPP */

/** @} */

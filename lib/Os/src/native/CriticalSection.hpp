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
 * @file   CriticalSection.hpp
 * @brief  Native critical section wrapper
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * Native counterpart of the Os library critical section wrapper. The interface
 * is the same, the implementation is based on the C++ standard library.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef CRITICAL_SECTION_HPP
#define CRITICAL_SECTION_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "MutexNative.hpp"
#include "OsTypes.hpp"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Wrapper for the native critical section.
 *
 * @note On the host there are no interrupts to disable, therefore the critical
 *       section is emulated by a recursive mutex. Like the freeRTOS spinlock
 *       counterpart it protects concurrent access, but it will not prevent the
 *       thread from being preempted.
 */
class CriticalSection
{
public:

    /**
     * Create critical section wrapper.
     */
    CriticalSection() :
        m_mutex(true)
    {
    }

    /**
     * Destroys critical section wrapper.
     */
    ~CriticalSection()
    {
    }

    /**
     * Enter the critical section.
     */
    void enter()
    {
        (void)m_mutex.take(portMAX_DELAY);
    }

    /**
     * Exit critical section.
     */
    void exit()
    {
        (void)m_mutex.give();
    }

private:

    MutexNative m_mutex; /**< Recursive mutex, which emulates the spinlock. */

    CriticalSection(const CriticalSection& CriticalSection);
    CriticalSection& operator=(const CriticalSection& CriticalSection);
};

/**
 * The critical section guard enters the critical section at creation and exits during
 * destruction.
 */
class CriticalSectionGuard
{
public:

    /**
     * Creates the critical section guard and enters it.
     *
     * @param[in] critSec The guard uses this critical section for protection.
     */
    CriticalSectionGuard(CriticalSection& critSec) :
        m_criticalSection(critSec)
    {
        m_criticalSection.enter();
    }

    /**
     * Destroys the critical section guard and exits the critical section.
     */
    ~CriticalSectionGuard()
    {
        m_criticalSection.exit();
    }

private:

    CriticalSection& m_criticalSection; /**< Critical section used for the guard. */

    CriticalSectionGuard();
    CriticalSectionGuard(const CriticalSectionGuard& guard);
    CriticalSectionGuard& operator=(const CriticalSectionGuard& guard);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* CRITICAL_SECTION_HPP */

/** @} */

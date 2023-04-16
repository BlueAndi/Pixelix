/* MIT License
 *
 * Copyright (c) 2019 - 2023 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Task monitor
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup common
 *
 * @{
 */

#ifndef TASK_MON_H
#define TASK_MON_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <SysMsgPlugin.h>
#include <WString.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Task monitor
 */
class TaskMon
{
public:

    /**
     * Get task monitor instance.
     *
     * @return Task monitor instance
     */
    static TaskMon& getInstance()
    {
        static TaskMon instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
    }

    /**
     * Get current number of tasks and their properties.
     */
    void process();

    /** Processing cycle in ms. */
    static const uint32_t PROCESSING_CYCLE  = 60U * 1000U;

private:

    SimpleTimer m_timer;    /**< Timer used for cyclic processing. */

    /**
     * Constructs the task monitor.
     */
    TaskMon()
    {
    }

    /**
     * Destroys the task monitor.
     */
    ~TaskMon()
    {
        /* Will never be called. */
    }

    TaskMon(const TaskMon& taskMon);
    TaskMon& operator=(const TaskMon& taskMon);

#if configUSE_TRACE_FACILITY

    /**
     * Get task state as user friendly string.
     *
     * @return Task state name
     */
    const char* taskState2Str(eTaskState state);

    /**
     * Fill string up with spaces until given length is reached.
     *
     * @param[in] str   String, which to fill up
     * @param[in] len   Length of the result string
     *
     * @return Filled up string
     */
    String fillUpSpaces(const char* str, size_t len);

#endif  /* configUSE_TRACE_FACILITY */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* TASK_MON_H */

/** @} */
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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "TaskMon.h"

#include <Logging.h>

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

void TaskMon::process()
{
#if configUSE_TRACE_FACILITY
    bool isProcessingTime = false;

    if (false == m_timer.isTimerRunning())
    {
        m_timer.start(PROCESSING_CYCLE);
        isProcessingTime = true;
    }
    else if (true == m_timer.isTimeout())
    {
        isProcessingTime = true;
        m_timer.restart();
    }

    if (true == isProcessingTime)
    {
        UBaseType_t     numOfTasks      = uxTaskGetNumberOfTasks();
        TaskStatus_t*   taskStatus      = new(std::nothrow) TaskStatus_t[numOfTasks];

        if (nullptr != taskStatus)
        {
            uint32_t    totalRunTime    = 0U;
            UBaseType_t index           = 0U;
            size_t      taskNameMaxLen  = 0U;
            size_t      taskStateMaxLen = 0U;

            numOfTasks = uxTaskGetSystemState(taskStatus, numOfTasks, &totalRunTime);

            /* Determine the length of the longest task name and the longest task state name. */
            for(index = 0U; index < numOfTasks; ++index)
            {
                size_t taskNameLen  = strlen(taskStatus[index].pcTaskName);
                size_t taskStateLen = taskState2Str(taskStatus[index].eCurrentState).length();

                if (taskNameMaxLen < taskNameLen)
                {
                    taskNameMaxLen = taskNameLen;
                }

                if (taskStateMaxLen < taskStateLen)
                {
                    taskStateMaxLen = taskStateLen;
                }
            }

            /* Show task informations */
            for(index = 0U; index < numOfTasks; ++index)
            {
                uint32_t statsAsPercentage = 0U;

                /* Calculate task load in percent, if total run time is available.
                 * Note, this depends on how FreeRTOS is configured.
                 */
                if (0U < totalRunTime)
                {
                    statsAsPercentage = taskStatus[index].ulRunTimeCounter / (totalRunTime / 100U);
                }

#if configTASKLIST_INCLUDE_COREID
                LOG_DEBUG("Task \"%s\": c %d, p %2u, %s, %3u%%, stack high water mark: %u",
                    fillUpSpaces(taskStatus[index].pcTaskName, taskNameMaxLen).c_str(),
                    taskStatus[index].xCoreID,
                    taskStatus[index].uxCurrentPriority,
                    fillUpSpaces(taskState2Str(taskStatus[index].eCurrentState).c_str(), taskStateMaxLen).c_str(),
                    statsAsPercentage,
                    taskStatus[index].usStackHighWaterMark);
    #else
                LOG_DEBUG("Task \"%s\": p %2u, %s, %3u%%, stack high water mark: %u",
                    fillUpSpaces(taskStatus[index].pcTaskName, taskNameMaxLen).c_str(),
                    taskStatus[index].uxCurrentPriority,
                    fillUpSpaces(taskState2Str(taskStatus[index].eCurrentState).c_str(), taskStateMaxLen).c_str(),
                    statsAsPercentage,
                    taskStatus[index].usStackHighWaterMark);
#endif
            }

            delete[] taskStatus;
        }
    }
#endif  /* configUSE_TRACE_FACILITY */
}

#if configUSE_TRACE_FACILITY

const char* TaskMon::taskState2Str(eTaskState state)
{
    const char* name = "";

    switch(state)
    {
    /* A task is querying the state of itself, so must be running. */
	case eRunning:
        name = "Running";
        break;

    /* The task being queried is in a read or pending ready list. */
	case eReady:
        name = "Ready";
        break;

    /* The task being queried is in the Blocked state. */
	case eBlocked:
        name = "Blocked";
        break;

    /* The task being queried is in the Suspended state, or is in the Blocked state with an infinite time out. */
	case eSuspended:
        name = "Suspended";
        break;

    /* The task being queried has been deleted, but its TCB has not yet been freed. */
	case eDeleted:
        name = "Deleted";
        break;

    default:
        break;
    }

    return name;
}

String TaskMon::fillUpSpaces(const char* str, size_t len)
{
    String  result      = str;
    size_t  resultLen   = result.length();

    if (len > result.length())
    {
        size_t index = 0U;

        for(index = 0U; index < (len - resultLen); ++index)
        {
            result += " ";
        }
    }

    return result;
}

#endif  /* configUSE_TRACE_FACILITY */

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

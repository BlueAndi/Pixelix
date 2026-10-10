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
 * @file   Task.hpp
 * @brief  freeRTOS task wrapper
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup OPERATING_SYSTEM
 *
 * @{
 */

#ifndef TASK_HPP
#define TASK_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <functional>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The task class provides two methods to process a task.
 *
 * The first method is the default process() method, which can be overridden by
 * the derived class.
 *
 * The second method is a task function, which can be set by the user of the
 * class.
 *
 * In general the task function and the process() method shall be cooperative,
 * because the internal low level task handles the task exit request.
 *
 * @tparam T   Type of the task parameters.
 */
template <typename T>
class Task
{
public:

    /**
     * Task function type, which can be used to set a custom task function.
     *
     * @param[in] parameters    Parameters for the task function.
     */
    typedef std::function<void(T* parameters)> TaskFunction;

    /**
     * Constructs the task with a empty task function.
     * Its expected that the derived class will override the process() method.
     *
     * @param[in] name      Name of the task.
     * @param[in] stackSize Stack size in bytes. Optional, default is 4096 bytes.
     * @param[in] priority  Task priority. Optional, default is 1.
     * @param[in] core      Core on which the task shall run. Optional, default is APP_CPU_NUM.
     */
    Task(const char* name, uint32_t stackSize = DEFAULT_STACK_SIZE, UBaseType_t priority = DEFAULT_PRIORITY, BaseType_t core = DEFAULT_CORE) :
        m_name(name),
        m_function(nullptr),
        m_stackSize(stackSize),
        m_priority(priority),
        m_core(core),
        m_taskHandle(nullptr),
        m_parameters(nullptr),
        m_exitSemaphore(nullptr),
        m_reqExit(false)
    {
    }

    /**
     * Constructs the task with a custom task function.
     *
     * The default process() method will not be called, but the provided task
     * function will be called periodically.
     *
     * @param[in] name      Name of the task.
     * @param[in] function  Task function to be called periodically.
     * @param[in] stackSize Stack size in bytes. Optional, default is 4096 bytes.
     * @param[in] priority  Task priority. Optional, default is 1.
     * @param[in] core      Core on which the task shall run. Optional, default is APP_CPU_NUM.
     */
    Task(const char* name, TaskFunction function, uint32_t stackSize = DEFAULT_STACK_SIZE, UBaseType_t priority = DEFAULT_PRIORITY, BaseType_t core = DEFAULT_CORE) :
        m_name(name),
        m_function(function),
        m_stackSize(stackSize),
        m_priority(priority),
        m_core(core),
        m_taskHandle(nullptr),
        m_parameters(nullptr),
        m_exitSemaphore(nullptr),
        m_reqExit(false)
    {
    }

    /**
     * Destroys the task.
     */
    virtual ~Task()
    {
        stop();
    }

    /**
     * Start the task.
     *
     * @param[in] parameters    Parameters for the task function.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool start(T* parameters);

    /**
     * Stop the task.
     * The task will be requested to exit and the task handle will be deleted.
     * If the task does not exit within the default timeout, it will be aborted.
     *
     * @param[in] timeout    Timeout in ms for task stop (optional).
     *
     * @return If successful, it will return true otherwise false.
     */
    bool stop(uint32_t timeout = DEFAULT_TIMEOUT);

    /**
     * Is the task running?
     *
     * @return If the task is running, it will return true otherwise false.
     */
    bool isRunning() const;

    /**
     * Default stack size in bytes.
     */
    static const uint32_t DEFAULT_STACK_SIZE  = 4096U;

    /**
     * Default task priority.
     */
    static const UBaseType_t DEFAULT_PRIORITY = 1U;

    /**
     * Default task core, which is the APP CPU core.
     * This is the default core on which the task shall run.
     */
    static const BaseType_t DEFAULT_CORE      = APP_CPU_NUM;

    /**
     * Default timeout in ms for task stop.
     * If the task does not stop within this time, it will be aborted.
     */
    static const uint32_t DEFAULT_TIMEOUT     = 2000U;

protected:

    /**
     * The default process method, which can be overridden by the derived class.
     *
     * @param[in] parameters    Parameters for the process method.
     */
    virtual void process(T* parameters)
    {
        (void)parameters;
    }

private:

    const char*       m_name;          /**< Name of the task */
    TaskFunction      m_function;      /**< Task function to be called periodically, if set */
    uint32_t          m_stackSize;     /**< Stack size in bytes */
    UBaseType_t       m_priority;      /**< Task priority */
    BaseType_t        m_core;          /**< Core on which the task shall run */
    TaskHandle_t      m_taskHandle;    /**< Task handle */
    T*                m_parameters;    /**< Task parameters */
    SemaphoreHandle_t m_exitSemaphore; /**< Binary semaphore to signal that the task has exited. */
    bool              m_reqExit;       /**< Request exit signal. */

    /**
     * Default constructor not allowed.
     */
    Task()                            = delete;

    /**
     * Copy constructor not allowed.
     *
     * @param[in] task    Task to copy.
     */
    Task(const Task& task)            = delete;

    /**
     * Assignment operator not allowed.
     *
     * @param[in] task    Task to assign.
     *
     * @return Reference to this task.
     */
    Task& operator=(const Task& task) = delete;

    /**
     * Low level task function, which is called by the OS.
     * It will call the process() method or the task function, if set.
     *
     * @param[in] parameters    Parameters for the task function.
     */
    static void lowLevelTaskFunction(void* parameters);
};

template <typename T>
bool Task<T>::start(T* parameters)
{
    bool status = false;

    /* Task not started yet? */
    if (nullptr == m_taskHandle)
    {
        m_parameters    = parameters;
        m_reqExit       = false;

        /* Create a semaphore to signal the task exit, if not created yet. */
        m_exitSemaphore = xSemaphoreCreateBinary();

        if (nullptr != m_exitSemaphore)
        {
            /* The semaphore must be given before it can be taken by the task. */
            if (pdTRUE == xSemaphoreGive(m_exitSemaphore))
            {
                /* Create the task. */
                BaseType_t result = xTaskCreateUniversal(
                    lowLevelTaskFunction,
                    m_name,
                    m_stackSize,
                    this,
                    m_priority,
                    &m_taskHandle,
                    m_core);

                if (pdPASS == result)
                {
                    status = true;
                }
            }
        }

        if (false == status)
        {
            /* If the task creation failed, delete the semaphore. */
            if (nullptr != m_exitSemaphore)
            {
                vSemaphoreDelete(m_exitSemaphore);
                m_exitSemaphore = nullptr;
            }

            m_taskHandle = nullptr;
        }
    }

    return status;
}

template <typename T>
bool Task<T>::stop(uint32_t timeout)
{
    bool status = false;

    if (nullptr != m_taskHandle)
    {
        /* Request the task to exit. */
        m_reqExit = true;

        /* Wait until the task has exited.
         * We don't care about the return value, because the task
         * will be aborted if it does not exit within the timeout.
         */
        if (pdFALSE == xSemaphoreTake(m_exitSemaphore, pdMS_TO_TICKS(timeout)))
        {
            /* Abort task. */
            vTaskDelete(m_taskHandle);
        }

        m_taskHandle = nullptr;

        vSemaphoreDelete(m_exitSemaphore);
        m_exitSemaphore = nullptr;

        status          = true;
    }

    return status;
}

template <typename T>
bool Task<T>::isRunning() const
{
    bool isRunning = false;

    if (nullptr != m_taskHandle)
    {
        isRunning = true;
    }

    return isRunning;
}

template <typename T>
void Task<T>::lowLevelTaskFunction(void* parameters)
{
    Task* task = static_cast<Task*>(parameters);

    if ((nullptr != task) &&
        (nullptr != task->m_exitSemaphore))
    {
        /* Grab the semaphore which is used later to signal that the
         * task exited.
         */
        (void)xSemaphoreTake(task->m_exitSemaphore, portMAX_DELAY);

        /* If a task function is available, it will be called instead of
         * the default process method.
         */
        if (nullptr != task->m_function)
        {
            while (false == task->m_reqExit)
            {
                task->m_function(task->m_parameters);
            }
        }
        else
        {
            while (false == task->m_reqExit)
            {
                /* Call the default process method. */
                task->process(task->m_parameters);
            }
        }

        /* Signal that the task has exited. */
        (void)xSemaphoreGive(task->m_exitSemaphore);
    }

    /* Delete the task itself. */
    vTaskDelete(nullptr);
}

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* TASK_HPP */

/** @} */
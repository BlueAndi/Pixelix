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
 * @brief  Native task wrapper
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * Native counterpart of the Os library task wrapper. The interface is the
 * same, the implementation is based on the C++ standard library.
 *
 * @addtogroup TEST
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
#include <chrono>
#include <condition_variable>
#include <exception>
#include <functional>
#include <memory>
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
 * @note On the host the stack size, the priority and the core are not used,
 *       because the underlying thread is scheduled by the host operating
 *       system. They are part of the interface to stay compatible to the
 *       target.
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
        m_parameters(nullptr),
        m_taskCtrl(nullptr),
        m_thread()
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
        m_parameters(nullptr),
        m_taskCtrl(nullptr),
        m_thread()
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
     *
     * @note In difference to the target, a task can not be killed on the host.
     *       A task which doesn't exit within the timeout, will be abandoned.
     *       It will not access this instance anymore, as soon as the pending
     *       process() call resp. task function call returned. But its thread
     *       keeps running until the process ends.
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
     * If the task does not stop within this time, it will be abandoned.
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

    /**
     * The task control block is shared between the task itself and its owner.
     * It survives an abandoned task, therefore the task will never access
     * released memory.
     */
    struct TaskCtrl
    {
        std::mutex              mutex;       /**< Protects the task control block. */
        std::condition_variable condVar;     /**< Signals a state change of the task. */
        bool                    reqExit;     /**< Request exit signal. */
        bool                    hasExited;   /**< The task exited. */
        bool                    isAbandoned; /**< The task didn't exit in time and is abandoned. */

        /**
         * Constructs the task control block.
         */
        TaskCtrl() :
            mutex(),
            condVar(),
            reqExit(false),
            hasExited(false),
            isAbandoned(false)
        {
        }
    };

    const char*               m_name;       /**< Name of the task */
    TaskFunction              m_function;   /**< Task function to be called periodically, if set */
    uint32_t                  m_stackSize;  /**< Stack size in bytes, not used on the host */
    UBaseType_t               m_priority;   /**< Task priority, not used on the host */
    BaseType_t                m_core;       /**< Core on which the task shall run, not used on the host */
    T*                        m_parameters; /**< Task parameters */
    std::shared_ptr<TaskCtrl> m_taskCtrl;   /**< Task control block, nullptr if the task is not running. */
    std::thread               m_thread;     /**< The thread which runs the task. */

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
     * @param[in] task      The task instance.
     * @param[in] taskCtrl  The task control block.
     */
    static void lowLevelTaskFunction(Task* task, std::shared_ptr<TaskCtrl> taskCtrl);
};

template <typename T>
bool Task<T>::start(T* parameters)
{
    bool status = false;

    /* Task not started yet? */
    if (nullptr == m_taskCtrl)
    {
        /* The C++ standard library signals errors by exception, which are
         * caught here to keep the interface compatible to the target.
         */
        try
        {
            m_parameters = parameters;
            m_taskCtrl   = std::shared_ptr<TaskCtrl>(new TaskCtrl());
            m_thread     = std::thread(&Task<T>::lowLevelTaskFunction, this, m_taskCtrl);

            status       = true;
        }
        catch (const std::exception& e)
        {
            (void)e;

            m_taskCtrl.reset();
            m_parameters = nullptr;
        }
    }

    return status;
}

template <typename T>
bool Task<T>::stop(uint32_t timeout)
{
    bool status = false;

    if (nullptr != m_taskCtrl)
    {
        std::shared_ptr<TaskCtrl> taskCtrl  = m_taskCtrl;
        bool                      hasExited = false;

        {
            std::unique_lock<std::mutex>                lock(taskCtrl->mutex);
            const std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);

            /* Request the task to exit. */
            taskCtrl->reqExit                                    = true;
            taskCtrl->condVar.notify_all();

            /* Wait until the task has exited. */
            hasExited = taskCtrl->condVar.wait_until(lock, deadline, [&taskCtrl]() { return taskCtrl->hasExited; });

            if (false == hasExited)
            {
                /* The task can not be killed on the host, therefore its
                 * abandoned. It will not access this instance anymore, as soon
                 * as the pending call returned.
                 */
                taskCtrl->isAbandoned = true;
            }
        }

        if (true == hasExited)
        {
            m_thread.join();

            m_parameters = nullptr;
            status       = true;
        }
        else
        {
            /* The task parameters are kept, because the abandoned task may
             * still use them until its pending call returned.
             */
            m_thread.detach();
        }

        m_taskCtrl.reset();
    }

    return status;
}

template <typename T>
bool Task<T>::isRunning() const
{
    bool isRunning = false;

    if (nullptr != m_taskCtrl)
    {
        isRunning = true;
    }

    return isRunning;
}

template <typename T>
void Task<T>::lowLevelTaskFunction(Task* task, std::shared_ptr<TaskCtrl> taskCtrl)
{
    if ((nullptr != task) &&
        (nullptr != taskCtrl))
    {
        bool isFinished = false;

        while (false == isFinished)
        {
            {
                std::lock_guard<std::mutex> lock(taskCtrl->mutex);

                isFinished = ((true == taskCtrl->reqExit) || (true == taskCtrl->isAbandoned));
            }

            if (false == isFinished)
            {
                /* If a task function is available, it will be called instead of
                 * the default process method.
                 */
                if (nullptr != task->m_function)
                {
                    task->m_function(task->m_parameters);
                }
                else
                {
                    /* Call the default process method. */
                    task->process(task->m_parameters);
                }
            }
        }

        /* Signal that the task has exited. */
        {
            std::lock_guard<std::mutex> lock(taskCtrl->mutex);

            taskCtrl->hasExited = true;
        }

        taskCtrl->condVar.notify_all();
    }
}

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* TASK_HPP */

/** @} */

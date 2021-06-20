/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Task proxy, used for task decoupling.
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup utilities
 *
 * @{
 */

#ifndef __TASK_PROXY_HPP__
#define __TASK_PROXY_HPP__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <Arduino.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The task proxy is used to decouple from tasks with different priority.
 * Especially in the case that priority inheritance shall be avoided.
 * 
 * @tparam T                Item type, which is transfered via queue
 * @tparam size             Max. number of items in the queue
 * @tparam waitTimeTicks    The time which shall be waited until a item is sent to queue or received from queue.
 */
template < typename T, size_t size, uint32_t waitTimeTicks >
class TaskProxy
{
public:

    /**
     * Create the task proxy with a empty queue.
     */
    TaskProxy() :
        m_queueHandle(xQueueCreate(size, sizeof(T)))
    {
    }

    /**
     * Destroy the task proxy.
     */
    ~TaskProxy()
    {
        if (nullptr != m_queueHandle)
        {
            vQueueDelete(m_queueHandle);
            m_queueHandle = nullptr;
        }
    }

    /**
     * Send a item to the queue. The item will be copied!
     * Attention: The copy takes place via memcpy, consider this in case of objects.
     * 
     * @param[in] item  The item, which to send.
     * 
     * @return If the item is successful sent, it will return true otherwise false.
     */
    bool send(const T& item)
    {
        bool isSuccessful = true;

        /* Queue item by copy. */
        if (pdTRUE != xQueueSendToBack(m_queueHandle, &item, waitTimeTicks))
        {
            isSuccessful = false;
        }

        return isSuccessful;
    }

    /**
     * Receive a item from the queue. The item will be copied, so enough space
     * must be provided!
     * 
     * @param[out] item The item, which will be overwritten from the queue.
     * 
     * @return If a item is successful received, it will return true otherwise false.
     */
    bool receive(T& item)
    {
        bool isSuccessful = true;

        /* Receive item by copy. */
        if (pdTRUE != xQueueReceive(m_queueHandle, &item, waitTimeTicks))
        {
            isSuccessful = false;
        }

        return isSuccessful;
    }

    /**
     * Reset the queue.
     * Attention: Every item in the queue gets lost!
     */
    void reset()
    {
        (void)xQueueReset(m_queueHandle);
    }

private:

    QueueHandle_t   m_queueHandle;  /**< Handle to queue with elements, used for decoupling from task. */

    TaskProxy(const TaskProxy& proxy);
    TaskProxy& operator=(const TaskProxy& proxy);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __TASK_PROXY_HPP__ */

/** @} */
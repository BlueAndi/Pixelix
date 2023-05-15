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
 * @brief  freeRTOS queue wrapper
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup os
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
#include <freertos/FreeRTOS.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Wrapper for the freeRTOS queue.
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
        m_queueHandle(nullptr)
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
        bool isSuccessful = false;

        if (nullptr == m_queueHandle)
        {
            m_queueHandle = xQueueCreate(length, sizeof(T));

            if (nullptr != m_queueHandle)
            {
                isSuccessful = true;
            }
        }

        return isSuccessful;
    }

    /**
     * Destroys the queue.
     */
    void destroy()
    {
        if (nullptr != m_queueHandle)
        {
            vQueueDelete(m_queueHandle);
            m_queueHandle = nullptr;
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
        bool isSuccessful = false;

        if (pdPASS == xQueueSendToBack(m_queueHandle, &itemToQueue, ticksToWait))
        {
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
        bool isSuccessful = false;

        if (pdPASS == xQueueSendToFront(m_queueHandle, &itemToQueue, ticksToWait))
        {
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
        bool isSuccessful = false;

        if (pdPASS == xQueueReceive(m_queueHandle, itemBuffer, ticksToWait))
        {
            isSuccessful = true;
        }

        return isSuccessful;
    }

private:

    QueueHandle_t   m_queueHandle;  /**< Queue handle */

    Queue(const Queue& queue);
    Queue& operator=(const Queue& queue);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* QUEUE_HPP */

/** @} */
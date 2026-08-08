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
 * @file   MemMon.cpp
 * @brief  Memory monitor
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MemMon.h"

#include <Logging.h>
#include <MemUtil.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>

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

bool MemMon::start()
{
    bool                    isSuccessful        = true;
    esp_alloc_failed_hook_t failedAllocCallback = [](size_t size, uint32_t capabilities, const char* functionName) -> void {
        TaskHandle_t hCurrentTask       = xTaskGetCurrentTaskHandle();
        const char*  taskName           = pcTaskGetName(hCurrentTask);
        UBaseType_t  stackHighWatermark = uxTaskGetStackHighWaterMark(hCurrentTask);

        LOG_ERROR("Memory allocation failed.");
        LOG_ERROR("Size          : %u bytes", size);
        LOG_ERROR("Capabilities  : 0x%04X", capabilities);
        LOG_ERROR("Function      : %s", functionName);
        LOG_ERROR("Task          : %s", (nullptr != taskName) ? taskName : "?");
        LOG_ERROR("Stack reserve : %u words", static_cast<unsigned>(stackHighWatermark));
        LOG_ERROR("Largest avail.: %u bytes", MemUtil::getLargestFreeBlockSize(capabilities));
    };

    m_timer.start(PROCESSING_CYCLE);

    if (ESP_OK != heap_caps_register_failed_alloc_callback(failedAllocCallback))
    {
        stop();
        isSuccessful = false;
    }

    return isSuccessful;
}

void MemMon::process()
{
    if (true == m_timer.isTimeout())
    {
        uint32_t availableHeap       = MemUtil::getFreeHeapSize();         /* Current available heap memory. */
        uint32_t lowestAvailableHeap = MemUtil::getMinFreeHeapSize();      /* Lowest level of available heap since boot. */
        uint32_t largestHeapBlock    = MemUtil::getLargestFreeBlockSize(); /* Largest block of heap that can be allocated at once. */

        if (MIN_HEAP_MEMORY >= availableHeap)
        {
            LOG_WARNING("Current available heap: %u byte.", availableHeap);
        }

        if (LOWEST_HEAP_MEMORY >= lowestAvailableHeap)
        {
            LOG_WARNING("Lowest available heap: %u byte.", lowestAvailableHeap);
        }

        if (LARGEST_HEAP_BLOCK_MEMORY > largestHeapBlock)
        {
            LOG_WARNING("Largest heap block which can be allocated: %u byte.", largestHeapBlock);
        }

        /* Any heap corrupt? */
        if (false == heap_caps_check_integrity_all(true))
        {
            LOG_FATAL("----- Heap corrupt! ------");
        }

#if (0 != CONFIG_MEM_MON_STACK_STATS)
        reportTaskStackStats();
#endif /* (0 != CONFIG_MEM_MON_STACK_STATS) */

        m_timer.restart();
    }
}

void MemMon::stop()
{
    m_timer.stop();
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

#if (0 != CONFIG_MEM_MON_STACK_STATS)

void MemMon::reportTaskStackStats()
{
    /* Names of the tasks whose stack size is worth watching and trimming. The
     * two framework tasks (loopTask, async_tcp) are the largest and are looked
     * up by name too. A name that does not exist in the current build (e.g.
     * AudioDrvTask) is simply skipped.
     *
     * uxTaskGetStackHighWaterMark() is used instead of uxTaskGetSystemState(),
     * because the latter needs configUSE_TRACE_FACILITY, which is disabled in
     * the Arduino ESP32 framework.
     */
    static const char* const taskNames[] = {
        "loopTask",    /* Arduino main loop */
        "async_tcp",   /* AsyncTCP, used by the web server */
        "processTask", /* DisplayMgr */
        "updateTask",  /* DisplayMgr */
        "buttonTask",  /* ButtonDrv */
        "HttpSrvWork", /* HttpService */
        "AudioDrvTask" /* AudioService (only in some builds) */
    };
    size_t idx;

    LOG_INFO("----- Task stack usage (min. free stack since start) -----");

    for (idx = 0U; idx < (sizeof(taskNames) / sizeof(taskNames[0])); ++idx)
    {
        TaskHandle_t taskHandle;

        /* xTaskGetHandle() asserts that the queried name is shorter than
         * configMAX_TASK_NAME_LEN. Skip any name that is too long, otherwise the
         * firmware would panic instead of just missing one line in the report.
         */
        if (configMAX_TASK_NAME_LEN <= strlen(taskNames[idx]))
        {
            continue;
        }

        taskHandle = xTaskGetHandle(taskNames[idx]);

        if (nullptr != taskHandle)
        {
            /* The high water mark is the smallest amount of free stack (in bytes
             * on ESP32) seen since the task started. A value near 0 means the
             * stack is almost exhausted and must not be reduced; a large value
             * is the headroom by which the stack size may be trimmed.
             */
            LOG_INFO("%-22s: %5u byte free",
                taskNames[idx],
                static_cast<unsigned>(uxTaskGetStackHighWaterMark(taskHandle)));
        }
    }
}

#endif /* (0 != CONFIG_MEM_MON_STACK_STATS) */

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

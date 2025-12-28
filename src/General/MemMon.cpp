/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
    esp_alloc_failed_hook_t failedAllocCallback = [](size_t size, uint32_t caps, const char* functionName) -> void {
        LOG_ERROR("Memory allocation failed.");
        LOG_ERROR("Size          : %u bytes", size);
        LOG_ERROR("Capability    : 0x%04X", caps);
        LOG_ERROR("Function      : %s", functionName);
        LOG_ERROR("Largest avail.: %u bytes", heap_caps_get_largest_free_block(MEM_CAPABILITIES));
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
        uint32_t availableHeap       = heap_caps_get_free_size(MEM_CAPABILITIES);          /* Current available heap memory. */
        uint32_t lowestAvailableHeap = heap_caps_get_minimum_free_size(MEM_CAPABILITIES);  /* Lowest level of available heap since boot. */
        uint32_t largestHeapBlock    = heap_caps_get_largest_free_block(MEM_CAPABILITIES); /* Largest block of heap that can be allocated at once. */

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

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

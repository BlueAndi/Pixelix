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
 * @file   MemUtil.cpp
 * @brief  Memory utility functions
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MemUtil.h"

#ifndef NATIVE
#include <esp_heap_caps.h>
#include <esp32-hal-psram.h>
#endif /* NATIVE */

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

#ifndef NATIVE

/**
 * Memory capabilities used for heap operations.
 */
static const uint32_t MEM_CAPABILITIES = MALLOC_CAP_INTERNAL | MALLOC_CAP_DEFAULT;

#endif /* NATIVE */

/******************************************************************************
 * Public Methods
 *****************************************************************************/

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

extern size_t MemUtil::getTotalHeapSize()
{
#ifndef NATIVE
    multi_heap_info_t info;

    heap_caps_get_info(&info, MEM_CAPABILITIES);

    return info.total_free_bytes + info.total_allocated_bytes;
#else  /* NATIVE */
    return 0;
#endif /* NATIVE */
}

extern size_t MemUtil::getFreeHeapSize()
{
#ifndef NATIVE
    return heap_caps_get_free_size(MEM_CAPABILITIES);
#else  /* NATIVE */
    return 0;
#endif /* NATIVE */
}

extern size_t MemUtil::getLargestFreeBlockSize()
{
#ifndef NATIVE
    return heap_caps_get_largest_free_block(MEM_CAPABILITIES);
#else  /* NATIVE */
    return 0;
#endif /* NATIVE */
}

extern size_t MemUtil::getMinFreeHeapSize()
{
#ifndef NATIVE
    return heap_caps_get_minimum_free_size(MEM_CAPABILITIES);
#else  /* NATIVE */
    return 0;
#endif /* NATIVE */
}

extern bool MemUtil::isPsramAvailable()
{
#ifndef NATIVE
    return psramFound();
#else  /* NATIVE */
    return false;
#endif /* NATIVE */
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/
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
 * @file   MemUtil.h
 * @brief  Memory utility functions
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup UTILITIES
 *
 * @{
 */

#ifndef MEMUTIL_H
#define MEMUTIL_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Memory utility functions */
namespace MemUtil
{

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Get total heap size which is available for malloc/new operation.
 *
 * @return Heap size in byte.
 */
extern size_t getTotalHeapSize();

/**
 * Get heap size which is available for malloc/new operation.
 *
 * @return Heap size in byte.
 */
extern size_t getFreeHeapSize();

/**
 * Get the largest free block of memory able to be allocated.
 *
 * @return Size of the largest free block in byte.
 */
extern size_t getLargestFreeBlockSize();

/**
 * Get the minimum free heap size since boot.
 * 
 * @return size_t 
 */
extern size_t getMinFreeHeapSize();

/**
 * Check if PSRAM is available.
 * 
 * @return true if PSRAM is available, false otherwise.
 */
extern bool isPsramAvailable();

} /* namespace MemUtil */

#endif /* MEMUTIL_H */

/** @} */
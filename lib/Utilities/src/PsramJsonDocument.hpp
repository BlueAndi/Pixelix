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
 * @file   PsramJsonDocument.hpp
 * @brief  JSON document which allocates its memory pool in PSRAM
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup UTILITIES
 *
 * @{
 */

#ifndef PSRAM_JSON_DOCUMENT_HPP
#define PSRAM_JSON_DOCUMENT_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdlib.h>
#include <ArduinoJson.h>

#ifndef NATIVE
#include <esp32-hal-psram.h>
#endif /* NATIVE */

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * ArduinoJson allocator which places the document memory pool in PSRAM if
 * available and falls back to the standard heap otherwise. This keeps the
 * internal RAM free for buffers which must not live in PSRAM (e.g. DMA).
 *
 * It is a stateless allocator, matching the ArduinoJson v6 allocator concept
 * (allocate/deallocate/reallocate).
 */
class PsramJsonAllocator
{
public:

    /**
     * Allocates a block of raw memory, preferring PSRAM.
     *
     * @param[in] size Size of the memory block to allocate in bytes.
     *
     * @return Pointer to allocated memory block or nullptr on error.
     */
    void* allocate(size_t size)
    {
        void* ptr = nullptr;

#ifndef NATIVE
        ptr = ps_malloc(size);
#endif /* NATIVE */

        if (nullptr == ptr)
        {
            ptr = malloc(size);
        }

        return ptr;
    }

    /**
     * Deallocates the allocated memory block.
     *
     * @param[in] ptr Pointer to memory block to deallocate.
     */
    void deallocate(void* ptr)
    {
        free(ptr);
    }

    /**
     * Resizes the allocated memory block, preferring PSRAM. On failure the
     * original block is left untouched, as with the standard realloc().
     *
     * @param[in] ptr   Pointer to memory block to resize.
     * @param[in] size  New size of the memory block in bytes.
     *
     * @return Pointer to resized memory block or nullptr on error.
     */
    void* reallocate(void* ptr, size_t size)
    {
        void* newPtr = nullptr;

#ifndef NATIVE
        newPtr = ps_realloc(ptr, size);
#endif /* NATIVE */

        if (nullptr == newPtr)
        {
            newPtr = realloc(ptr, size);
        }

        return newPtr;
    }
};

/**
 * JSON document which allocates its memory pool in PSRAM if available.
 * Use it as a drop-in replacement for DynamicJsonDocument.
 */
using PsramJsonDocument = BasicJsonDocument<PsramJsonAllocator>;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* PSRAM_JSON_DOCUMENT_HPP */

/** @} */

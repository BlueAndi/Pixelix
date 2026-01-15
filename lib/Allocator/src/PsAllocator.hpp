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
 * @file   PsAllocator.hpp
 * @brief  PSRAM first memory allocator
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup UTILITIES
 *
 * @{
 */

#ifndef PSALLOCATOR_HPP
#define PSALLOCATOR_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdlib.h>
#include "IAllocator.h"
#include "esp32-hal-psram.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * PSRAM first allocator which uses PSRAM if available, otherwise falls back to
 * standard allocation. It just allocates raw memory, but does not construct objects.
 * It uses ps_malloc() for PSRAM allocation and malloc() for standard allocation.
 * 
 * Its a stateless allocator.
 */
class PsAllocator : public IAllocator
{
public:

    /** Threshold size in bytes to decide whether to allocate in standard RAM or PSRAM. */
    static constexpr size_t THRESHOLD_STD_RAM = 128U;

    /**
     * Constructs the allocator.
     */
    PsAllocator() :
        IAllocator()
    {
    }

    /**
     * Destroys the allocator.
     */
    virtual ~PsAllocator()
    {
    }

    /**
     * Allocates a block of raw memory.
     * No exception is thrown on error, nullptr is returned instead.
     *
     * @param[in] size Size of the memory block to allocate in bytes.
     *
     * @return Pointer to allocated memory block or nullptr on error.
     */
    virtual void* allocate(size_t size) noexcept override
    {
        void* ptr = nullptr;

        if (THRESHOLD_STD_RAM <= size)
        {
            ptr = ps_malloc(size);
        }

        if (nullptr == ptr)
        {
            ptr = malloc(size);
        }

        return ptr;
    }

    /**
     * Deallocates the allocated memory block.
     * No exception is thrown on error.
     *
     * @param[in] ptr Pointer to memory block to deallocate.
     */
    virtual void deallocate(void* ptr) noexcept override
    {
        free(ptr);
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* PSALLOCATOR_HPP */

/** @} */
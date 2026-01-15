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
 * @file   IAllocator.h
 * @brief  Interface for memory allocators
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup UTILITIES
 *
 * @{
 */

#ifndef IALLOCATOR_H
#define IALLOCATOR_H

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

/**
 * Interface for memory allocators that just allocate raw memory, but do not
 * construct objects.
 */
class IAllocator
{
public:

    /**
     * Destroys the allocator.
     */
    virtual ~IAllocator()
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
    virtual void* allocate(size_t size) noexcept = 0;

    /**
     * Deallocates the allocated memory block.
     * No exception is thrown on error.
     *
     * @param[in] ptr Pointer to memory block to deallocate.
     */
    virtual void deallocate(void* ptr) noexcept = 0;

protected:

    /**
     * Constructs the allocator.
     */
    IAllocator()
    {
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* IALLOCATOR_H */

/** @} */
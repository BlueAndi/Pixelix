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
 * @file   TypedAllocator.hpp
 * @brief  Typed memory allocator
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup UTILITIES
 *
 * @{
 */

#ifndef TYPEDALLOCATOR_HPP
#define TYPEDALLOCATOR_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include "StdAllocator.hpp"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Typed memory allocator which constructs and destructs objects.
 * Uses an underlying allocator for raw memory allocation.
 *
 * If array allocation is used, the number of elements is stored at the beginning
 * of the allocated memory.
 * 
 * Its a stateless allocator.
 *
 * @tparam T                Type of the objects to allocate.
 * @tparam Allocator_Type   Type of the underlying allocator to use for raw memory allocation.
 *                          Default is StdAllocator.
 */
template <typename T, typename Allocator_Type = StdAllocator>
class TypedAllocator
{
public:

    /**
     * Constructs the typed allocator.
     */
    TypedAllocator()
    {
    }

    /**
     * Destroys the typed allocator.
     */
    ~TypedAllocator()
    {
    }

    /**
     * Allocates memory for a single object and constructs it.
     * No exception is thrown on error, nullptr is returned instead.
     *
     * @return Pointer to the constructed object or nullptr if allocation failed.
     */
    T* allocate() noexcept
    {
        Allocator_Type allocator;

        /* Allocate raw memory for single object. */
        void* vPtr = allocator.allocate(sizeof(T));
        T*    ptr  = nullptr;

        if (nullptr != vPtr)
        {
            /* Construct object in allocated memory by placement new. */
            ptr = new (vPtr) T();
        }

        return ptr;
    }

    /**
     * Deallocates the object and destructs it.
     * No exception is thrown on error.
     * Never use it for pointers to arrays!
     *
     * @param[in] ptr Pointer to the object to deallocate.
     */
    void deallocate(T* ptr) noexcept
    {
        if (nullptr != ptr)
        {
            Allocator_Type allocator;

            /* Destruct object explicitly. */
            ptr->~T();

            /* Free allocated memory. */
            allocator.deallocate(static_cast<void*>(ptr));
        }
    }

    /**
     * Allocates memory for an array of objects and constructs them.
     * No exception is thrown on error, nullptr is returned instead.
     *
     * @param[in] num Number of objects to allocate.
     *
     * @return Pointer to the first constructed object or nullptr if allocation failed.
     */
    T* allocateArray(size_t num) noexcept
    {
        T* ptr = nullptr;

        if (0U < num)
        {
            Allocator_Type allocator;

            /* Allocate raw memory for array of objects. */
            void* vPtr = allocator.allocate(sizeof(T) * num + sizeof(size_t));

            if (nullptr != vPtr)
            {
                /* Store number of elements at the beginning of allocated memory. */
                size_t* count = static_cast<size_t*>(vPtr);
                *count        = num;

                /* Adjust pointer to point to the first object. */
                vPtr          = static_cast<void*>(count + 1);
                ptr           = static_cast<T*>(vPtr);

                /* Construct each object in allocated memory by placement new. */
                for (size_t idx = 0; idx < num; ++idx)
                {
                    new (&ptr[idx]) T();
                }
            }
        }

        return ptr;
    }

    /**
     * Deallocates the array of objects and destructs them.
     * No exception is thrown on error.
     * Never use it for pointers to single objects!
     *
     * @param[in] ptr Pointer to the first object of the array to deallocate.
     */
    void deallocateArray(T* ptr) noexcept
    {
        if (nullptr != ptr)
        {
            Allocator_Type allocator;

            /* Get pointer to the stored number of elements. */
            void*   vPtr  = static_cast<void*>(ptr);
            size_t* count = static_cast<size_t*>(vPtr) - 1;
            size_t  num   = *count;

            /* Destruct each object explicitly. */
            for (size_t idx = 0; idx < num; ++idx)
            {
                ptr[idx].~T();
            }

            /* Free allocated memory. */
            allocator.deallocate(static_cast<void*>(count));
        }
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* TYPEDALLOCATOR_HPP */

/** @} */
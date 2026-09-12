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
 * @file   platform.h
 * @brief  Stub for the mbedTLS platform abstraction
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * The application redirects the mbedTLS memory allocation to the PSRAM. There
 * is no PSRAM on the host, therefore the standard heap is used.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef MBEDTLS_PLATFORM_H
#define MBEDTLS_PLATFORM_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stddef.h>
#include <stdlib.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Memory allocation, which mbedTLS uses. */
#define mbedtls_calloc calloc

/** Memory deallocation, which mbedTLS uses. */
#define mbedtls_free free

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Set the memory allocation functions, which mbedTLS shall use.
 * On the host the standard heap is always used.
 *
 * @param[in] callocFunc    Memory allocation function.
 * @param[in] freeFunc      Memory deallocation function.
 *
 * @return Always 0, which means successful.
 */
static inline int mbedtls_platform_set_calloc_free(void* (*callocFunc)(size_t, size_t), void (*freeFunc)(void*))
{
    (void)callocFunc;
    (void)freeFunc;

    return 0;
}

#endif /* MBEDTLS_PLATFORM_H */

/** @} */

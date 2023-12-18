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
 * @brief  Memory monitor
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup common
 *
 * @{
 */

#ifndef MEM_MON_H
#define MEM_MON_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <SysMsgPlugin.h>
#include <WString.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Memory monitor
 */
class MemMon
{
public:

    /**
     * Get memory monitor instance.
     *
     * @return Memory monitor instance
     */
    static MemMon& getInstance()
    {
        static MemMon instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
    }

    /**
     * Process memory monitor.
     */
    void process();

    /** Processing cycle in ms. */
    static const uint32_t   PROCESSING_CYCLE            = 60U * 1000U;

    /**
     * Minimum size of current heap memory in bytes, the monitor starts to warn.
     * See https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/mbedtls.html#performance-and-memory-tweaks
     */
    static const size_t     MIN_HEAP_MEMORY             = (60U * 1024U);

    /**
     * Lowest size of heap memory in bytes, the monitor starts to warn.
     * See https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/mbedtls.html#performance-and-memory-tweaks
     */
    static const size_t     LOWEST_HEAP_MEMORY          = (50U * 1024U);

    /**
     * Minimum size of largest block of heap that can be allocated at once in bytes, the monitor starts to warn.
     */
    static const size_t     LARGEST_HEAP_BLOCK_MEMORY   = CONFIG_MBEDTLS_SSL_MAX_CONTENT_LEN;

private:

    SimpleTimer m_timer;    /**< Timer used for cyclic processing. */

    /**
     * Constructs the memory monitor.
     */
    MemMon()
    {
    }

    /**
     * Destroys the memory monitor.
     */
    ~MemMon()
    {
        /* Will never be called. */
    }

    MemMon(const MemMon& taskMon);
    MemMon& operator=(const MemMon& taskMon);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* MEM_MON_H */

/** @} */
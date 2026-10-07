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
 * @file   esp_core_dump.h
 * @brief  Stub for the ESP-IDF coredump
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * A coredump is written by the ESP32 in case of a crash. There is no coredump
 * partition on the host, therefore the coredump decoder always reports that
 * there is none. See also esp_partition.h.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef ESP_CORE_DUMP_H
#define ESP_CORE_DUMP_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>

#include "esp_err.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Max. number of backtrace entries. */
#define COREDUMP_BACKTRACE_DEPTH_MAX (16)

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Backtrace information of the crashed task. */
typedef struct
{
    uint32_t bt[COREDUMP_BACKTRACE_DEPTH_MAX]; /**< Backtrace program counters */
    uint32_t depth;                            /**< Number of backtrace entries */
    bool     corrupted;                        /**< Is the backtrace corrupted? */

} esp_core_dump_bt_info_t;

/** Exception information of the crashed task. */
typedef struct
{
    uint32_t exc_cause;     /**< Cause of the exception */
    uint32_t exc_vaddr;     /**< Virtual address of the exception */
    uint32_t exc_a[16U];    /**< Registers at the exception */
    uint32_t epcx[8U];      /**< Program counters at the exception */
    uint8_t  epcx_reg_bits; /**< Bit mask of the valid epcx entries */

} esp_core_dump_ex_info_t;

/** Summary of a coredump. */
typedef struct
{
    uint32_t                core_dump_version;   /**< Coredump version */
    uint8_t                 app_elf_sha256[64U]; /**< SHA256 of the application */
    uint32_t                exc_tcb;             /**< Task control block of the crashed task */
    char                    exc_task[16U];       /**< Name of the crashed task */
    uint32_t                exc_pc;              /**< Program counter of the exception */
    esp_core_dump_bt_info_t exc_bt_info;         /**< Backtrace information */
    esp_core_dump_ex_info_t ex_info;             /**< Exception information */

} esp_core_dump_summary_t;

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Check whether a valid coredump is available.
 *
 * @return Always ESP_FAIL, there is no coredump on the host.
 */
static inline esp_err_t esp_core_dump_image_check(void)
{
    return ESP_FAIL;
}

/**
 * Get the address and size of the coredump.
 *
 * @param[out] addr Address of the coredump.
 * @param[out] size Size of the coredump in byte.
 *
 * @return Always ESP_FAIL, there is no coredump on the host.
 */
static inline esp_err_t esp_core_dump_image_get(size_t* addr, size_t* size)
{
    (void)addr;
    (void)size;

    return ESP_FAIL;
}

/**
 * Get the summary of the coredump.
 *
 * @param[out] summary  The summary.
 *
 * @return Always ESP_FAIL, there is no coredump on the host.
 */
static inline esp_err_t esp_core_dump_get_summary(esp_core_dump_summary_t* summary)
{
    (void)summary;

    return ESP_FAIL;
}

#endif /* ESP_CORE_DUMP_H */

/** @} */

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
 * @file   esp_task_wdt.h
 * @brief  Stub for the ESP-IDF task watchdog
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * There is no watchdog on the host. A deadlock of the main loop is not detected
 * and will not cause a restart.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef ESP_TASK_WDT_H
#define ESP_TASK_WDT_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>

#include "esp_err.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Initialize the task watchdog.
 *
 * @param[in] timeout   Timeout in s.
 * @param[in] panic     Restart on timeout or not.
 *
 * @return Always ESP_OK, there is no watchdog on the host.
 */
static inline esp_err_t esp_task_wdt_init(uint32_t timeout, bool panic)
{
    (void)timeout;
    (void)panic;

    return ESP_OK;
}

/**
 * Add the given task to the task watchdog.
 *
 * @param[in] taskHandle    Task handle, nullptr for the current task.
 *
 * @return Always ESP_OK, there is no watchdog on the host.
 */
static inline esp_err_t esp_task_wdt_add(void* taskHandle)
{
    (void)taskHandle;

    return ESP_OK;
}

/**
 * Reset the task watchdog of the current task.
 *
 * @return Always ESP_OK, there is no watchdog on the host.
 */
static inline esp_err_t esp_task_wdt_reset(void)
{
    return ESP_OK;
}

/**
 * Remove the given task from the task watchdog.
 *
 * @param[in] taskHandle    Task handle, nullptr for the current task.
 *
 * @return Always ESP_OK, there is no watchdog on the host.
 */
static inline esp_err_t esp_task_wdt_delete(void* taskHandle)
{
    (void)taskHandle;

    return ESP_OK;
}

#endif /* ESP_TASK_WDT_H */

/** @} */

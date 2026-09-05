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
 * @file   OsTypes.hpp
 * @brief  freeRTOS compatible types for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * The operating system wrappers of the Os library expose some freeRTOS types
 * and constants in their interfaces. To be able to use the very same interfaces
 * in the native environment, they are provided here as well.
 *
 * One tick is one millisecond, which is the default of the ESP32 freeRTOS port.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef OS_TYPES_HPP
#define OS_TYPES_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Tick type, compatible to the freeRTOS TickType_t. */
typedef uint32_t TickType_t;

/** Signed base type, compatible to the freeRTOS BaseType_t. */
typedef int32_t BaseType_t;

/** Unsigned base type, compatible to the freeRTOS UBaseType_t. */
typedef uint32_t UBaseType_t;

/** Tick rate in Hz, compatible to the freeRTOS configTICK_RATE_HZ. */
static const uint32_t configTICK_RATE_HZ = 1000U;

/** Block infinite, compatible to the freeRTOS portMAX_DELAY. */
static const TickType_t portMAX_DELAY    = UINT32_MAX;

/** Boolean true, compatible to the freeRTOS pdTRUE. */
static const BaseType_t pdTRUE           = 1;

/** Boolean false, compatible to the freeRTOS pdFALSE. */
static const BaseType_t pdFALSE          = 0;

/** Operation succeeded, compatible to the freeRTOS pdPASS. */
static const BaseType_t pdPASS           = pdTRUE;

/** Operation failed, compatible to the freeRTOS pdFAIL. */
static const BaseType_t pdFAIL           = pdFALSE;

/** Id of the protocol CPU core, compatible to the ESP32 PRO_CPU_NUM. */
static const BaseType_t PRO_CPU_NUM      = 0;

/** Id of the application CPU core, compatible to the ESP32 APP_CPU_NUM. */
static const BaseType_t APP_CPU_NUM      = 1;

/** No CPU core affinity, compatible to the freeRTOS tskNO_AFFINITY. */
static const BaseType_t tskNO_AFFINITY   = INT32_MAX;

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Convert a time in ms to ticks, compatible to the freeRTOS pdMS_TO_TICKS.
 *
 * @param[in] timeMs    Time in ms.
 *
 * @return Time in ticks.
 */
constexpr TickType_t pdMS_TO_TICKS(uint32_t timeMs)
{
    return static_cast<TickType_t>((static_cast<uint64_t>(timeMs) * static_cast<uint64_t>(configTICK_RATE_HZ)) / 1000U);
}

/**
 * Convert a time in ticks to ms, compatible to the freeRTOS pdTICKS_TO_MS.
 *
 * @param[in] ticks Time in ticks.
 *
 * @return Time in ms.
 */
constexpr uint32_t pdTICKS_TO_MS(TickType_t ticks)
{
    return static_cast<uint32_t>((static_cast<uint64_t>(ticks) * 1000U) / static_cast<uint64_t>(configTICK_RATE_HZ));
}

#endif /* OS_TYPES_HPP */

/** @} */

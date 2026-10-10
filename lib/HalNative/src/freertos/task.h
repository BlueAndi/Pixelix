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
 * @file   task.h
 * @brief  Stub for the freeRTOS task API
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * There is no freeRTOS on the host, the Os library provides the abstraction on
 * top of the C++ standard library. Only the few definitions which the
 * application shows in the webinterface are provided here.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef FREERTOS_TASK_H
#define FREERTOS_TASK_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <OsTypes.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Kernel version, shown in the webinterface. There is no freeRTOS on the host. */
#define tskKERNEL_VERSION_NUMBER "N/A"

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Task handle, compatible to the freeRTOS TaskHandle_t. */
typedef void* TaskHandle_t;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* FREERTOS_TASK_H */

/** @} */

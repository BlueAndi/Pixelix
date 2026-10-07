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
 * @file   esp_err.h
 * @brief  Stub for the ESP-IDF error codes
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef ESP_ERR_H
#define ESP_ERR_H

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

/** No error. */
#define ESP_OK (0)

/** Generic failure. */
#define ESP_FAIL (-1)

/** Invalid argument. */
#define ESP_ERR_INVALID_ARG (0x102)

/** Invalid state. */
#define ESP_ERR_INVALID_STATE (0x103)

/** Out of memory. */
#define ESP_ERR_NO_MEM (0x101)

/** Not found. */
#define ESP_ERR_NOT_FOUND (0x105)

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Error code, compatible to the ESP-IDF esp_err_t. */
typedef int32_t esp_err_t;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* ESP_ERR_H */

/** @} */

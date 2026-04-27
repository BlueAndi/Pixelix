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
 * @file   CoredumpDecoder.h
 * @brief  Coredump partition decoder
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup WEB
 *
 * @{
 */

#ifndef COREDUMP_DECODER_H
#define COREDUMP_DECODER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <ArduinoJson.h>

/** Coredump decoder utilities */
namespace CoredumpDecoder
{

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Coredump status enumeration */
typedef enum
{
    COREDUMP_STATUS_OK = 0,         /**< Operation successful */
    COREDUMP_STATUS_NO_PARTITION,   /**< Coredump partition not found */
    COREDUMP_STATUS_READ_ERROR,     /**< Error reading partition */
    COREDUMP_STATUS_NO_COREDUMP,    /**< No coredump data present */
    COREDUMP_STATUS_INVALID_FORMAT, /**< Invalid coredump format */
    COREDUMP_STATUS_ERASE_ERROR     /**< Error erasing partition */

} CoredumpStatus;

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Check if coredump partition contains valid data.
 *
 * @param[out] hasCoredump  True if valid coredump exists, false otherwise
 *
 * @return Status of the operation
 */
CoredumpStatus hasCoredump(bool& hasCoredump);

/**
 * Get coredump information and populate JSON document.
 *
 * @param[out] jsonData  JSON object to populate with coredump info
 *
 * @return Status of the operation
 */
CoredumpStatus getCoredumpInfo(JsonObject& jsonData);

/**
 * Get human-readable coredump summary with crash details.
 * Extracts exception cause, crashed task name, registers (A0-A15), and backtrace.
 *
 * @param[out] jsonData  JSON object to populate with decoded crash info
 *
 * @return Status of the operation
 */
CoredumpStatus getCoredumpSummary(JsonObject& jsonData);

/**
 * Clear/erase the coredump partition.
 *
 * @return Status of the operation
 */
CoredumpStatus clearCoredump();

} /* namespace CoredumpDecoder */

#endif /* COREDUMP_DECODER_H */

/** @} */

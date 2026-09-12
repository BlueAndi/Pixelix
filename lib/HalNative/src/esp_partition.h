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
 * @file   esp_partition.h
 * @brief  Stub for the ESP-IDF partition access
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * The coredump decoder reads the coredump from a flash partition. There is no
 * flash on the host, therefore no partition is ever found and the decoder
 * reports that there is no coredump partition.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef ESP_PARTITION_H
#define ESP_PARTITION_H

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

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Partition type, compatible to the ESP-IDF esp_partition_type_t. */
typedef enum
{
    ESP_PARTITION_TYPE_APP  = 0x00, /**< Application partition */
    ESP_PARTITION_TYPE_DATA = 0x01, /**< Data partition */
    ESP_PARTITION_TYPE_ANY  = 0xFF  /**< Any partition */

} esp_partition_type_t;

/** Partition subtype, compatible to the ESP-IDF esp_partition_subtype_t. */
typedef enum
{
    ESP_PARTITION_SUBTYPE_APP_FACTORY   = 0x00, /**< Factory application partition */
    ESP_PARTITION_SUBTYPE_APP_OTA_0     = 0x10, /**< First OTA application partition */
    ESP_PARTITION_SUBTYPE_APP_OTA_1     = 0x11, /**< Second OTA application partition */
    ESP_PARTITION_SUBTYPE_DATA_COREDUMP = 0x03, /**< Coredump partition */
    ESP_PARTITION_SUBTYPE_ANY           = 0xFF  /**< Any subtype */

} esp_partition_subtype_t;

/** Partition information, compatible to the ESP-IDF esp_partition_t. */
typedef struct
{
    esp_partition_type_t    type;       /**< Partition type */
    esp_partition_subtype_t subtype;    /**< Partition subtype */
    uint32_t                address;    /**< Address in the flash */
    uint32_t                size;       /**< Size in byte */
    char                    label[17U]; /**< Partition label */

} esp_partition_t;

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Find the first partition with the given type, subtype and label.
 * There is no flash on the host, therefore nothing is found.
 *
 * @param[in] type      Partition type
 * @param[in] subtype   Partition subtype
 * @param[in] label     Partition label, may be nullptr.
 *
 * @return Always nullptr.
 */
static inline const esp_partition_t* esp_partition_find_first(esp_partition_type_t type, esp_partition_subtype_t subtype, const char* label)
{
    (void)type;
    (void)subtype;
    (void)label;

    return nullptr;
}

/**
 * Read from the given partition.
 *
 * @param[in]  partition    The partition.
 * @param[in]  offset       Offset in the partition.
 * @param[out] dst          Destination buffer.
 * @param[in]  size         Number of bytes to read.
 *
 * @return Always ESP_FAIL, there is no flash on the host.
 */
static inline esp_err_t esp_partition_read(const esp_partition_t* partition, size_t offset, void* dst, size_t size)
{
    (void)partition;
    (void)offset;
    (void)dst;
    (void)size;

    return ESP_FAIL;
}

/**
 * Erase a range of the given partition.
 *
 * @param[in] partition The partition.
 * @param[in] offset    Offset in the partition.
 * @param[in] size      Number of bytes to erase.
 *
 * @return Always ESP_FAIL, there is no flash on the host.
 */
static inline esp_err_t esp_partition_erase_range(const esp_partition_t* partition, size_t offset, size_t size)
{
    (void)partition;
    (void)offset;
    (void)size;

    return ESP_FAIL;
}

#endif /* ESP_PARTITION_H */

/** @} */

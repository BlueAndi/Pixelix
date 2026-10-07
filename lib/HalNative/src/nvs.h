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
 * @file   nvs.h
 * @brief  Stub for the ESP-IDF non-volatile storage
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * The settings service iterates over the non-volatile storage to find obsolete
 * keys. There is no non-volatile storage on the host, therefore the iterator is
 * always empty and nothing is cleaned up.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef NVS_H
#define NVS_H

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

/** Name of the default non-volatile storage partition. */
#define NVS_DEFAULT_PART_NAME "nvs"

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Type of a non-volatile storage entry. */
typedef enum
{
    NVS_TYPE_U8   = 0x01, /**< Unsigned 8-bit integer */
    NVS_TYPE_I8   = 0x11, /**< Signed 8-bit integer */
    NVS_TYPE_U16  = 0x02, /**< Unsigned 16-bit integer */
    NVS_TYPE_I16  = 0x12, /**< Signed 16-bit integer */
    NVS_TYPE_U32  = 0x04, /**< Unsigned 32-bit integer */
    NVS_TYPE_I32  = 0x14, /**< Signed 32-bit integer */
    NVS_TYPE_U64  = 0x08, /**< Unsigned 64-bit integer */
    NVS_TYPE_I64  = 0x18, /**< Signed 64-bit integer */
    NVS_TYPE_STR  = 0x21, /**< String */
    NVS_TYPE_BLOB = 0x42, /**< Binary large object */
    NVS_TYPE_ANY  = 0xFF  /**< Any type */

} nvs_type_t;

/** Information about a single non-volatile storage entry. */
typedef struct
{
    char       namespace_name[16]; /**< Namespace of the entry */
    char       key[16];            /**< Key of the entry */
    nvs_type_t type;               /**< Type of the entry */

} nvs_entry_info_t;

/** Opaque iterator over the non-volatile storage entries. */
typedef struct nvs_opaque_iterator_t* nvs_iterator_t;

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Find the first entry of the given namespace and type.
 * There is no non-volatile storage on the host, therefore nothing is found.
 *
 * @param[in] partName      Name of the partition.
 * @param[in] namespaceName Name of the namespace.
 * @param[in] type          Type of the entry.
 *
 * @return Always nullptr.
 */
static inline nvs_iterator_t nvs_entry_find(const char* partName, const char* namespaceName, nvs_type_t type)
{
    (void)partName;
    (void)namespaceName;
    (void)type;

    return nullptr;
}

/**
 * Get the next entry.
 *
 * @param[in] it    Current iterator.
 *
 * @return Always nullptr.
 */
static inline nvs_iterator_t nvs_entry_next(nvs_iterator_t it)
{
    (void)it;

    return nullptr;
}

/**
 * Get the information about the entry the iterator points to.
 *
 * @param[in]  it   Iterator.
 * @param[out] info Information about the entry.
 */
static inline void nvs_entry_info(nvs_iterator_t it, nvs_entry_info_t* info)
{
    (void)it;

    if (nullptr != info)
    {
        info->namespace_name[0] = '\0';
        info->key[0]            = '\0';
        info->type              = NVS_TYPE_ANY;
    }
}

#endif /* NVS_H */

/** @} */

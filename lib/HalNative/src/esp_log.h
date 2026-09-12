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
 * @file   esp_log.h
 * @brief  Stub for the ESP-IDF logging
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * Pixelix uses its own logging, see the Logging library. The ESP-IDF logging is
 * only used by the framework and 3rd party libraries, therefore it is silent on
 * the host.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef ESP_LOG_H
#define ESP_LOG_H

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

/** Log a error, which is discarded on the host. */
#define ESP_LOGE(tag, format, ...) \
    do                             \
    {                              \
        (void)(tag);               \
    }                              \
    while (0)

/** Log a warning, which is discarded on the host. */
#define ESP_LOGW(tag, format, ...) \
    do                             \
    {                              \
        (void)(tag);               \
    }                              \
    while (0)

/** Log a info, which is discarded on the host. */
#define ESP_LOGI(tag, format, ...) \
    do                             \
    {                              \
        (void)(tag);               \
    }                              \
    while (0)

/** Log a debug info, which is discarded on the host. */
#define ESP_LOGD(tag, format, ...) \
    do                             \
    {                              \
        (void)(tag);               \
    }                              \
    while (0)

/** Log a verbose info, which is discarded on the host. */
#define ESP_LOGV(tag, format, ...) \
    do                             \
    {                              \
        (void)(tag);               \
    }                              \
    while (0)

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Log severity, compatible to the ESP-IDF esp_log_level_t. */
typedef enum
{
    ESP_LOG_NONE = 0, /**< No log output */
    ESP_LOG_ERROR,    /**< Critical errors */
    ESP_LOG_WARN,     /**< Errors which are recovered from */
    ESP_LOG_INFO,     /**< Information messages */
    ESP_LOG_DEBUG,    /**< Debug information */
    ESP_LOG_VERBOSE   /**< Very detailed debug information */

} esp_log_level_t;

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Set the log severity of the given tag.
 * The ESP-IDF logging is silent on the host, therefore nothing happens.
 *
 * @param[in] tag   Tag of the log output.
 * @param[in] level Log severity.
 */
static inline void esp_log_level_set(const char* tag, esp_log_level_t level)
{
    (void)tag;
    (void)level;
}

#endif /* ESP_LOG_H */

/** @} */

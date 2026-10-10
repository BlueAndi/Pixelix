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
 * @file   esp_sntp.h
 * @brief  Stub for the ESP-IDF SNTP client
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * The host provides its own time, therefore no time synchronization by NTP
 * takes place.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef ESP_SNTP_H
#define ESP_SNTP_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <sys/time.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Callback which is notified after the time is synchronized.
 *
 * @param[in] tv    The synchronized time.
 */
typedef void (*sntp_sync_time_cb_t)(struct timeval* tv);

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Set the callback which is notified after the time is synchronized.
 * There is no time synchronization on the host, therefore it is never called.
 *
 * @param[in] callback  The callback.
 */
static inline void sntp_set_time_sync_notification_cb(sntp_sync_time_cb_t callback)
{
    (void)callback;
}

/**
 * Set the interval of the time synchronization.
 *
 * @param[in] interval  Interval in ms.
 */
static inline void sntp_set_sync_interval(uint32_t interval)
{
    (void)interval;
}

/**
 * Configure the time zone and the NTP server.
 * On the host the time zone of the host is used.
 *
 * @param[in] tz        Time zone string in POSIX format.
 * @param[in] server    NTP server address.
 */
static inline void configTzTime(const char* tz, const char* server)
{
    (void)tz;
    (void)server;
}

#endif /* ESP_SNTP_H */

/** @} */

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
 * @file   ClockDrv.cpp
 * @brief  Clock driver for test purposes only
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ClockDrv.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool ClockDrv::getTime(struct tm& timeInfo)
{
    time_t     now    = time(nullptr);
    struct tm* result = localtime(&now);
    bool       status = false;

    if (nullptr != result)
    {
        timeInfo = *result;
        status   = true;
    }

    return status;
}

bool ClockDrv::getTimeUtc(struct tm& timeInfo)
{
    time_t     now    = time(nullptr);
    struct tm* result = gmtime(&now);
    bool       status = false;

    if (nullptr != result)
    {
        timeInfo = *result;
        status   = true;
    }

    return status;
}

bool ClockDrv::getTzTime(const char* tz, struct tm& timeInfo)
{
    /* The timezone is not supported on the host. */
    (void)tz;

    return getTime(timeInfo);
}

long ClockDrv::getCurrentTimeZoneOffset() const
{
    time_t     now         = time(nullptr);
    long       offsetInSec = 0;
    struct tm* result      = localtime(&now);

    if (nullptr != result)
    {
        /* Copied, because localtime() and gmtime() may share the same buffer.
         * The reentrant variants are not used, because they are not available
         * on every host.
         */
        struct tm localTimeInfo = *result;

        result                  = gmtime(&now);

        if (nullptr != result)
        {
            struct tm gmTimeInfo = *result;

            offsetInSec          = static_cast<long>(mktime(&localTimeInfo)) - static_cast<long>(mktime(&gmTimeInfo));
        }
    }

    return offsetInSec;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

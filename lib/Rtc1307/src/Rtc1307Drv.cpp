/* MIT License
 *
 * Copyright (c) 2019 - 2023 Andreas Merkle <web@blue-andi.de>
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
 * @brief  RTC 1307 driver
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Rtc1307Drv.h"

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

bool Rtc1307Drv::begin()
{
    bool isSuccess = false;

    if (false == m_isInitialized)
    {
        isSuccess = m_rtc.begin();

        if (true == isSuccess)
        {
            /* If RTC doesn't run, the RTC will be set. */
            if (false == m_rtc.isrunning())
            {
                /* Set explicit to date & time this version of Pixelix was compiled. */
                m_rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
            }
        }

        m_isInitialized = isSuccess;
    }

    return isSuccess;
}

bool Rtc1307Drv::getTime(struct tm& timeInfo)
{
    bool isSuccess = false;

    if (true == m_isInitialized)
    {
        DateTime dateTime = m_rtc.now();

        memset(&timeInfo, 0, sizeof(struct tm));

        timeInfo.tm_sec     = dateTime.second();
        timeInfo.tm_min     = dateTime.minute();
        timeInfo.tm_hour    = dateTime.hour();
        timeInfo.tm_mday    = dateTime.day();
        timeInfo.tm_mon     = dateTime.month() - 1;
        timeInfo.tm_year    = dateTime.year() - 1900;
    }

    return isSuccess;
}

void Rtc1307Drv::setTime(const struct tm& timeInfo)
{
    if (true == m_isInitialized)
    {
        DateTime    time(
            timeInfo.tm_year + 1900,
            timeInfo.tm_mon + 1,
            timeInfo.tm_mday,
            timeInfo.tm_hour,
            timeInfo.tm_min,
            timeInfo.tm_sec
        );
        m_rtc.adjust(time);
    }
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

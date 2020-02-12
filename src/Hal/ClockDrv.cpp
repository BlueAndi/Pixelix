/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Clock driver
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ClockDrv.h"
#include "Settings.h"
#include "time.h"

#include <sys/time.h>
#include <Logging.h>

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

/* Instantiate the button driver singleton. */
ClockDrv ClockDrv::m_instance;

/** Daylight saving time offset in s */
static const int16_t NTP_DAYLIGHT_OFFSET_SEC    = 3600;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void ClockDrv::init()
{
    struct tm   timeInfo;
    bool        isDaylightSaving;

    if (false == m_isClockDrvInitialized)
    {
        m_ntpSyncTimer.start(NTP_SYNC_PERIOD);

        /* Get the GMT offset. */
        if (false == Settings::getInstance().open(true))
        {
            LOG_WARNING("Use default values for NTP request.");
            m_gmtOffset = Settings::getInstance().getGmtOffset().getDefault();
            isDaylightSaving = Settings::getInstance().getDaylightSavingAdjustment().getDefault();
            m_ntpServerAddress = Settings::getInstance().getNTPServerAddress().getDefault();
        }
        else
        {
            m_gmtOffset = Settings::getInstance().getGmtOffset().getValue();
            isDaylightSaving = Settings::getInstance().getDaylightSavingAdjustment().getValue();
            m_ntpServerAddress = Settings::getInstance().getNTPServerAddress().getValue();
            Settings::getInstance().close();
        }

        m_daylightSavingValue = (false != isDaylightSaving) ? NTP_DAYLIGHT_OFFSET_SEC : 0;

        configTime(m_gmtOffset, m_daylightSavingValue, m_ntpServerAddress.c_str());

        if (!getLocalTime(&timeInfo))
        {
            m_isSynchronized = false;
            LOG_ERROR("Failed to synchronize time");
        }
        else
        {
            m_isSynchronized = true;
            LOG_INFO("Time successfully synchronized:  %d:%d", timeInfo.tm_hour, timeInfo.tm_min);
        }
    }
}

bool ClockDrv::getTime(tm *currentTime)
{
    bool ret = false;

    if (m_isClockDrvInitialized)
    {
        ret = getLocalTime(currentTime);
    }

    return ret;
}

void ClockDrv::process()
{
    if ((true == m_ntpSyncTimer.isTimerRunning()) &&
        (true == m_ntpSyncTimer.isTimeout()))
    {
        m_isSynchronized = false;
        m_ntpSyncTimer.restart();
    }

    if ( false == m_isSynchronized)
    {
        struct tm timeInfo;

        configTime(m_gmtOffset, m_daylightSavingValue, m_ntpServerAddress.c_str());
        if (!getLocalTime(&timeInfo))
        {
            m_isSynchronized = false;
            LOG_ERROR("Failed to synchronize time");
        }
        else
        {
            m_isSynchronized = true;
            LOG_INFO("Time successfully synchronized:  %d:%d", timeInfo.tm_hour, timeInfo.tm_min);
        }
    }
}

bool ClockDrv::isSynchronized()
{
    return m_isSynchronized;
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


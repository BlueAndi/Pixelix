/* MIT License
 *
 * Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Reset monitor
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ResetMon.h"

#include <Logging.h>

#include "CRC32.h"

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

/**
 * Counts the number of resets.
 * It doesn't differ between unexpected resets and requested resets.
 */
static __NOINIT_ATTR uint32_t   gResetCounter;

/**
 * The guard is used to determine whether the reset counter value is valid
 * or not. In case of a power-up it will be random set.
 */
static __NOINIT_ATTR uint32_t   gNonInitGuardCrc;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void ResetMon::begin()
{
    handleResetCounter();

    m_resetReasonAppCpu = rtc_get_reset_reason(APP_CPU_NUM);
    m_resetReasonProCpu = rtc_get_reset_reason(PRO_CPU_NUM);
}

void ResetMon::process()
{
    bool isProcessingTime = false;

    if (false == m_timer.isTimerRunning())
    {
        m_timer.start(PROCESSING_CYCLE);
        isProcessingTime = true;
    }
    else if (true == m_timer.isTimeout())
    {
        isProcessingTime = true;
        m_timer.restart();
    }

    if (true == isProcessingTime)
    {
        String resetReasonAppCpu;
        String resetReasonProCpu;

        getResetReasonToStr(resetReasonAppCpu, m_resetReasonAppCpu);
        getResetReasonToStr(resetReasonProCpu, m_resetReasonProCpu);

        LOG_INFO("Reset reason APP CPU: %s", resetReasonAppCpu.c_str());
        LOG_INFO("Reset reason PRO CPU: %s", resetReasonProCpu.c_str());
        LOG_INFO("Counted resets: %u", gResetCounter);
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void ResetMon::handleResetCounter()
{
    CRC32 crc;

    crc.setPolynome(CRC32_CASTAGNOLI);
    crc.add(reinterpret_cast<uint8_t*>(&gResetCounter), sizeof(gResetCounter));

    if (crc.getCRC() != gNonInitGuardCrc)
    {
        gResetCounter = 0U;
    }
    else
    {
        ++gResetCounter;
    }

    crc.restart();
    crc.add(reinterpret_cast<uint8_t*>(&gResetCounter), sizeof(gResetCounter));

    gNonInitGuardCrc = crc.getCRC();
}

void ResetMon::getResetReasonToStr(String& str, RESET_REASON resetReason)
{
    switch(resetReason)
    {
    case NO_MEAN:
        str = "NO_MEAN";
        break;
    
    case POWERON_RESET:
        str = "Vbat power on reset";
        break;
    
    case SW_RESET:
        str = "Software reset digital core";
        break;

    case OWDT_RESET:
        str = "Legacy watch dog reset digital core";
        break;

    case DEEPSLEEP_RESET:
        str = "Deep Sleep reset digital core";
        break;

    case SDIO_RESET:
        str = "Reset by SLC module, reset digital core";
        break;

    case TG0WDT_SYS_RESET:
        str = "Timer Group0 Watch dog reset digital core";
        break;

    case TG1WDT_SYS_RESET:
        str = "Timer Group1 Watch dog reset digital core";
        break;

    case RTCWDT_SYS_RESET:
        str = "RTC Watch dog Reset digital core";
        break;

    case INTRUSION_RESET:
        str = "Instrusion tested to reset CPU";
        break;

    case TGWDT_CPU_RESET:
        str = "Time Group reset CPU";
        break;

    case SW_CPU_RESET:
        str = "Software reset CPU";
        break;

    case RTCWDT_CPU_RESET:
        str = "RTC Watch dog Reset CPU";
        break;

    case EXT_CPU_RESET:
        str = "For APP CPU, reseted by PRO CPU";
        break;

    case RTCWDT_BROWN_OUT_RESET:
        str = "Reset when the vdd voltage is not stable";
        break;

    case RTCWDT_RTC_RESET:
        str = "RTC Watch dog reset digital core and rtc module";
        break;

    default:
        str = "Unknown reset reason!";
        break;
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

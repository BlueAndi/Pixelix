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
 * @brief  Reset monitor
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ResetMon.h"

#include <Logging.h>

#include "rom/crc.h"

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
    uint32_t calcCrc32 = crc32_le(0U, reinterpret_cast<uint8_t*>(&gResetCounter), sizeof(gResetCounter));

    /* Is the current reset counter invalid? */
    if (calcCrc32 != gNonInitGuardCrc)
    {
        /* Its invalid, we assume it was a power-up cycle and don't consider
         * the case being corrupted by anyone else.
         */
        gResetCounter = 0U;
    }
    else
    {
        /* The reset counter is valid and the current one can be counted. */
        ++gResetCounter;
    }

    gNonInitGuardCrc = crc32_le(0U, reinterpret_cast<uint8_t*>(&gResetCounter), sizeof(gResetCounter));
}

void ResetMon::getResetReasonToStr(String& str, RESET_REASON resetReason)
{
    switch(resetReason)
    {
#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32S2)
    case NO_MEAN:
        str = "NO_MEAN";
        break;

    case POWERON_RESET:
        str = "Vbat power on reset";
        break;

    case RTC_SW_SYS_RESET:
        str = "Software reset digital core";
        break;

    case DEEPSLEEP_RESET:
        str = "Deep Sleep reset digital core";
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

    case TG0WDT_CPU_RESET:
        str = "Time Group0 reset CPU";
        break;

    case RTC_SW_CPU_RESET:
        str = "Software reset CPU";
        break;

    case RTCWDT_CPU_RESET:
        str = "RTC Watch dog Reset CPU";
        break;

    case RTCWDT_BROWN_OUT_RESET:
        str = "Reset when the vdd voltage is not stable";
        break;

    case RTCWDT_RTC_RESET:
        str = "RTC Watch dog reset digital core and rtc module";
        break;

    case TG1WDT_CPU_RESET:
        str = "Time Group1 reset CPU";
        break;

    case SUPER_WDT_RESET:
        str = "super watchdog reset digital core and rtc module";
        break;

    case GLITCH_RTC_RESET:
        str = "glitch reset digital core and rtc module";
        break;

    case EFUSE_RESET:
        str = "efuse reset digital core";
        break;

#if defined(CONFIG_IDF_TARGET_ESP32S3)

    case USB_UART_CHIP_RESET:
        str = "usb uart reset digital core";
        break;

    case USB_JTAG_CHIP_RESET:
        str = "usb jtag reset digital core";
        break;

    case POWER_GLITCH_RESET:
        str = "power glitch reset digital core and rtc module";
        break;

#endif /* defined(CONFIG_IDF_TARGET_ESP32S3) */

#else

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
#endif

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

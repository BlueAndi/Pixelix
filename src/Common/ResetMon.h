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
 *
 * @addtogroup common
 *
 * @{
 */

#ifndef __RESET_MON_H__
#define __RESET_MON_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <SysMsgPlugin.h>
#include <WString.h>

#if defined(CONFIG_IDF_TARGET_ESP32S3)
#include "esp32s3/rom/rtc.h"
#else
#include "esp32/rom/rtc.h"
#endif
/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Reset monitor
 */
class ResetMon
{
public:

    /**
     * Get memory monitor instance.
     *
     * @return Reset monitor instance
     */
    static ResetMon& getInstance()
    {
        static ResetMon instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
    }

    /**
     * This method shall be called in a early stage and
     * handles the reset counter and reads the reset reason.
     */
    void begin();

    /**
     * Process reset monitor.
     */
    void process();

    /** Processing cycle in ms. */
    static const uint32_t PROCESSING_CYCLE  = 5U * 60U * 1000U;

private:

    SimpleTimer     m_timer;                /**< Timer used for cyclic processing. */
    RESET_REASON    m_resetReasonAppCpu;    /**< The reset reason of the APP cpu. */
    RESET_REASON    m_resetReasonProCpu;    /**< The reset reason of the PRO cpu.

    /**
     * Constructs the reset monitor.
     */
    ResetMon() :
        m_timer(),
        m_resetReasonAppCpu(NO_MEAN),
        m_resetReasonProCpu(NO_MEAN)
    {
    }

    /**
     * Destroys the reset monitor.
     */
    ~ResetMon()
    {
        /* Will never be called. */
    }

    ResetMon(const ResetMon& taskMon);
    ResetMon& operator=(const ResetMon& taskMon);

    /**
     * This function counts the number of resets.
     */
    void handleResetCounter();

    /**
     * Get the reset reason as string.
     *
     * @param[out]  str         Reset reason as string
     * @param[in]   resetReason Reset reason id
     */
    void getResetReasonToStr(String& str, RESET_REASON resetReason);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __RESET_MON_H__ */

/** @} */
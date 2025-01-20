/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Websocket command to get/set display brightness
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WsCmdBrightness.h"
#include "DisplayMgr.h"

#include <Util.h>
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

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void WsCmdBrightness::execute(AsyncWebSocket* server, uint32_t clientId)
{
    if (nullptr == server)
    {
        return;
    }

    /* Any error happended? */
    if (true == m_isError)
    {
        sendNegativeResponse(server, clientId, "\"Parameter invalid.\"");
    }
    else if ((0U != m_parCnt) &&
             (4U != m_parCnt))
    {
        sendNegativeResponse(server, clientId, "\"Parameter missing.\"");
    }
    else
    {
        DisplayMgr& displayMgr = DisplayMgr::getInstance();
        uint8_t     brightness;
        uint8_t     minBrightnessSoftLimit;
        uint8_t     maxBrightnessSoftLimit;
        bool        isAutomaticBrightnessCtrlEnabled;
        String      msg;

        if (4U == m_parCnt)
        {
            displayMgr.setBrightnessSoftLimits(m_minBrightness, m_maxBrightness);
            displayMgr.setAutoBrightnessAdjustment(m_isAutomaticBrightnessCtrlEnabled);

            /* Set brightness after possible adjusted soft limits. */
            displayMgr.setBrightness(m_brightness);
        }

        preparePositiveResponse(msg);

        brightness                       = displayMgr.getBrightness();
        isAutomaticBrightnessCtrlEnabled = displayMgr.getAutoBrightnessAdjustment();
        displayMgr.getBrightnessSoftLimits(minBrightnessSoftLimit, maxBrightnessSoftLimit);

        msg += brightness;
        msg += DELIMITER;
        msg += minBrightnessSoftLimit;
        msg += DELIMITER;
        msg += maxBrightnessSoftLimit;
        msg += DELIMITER;
        msg += (true == isAutomaticBrightnessCtrlEnabled) ? 1 : 0;

        sendResponse(server, clientId, msg);
    }

    m_isError = false;
    m_parCnt  = 0U;
}

void WsCmdBrightness::setPar(const char* par)
{
    switch (m_parCnt)
    {
    case 0U:
        if (false == Util::strToUInt8(par, m_brightness))
        {
            LOG_ERROR("Conversion failed: %s", par);
            m_isError = true;
        }
        break;

    case 1U:
        if (false == Util::strToUInt8(par, m_minBrightness))
        {
            LOG_ERROR("Conversion failed: %s", par);
            m_isError = true;
        }
        break;

    case 2U:
        if (false == Util::strToUInt8(par, m_maxBrightness))
        {
            LOG_ERROR("Conversion failed: %s", par);
            m_isError = true;
        }
        break;

    case 3U:
        if (0 == strcmp(par, "0"))
        {
            m_isAutomaticBrightnessCtrlEnabled = false;
        }
        else if (0 == strcmp(par, "1"))
        {
            m_isAutomaticBrightnessCtrlEnabled = true;
        }
        else
        {
            m_isError = true;
        }
        break;

    default:
        m_isError = true;
        break;
    }

    ++m_parCnt;
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

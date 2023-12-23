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

void WsCmdBrightness::execute(AsyncWebSocket* server, AsyncWebSocketClient* client)
{
    if ((nullptr == server) ||
        (nullptr == client))
    {
        return;
    }

    /* Any error happended? */
    if (true == m_isError)
    {
        sendNegativeResponse(server, client, "\"Parameter invalid.\"");
    }
    else
    {
        String msg;

        if (1U == m_parCnt)
        {
            DisplayMgr::getInstance().setBrightness(m_brightness);
        }
        else if (2U == m_parCnt)
        {
            DisplayMgr::getInstance().setBrightness(m_brightness);
            DisplayMgr::getInstance().setAutoBrightnessAdjustment(m_isEnabled);
        }
        else
        {
            ;
        }

        preparePositiveResponse(msg);

        msg += DisplayMgr::getInstance().getBrightness();
        msg += DELIMITER;
        msg += (true == DisplayMgr::getInstance().getAutoBrightnessAdjustment()) ? 1 : 0;

        sendResponse(server, client, msg);
    }

    m_isError = false;
    m_parCnt = 0U;
}

void WsCmdBrightness::setPar(const char* par)
{
    switch(m_parCnt)
    {
    case 0:
        if (false == Util::strToUInt8(String(par), m_brightness))
        {
            LOG_ERROR("Conversion failed: %s", par);
            m_isError = true;
        }
        break;

    case 1:
        if (0 == strcmp(par, "0"))
        {
            m_isEnabled = false;
        }
        else if (0 == strcmp(par, "1"))
        {
            m_isEnabled = true;
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

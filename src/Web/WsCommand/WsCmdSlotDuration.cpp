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
 * @brief  Websocket command to get/set slot duration
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WsCmdSlotDuration.h"
#include "DisplayMgr.h"

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

void WsCmdSlotDuration::execute(AsyncWebSocket* server, AsyncWebSocketClient* client)
{
    if ((nullptr == server) ||
        (nullptr == client))
    {
        return;
    }

    /* Any error happended? */
    if (true == m_isError)
    {
        server->text(client->id(), "NACK;\"Parameter invalid.\"");
    }
    else
    {
        String      rsp         = "ACK";
        const char  DELIMITER   = ';';

        if (2U == m_parCnt)
        {
            (void)DisplayMgr::getInstance().setSlotDuration(m_slotId, m_slotDuration);
        }

        rsp += DELIMITER;
        rsp += DisplayMgr::getInstance().getSlotDuration(m_slotId);

        server->text(client->id(), rsp);
    }

    m_isError = false;
    m_parCnt = 0U;

    return;
}

void WsCmdSlotDuration::setPar(const char* par)
{
    switch(m_parCnt)
    {
    case 0:
        if (false == Util::strToUInt8(String(par), m_slotId))
        {
            LOG_ERROR("Conversion failed: %s", par);
            m_isError = true;
        }
        break;

    case 1:
        if (false == Util::strToUInt32(String(par), m_slotDuration))
        {
            LOG_ERROR("Conversion failed: %s", par);
            m_isError = true;
        }
        break;

    default:
        m_isError = true;
        break;
    }

    ++m_parCnt;

    return;
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

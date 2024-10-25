/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Websocket command to move a plugin to a different slot.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WsCmdMove.h"
#include "PluginMgr.h"
#include "SlotList.h"

#include <Logging.h>
#include <Util.h>

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

void WsCmdMove::execute(AsyncWebSocket* server, uint32_t clientId)
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
    else
    {
        uint8_t             srcSlotId   = DisplayMgr::getInstance().getSlotIdByPluginUID(m_uid);
        IPluginMaintenance* plugin      = DisplayMgr::getInstance().getPluginInSlot(srcSlotId);

        if (SlotList::SLOT_ID_INVALID == srcSlotId)
        {
            sendNegativeResponse(server, clientId, "\"Plugin UID not found.\"");
        }
        else if (nullptr == plugin)
        {
            sendNegativeResponse(server, clientId, "\"Plugin not found.\"");
        }
        else if (false == DisplayMgr::getInstance().movePluginToSlot(plugin, m_slotId))
        {
            sendNegativeResponse(server, clientId, "\"Move failed.\"");
        }
        else
        {
            /* Save new location of plugin in persistent memory. */
            PluginMgr::getInstance().save();

            sendPositiveResponse(server, clientId);
        }
    }

    m_isError = false;
    m_parCnt = 0U;
}

void WsCmdMove::setPar(const char* par)
{
    switch(m_parCnt)
    {
    case 0U:
        if (false == Util::strToUInt16(par, m_uid))
        {
            m_isError = true;
        }
        break;

    case 1U:
        if (false == Util::strToUInt8(par, m_slotId))
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

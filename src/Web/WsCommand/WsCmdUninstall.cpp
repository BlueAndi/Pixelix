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
 * @brief  Websocket command to uninstall a plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WsCmdUninstall.h"
#include "DisplayMgr.h"
#include "PluginMgr.h"
#include "SlotList.h"

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

void WsCmdUninstall::execute(AsyncWebSocket* server, uint32_t clientId)
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
        String              rsp;
        IPluginMaintenance* plugin  = DisplayMgr::getInstance().getPluginInSlot(m_slotId);

        if (nullptr == plugin)
        {
            sendNegativeResponse(server, clientId, "\"Slot is empty.\"");
        }
        else if (true == DisplayMgr::getInstance().isSlotLocked(m_slotId))
        {
            sendNegativeResponse(server, clientId, "\"Slot is locked.\"");
        }
        else if (false == PluginMgr::getInstance().uninstall(plugin))
        {
            sendNegativeResponse(server, clientId, "\"Failed to uninstall.\"");
        }
        else
        {
            /* Save current installed plugins to persistent memory. */
            PluginMgr::getInstance().save();

            sendPositiveResponse(server, clientId);
        }
    }

    m_isError   = false;
    m_slotId    = SlotList::SLOT_ID_INVALID;
}

void WsCmdUninstall::setPar(const char* par)
{
    if (false == m_isError)
    {
        if (SlotList::SLOT_ID_INVALID == m_slotId)
        {
            if (false == Util::strToUInt8(par, m_slotId))
            {
                LOG_ERROR("Conversion failed: %s", par);
                m_isError = true;
            }
        }
        else
        {
            m_isError = true;
        }
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

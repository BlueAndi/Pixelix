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
 * @brief  Websocket command to uninstall a plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WsCmdUninstall.h"
#include "PluginMgr.h"

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

void WsCmdUninstall::execute(AsyncWebSocket* server, AsyncWebSocketClient* client)
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
        String              rsp;
        IPluginMaintenance* plugin  = DisplayMgr::getInstance().getPluginInSlot(m_slotId);

        if (nullptr == plugin)
        {
            rsp = "NACK;\"Slot is empty.\"";
        }
        else if (true == DisplayMgr::getInstance().isSlotLocked(m_slotId))
        {
            rsp = "NACK;\"Slot is locked.\"";
        }
        else if (false == PluginMgr::getInstance().uninstall(plugin))
        {
            rsp = "NACK;\"Failed to uninstall.\"";
        }
        else
        {
            /* Save current installed plugins to persistent memory. */
            PluginMgr::getInstance().save();

            rsp = "ACK";
        }

        server->text(client->id(), rsp);
    }

    m_isError   = false;
    m_slotId    = DisplayMgr::SLOT_ID_INVALID;

    return;
}

void WsCmdUninstall::setPar(const char* par)
{
    if (false == m_isError)
    {
        if (DisplayMgr::SLOT_ID_INVALID == m_slotId)
        {
            if (false == Util::strToUInt8(String(par), m_slotId))
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

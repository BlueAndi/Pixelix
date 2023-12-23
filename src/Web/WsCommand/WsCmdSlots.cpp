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
 * @brief  Websocket command get slots information
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WsCmdSlots.h"
#include "DisplayMgr.h"
#include "SlotList.h"

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

void WsCmdSlots::execute(AsyncWebSocket* server, AsyncWebSocketClient* client)
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
        String      msg;
        DisplayMgr& displayMgr  = DisplayMgr::getInstance();
        uint8_t     slotId      = SlotList::SLOT_ID_INVALID;
        uint8_t     stickySlot  = displayMgr.getStickySlot();

        preparePositiveResponse(msg);

        msg += displayMgr.getMaxSlots();

        /* Provides for every slot:
         * - Name of plugin.
         * - Plugin UID.
         * - Plugin alias name.
         * - Information about whether the slot is locked or not.
         * - Information about whether the slot is sticky or not.
         * - Slot duration in ms.
         */
        for(slotId = 0U; slotId < displayMgr.getMaxSlots(); ++slotId)
        {
            IPluginMaintenance* plugin      = displayMgr.getPluginInSlot(slotId);
            const char*         name        = (nullptr != plugin) ? plugin->getName() : "";
            uint16_t            uid         = (nullptr != plugin) ? plugin->getUID() : 0U;
            String              alias       = (nullptr != plugin) ? plugin->getAlias() : "";
            bool                isLocked    = displayMgr.isSlotLocked(slotId);
            bool                isSticky    = (stickySlot == slotId) ? true : false;
            uint32_t            duration    = displayMgr.getSlotDuration(slotId);

            msg += DELIMITER;
            msg += "\"";
            msg += name;
            msg += "\"";
            msg += DELIMITER;
            msg += uid;
            msg += DELIMITER;
            msg += "\"";
            msg += alias;
            msg += "\"";
            msg += DELIMITER;
            msg += (false == isLocked) ? "0" : "1";
            msg += DELIMITER;
            msg += (false == isSticky) ? "0" : "1";
            msg += DELIMITER;
            msg += duration;
        }

        sendResponse(server, client, msg);
    }

    m_isError = false;
}

void WsCmdSlots::setPar(const char* par)
{
    UTIL_NOT_USED(par);

    m_isError = true;
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

/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   WsCmdSlots.cpp
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

void WsCmdSlots::execute(AsyncWebSocket* server, uint32_t clientId)
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
        String      msg;
        DisplayMgr& displayMgr = DisplayMgr::getInstance();
        uint8_t     maxSlots   = displayMgr.getMaxSlots();
        uint8_t     slotId;

        preparePositiveResponse(msg);

        msg += displayMgr.getMaxSlots();

        /* Provides for every slot:
         * - Name of plugin.
         * - Plugin UID.
         * - Plugin alias name.
         * - Plugin font type.
         * - Information about whether the slot is locked or not.
         * - Information about whether the slot is sticky or not.
         * - Information about whether the slot is disabled or not.
         * - Slot duration in ms.
         */
        for (slotId = 0U; slotId < maxSlots; ++slotId)
        {
            DisplayMgr::SlotConfig config;

            (void)displayMgr.getSlotConfig(slotId, config);

            msg += DELIMITER;
            msg += "\"";
            msg += config.name;
            msg += "\"";
            msg += DELIMITER;
            msg += config.uid;
            msg += DELIMITER;
            msg += "\"";
            msg += config.alias;
            msg += "\"";
            msg += DELIMITER;
            msg += "\"";
            msg += Fonts::fontTypeToStr(config.fontType);
            msg += "\"";
            msg += DELIMITER;
            msg += (false == config.isLocked) ? "0" : "1";
            msg += DELIMITER;
            msg += (false == config.isSticky) ? "0" : "1";
            msg += DELIMITER;
            msg += (false == config.isDisabled) ? "0" : "1";
            msg += DELIMITER;
            msg += config.duration;
        }

        sendResponse(server, clientId, msg);
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

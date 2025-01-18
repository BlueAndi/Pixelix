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
 * @brief  Websocket command set slot properties
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WsCmdSlot.h"
#include "DisplayMgr.h"
#include "SlotList.h"

#include <Util.h>
#include <PluginMgr.h>

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

void WsCmdSlot::execute(AsyncWebSocket* server, uint32_t clientId)
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
        DisplayMgr& displayMgr          = DisplayMgr::getInstance();
        bool        isSlotConfigDirty   = false;

        if (FLAG_STATUS_NA != m_stickyFlag)
        {
            /* Clear sticky flag? */
            if (FLAG_STATUS_FALSE == m_stickyFlag)
            {
                /* Clear it only if its set to this slot. */
                if (m_slotId == displayMgr.getStickySlot())
                {
                    displayMgr.clearSticky();
                    isSlotConfigDirty = true;
                }
            }
            /* Set sticky flag. */
            else
            {
                if (m_slotId != displayMgr.getStickySlot())
                {
                    if (false == displayMgr.setSlotSticky(m_slotId))
                    {
                        m_isError = true;
                    }
                    else
                    {
                        isSlotConfigDirty = true;
                    }
                }
            }
        }

        /* If no error happened, continuoue with handling the slot status. */
        if ((false == m_isError) &&
            (FLAG_STATUS_NA != m_isDisabled))
        {
            if (FLAG_STATUS_FALSE == m_isDisabled)
            {
                displayMgr.enableSlot(m_slotId);
                isSlotConfigDirty = true;
            }
            else
            {
                if (false == displayMgr.disableSlot(m_slotId))
                {
                    m_isError = true;
                }
                else
                {
                    isSlotConfigDirty = true;
                }
            }
        }

        if (true == m_isError)
        {
            sendNegativeResponse(server, clientId, "\"Parameter invalid.\"");
        }
        else
        {
            uint8_t             stickySlot  = displayMgr.getStickySlot();
            IPluginMaintenance* plugin      = displayMgr.getPluginInSlot(m_slotId);
            const char*         name        = (nullptr != plugin) ? plugin->getName() : "";
            uint16_t            uid         = (nullptr != plugin) ? plugin->getUID() : 0U;
            String              alias       = (nullptr != plugin) ? plugin->getAlias() : "";
            bool                isLocked    = displayMgr.isSlotLocked(m_slotId);
            bool                isSticky    = (stickySlot == m_slotId) ? true : false;
            bool                isDisabled  = displayMgr.isSlotDisabled(m_slotId);
            uint32_t            duration    = displayMgr.getSlotDuration(m_slotId);

            preparePositiveResponse(msg);

            msg += m_slotId;
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
            msg += (false == isDisabled) ? "0" : "1";
            msg += DELIMITER;
            msg += duration;

            if (true == isSlotConfigDirty)
            {
                /* Ensure that the changes will be available after power-up. */
                PluginMgr::getInstance().save();
            }
        }

        sendResponse(server, clientId, msg);
    }

    m_isError   = false;
    m_parCnt    = 0U;
}

void WsCmdSlot::setPar(const char* par)
{
    uint8_t tmp;

    switch(m_parCnt)
    {
    /* Slot id*/
    case 0U:
        if (false == Util::strToUInt16(par, m_slotId))
        {
            m_isError = true;
        }
        break;

    /* Sticky flag */
    case 1U:
        if (false == Util::strToUInt8(par, tmp))
        {
            m_isError = true;
        }
        else
        {
            m_stickyFlag = static_cast<FlagStatus>(tmp);
        }
        break;

    /* Slot status */
    case 2U:
        if (false == Util::strToUInt8(par, tmp))
        {
            m_isError = true;
        }
        else
        {
            m_isDisabled = static_cast<FlagStatus>(tmp);
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

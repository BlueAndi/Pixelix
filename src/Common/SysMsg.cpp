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
 * @brief  System message
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "SysMsg.h"
#include "DisplayMgr.h"
#include "PluginMgr.h"

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

bool SysMsg::init()
{
    bool status = false;

    m_plugin = static_cast<SysMsgPlugin*>(PluginMgr::getInstance().install("SysMsgPlugin"));

    if (nullptr != m_plugin)
    {
        uint8_t slotId = DisplayMgr::getInstance().getSlotIdByPluginUID(m_plugin->getUID());

        /* Set infinite slot duration, because the system message plugin will enable/disable
         * itself.
         */
        DisplayMgr::getInstance().setSlotDuration(slotId, 0U, false);
        DisplayMgr::getInstance().lockSlot(slotId);
        status = true;
    }

    return status;
}

void SysMsg::show(const String& msg, uint32_t duration, uint32_t max, bool blocking)
{
    if (nullptr != m_plugin)
    {
        m_plugin->show(msg, duration, max);

        /* Schedule plugin immediately */
        DisplayMgr::getInstance().activatePlugin(m_plugin);

        if (true == blocking)
        {
            while(true == m_plugin->isEnabled())
            {
                delay(1U);
            }
        }
    }

    return;
}

bool SysMsg::isReady() const
{
    bool isReady = false;

    if (nullptr != m_plugin)
    {
        isReady = (false == m_plugin->isEnabled()) ? true : false;
    }

    return isReady;
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

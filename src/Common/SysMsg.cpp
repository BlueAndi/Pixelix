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

void SysMsg::show(const String& msg, uint32_t duration, uint32_t max)
{
    if (nullptr != m_plugin)
    {
        uint8_t slotId = DisplayMgr::getInstance().getSlotIdByPluginUID(m_plugin->getUID());

        /* Important: Call first show() to enable plugin. Otherwise the slot activation request will fail. */
        m_plugin->show(msg, duration, max);

        if (false == DisplayMgr::getInstance().activateSlot(slotId))
        {
            LOG_WARNING("System message suppressed.");
        }
    }
}

void SysMsg::enableSignal()
{
    if (nullptr != m_plugin)
    {
        m_plugin->enableSignal();
    }
}

void SysMsg::disableSignal()
{
    if (nullptr != m_plugin)
    {
        m_plugin->disableSignal();
    }
}

bool SysMsg::isActive() const
{
    bool isActive = false;

    if (nullptr != m_plugin)
    {
        isActive = m_plugin->isEnabled();
    }

    return isActive;
}

void SysMsg::next()
{
    if (nullptr != m_plugin)
    {
        m_plugin->next();
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

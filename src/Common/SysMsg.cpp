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

 /* Initialize system message handler instance */
SysMsg  SysMsg::m_instance;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool SysMsg::init()
{
    bool status = false;

    m_plugin = static_cast<SysMsgPlugin*>(PluginMgr::getInstance().install("SysMsgPlugin"));

    if (nullptr != m_plugin)
    {
        DisplayMgr::getInstance().lockSlot(m_plugin->getSlotId());
        status = true;
    }

    return status;
}

void SysMsg::show(const String& msg, uint32_t duration)
{
    if (nullptr != m_plugin)
    {
        m_plugin->enable();
        m_plugin->show(msg, duration);

        /* Schedule plugin immediately */
        DisplayMgr::getInstance().activatePlugin(m_plugin);
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

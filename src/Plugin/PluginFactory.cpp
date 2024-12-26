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
 * @brief  Plugin factory
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "PluginFactory.h"
#include "PluginList.h"

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

IPluginMaintenance* PluginFactory::createPlugin(const char* name)
{
    return createPlugin(name, generateUID());
}

IPluginMaintenance* PluginFactory::createPlugin(const char* name, uint16_t uid)
{
    IPluginMaintenance*         plugin                  = nullptr;
    uint8_t                     pluginTypeListLength    = 0U;
    const PluginList::Element*  pluginTypeList          = PluginList::getList(pluginTypeListLength);
    uint8_t                     idx                     = 0U;

    /* Walk through registry and find the requested plugin type. */
    while((nullptr == plugin) && (pluginTypeListLength > idx))
    {
        const PluginList::Element* elem = &pluginTypeList[idx];

        /* Plugin type found? */
        if (0 == strcmp(name, elem->name))
        {
            /* Produce the plugin object. Its important to use the name from the PluginList,
             * because it must exist over plugin instance lifetime.
             */
            plugin = elem->createFunc(elem->name, uid);

            if (nullptr != plugin)
            {
                m_plugins.push_back(plugin);
            }
        }

        ++idx;
    }

    return plugin;
}

void PluginFactory::destroyPlugin(const IPluginMaintenance* plugin)
{
    if (nullptr != plugin)
    {
        if (0U < m_plugins.size())
        {
            ListOfPlugins::iterator it = m_plugins.begin();

            while(it != m_plugins.end())
            {
                IPluginMaintenance* currentPlugin = *it;

                if (currentPlugin == plugin)
                {
                    it = m_plugins.erase(it);

                    delete currentPlugin;
                }
                else
                {
                    ++it;
                }
            }
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

uint16_t PluginFactory::generateUID()
{
    uint16_t    uid;
    bool        isFound;
    
    do
    {
        isFound = false;
        uid     = random(UINT16_MAX);

        if (0U < m_plugins.size())
        {
            ListOfPlugins::const_iterator it = m_plugins.begin();

            /* Ensure that UID is really unique. */
            while((it != m_plugins.end()) && (false == isFound))
            {
                const IPluginMaintenance* currentPlugin = *it;

                if (currentPlugin->getUID() == uid)
                {
                    isFound = true;
                }

                ++it;
            }
        }
    }
    while(true == isFound);

    return uid;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

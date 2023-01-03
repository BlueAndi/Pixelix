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
 * @brief  Plugin factory
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "PluginFactory.h"

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

void PluginFactory::registerPlugin(const String& name, IPluginMaintenance::CreateFunc createFunc)
{
    PluginRegEntry* entry = new(std::nothrow) PluginRegEntry();

    if (nullptr != entry)
    {
        entry->name         = name;
        entry->createFunc   = createFunc;

        if (false == m_registry.append(entry))
        {
            LOG_ERROR("Couldn't add %s to registry.", name.c_str());

            delete entry;
            entry = nullptr;
        }
        else
        {
            LOG_INFO("Plugin type %s registered.", name.c_str());
        }
    }
    else
    {
        LOG_ERROR("Couldn't add %s to registry.", name.c_str());
    }

    return;
}

IPluginMaintenance* PluginFactory::createPlugin(const String& name)
{
    return createPlugin(name, generateUID());
}

IPluginMaintenance* PluginFactory::createPlugin(const String& name, uint16_t uid)
{
    IPluginMaintenance*                     plugin  = nullptr;
    PluginRegEntry*                         entry   = nullptr;
    DLinkedListIterator<PluginRegEntry*>    itPluginReg(m_registry);

    /* Walk through registry and find the requested plugin type. */
    if (true == itPluginReg.first())
    {
        bool isFound = false;

        /* Find plugin type in the registry */
        entry = *itPluginReg.current();

        while((false == isFound) && (nullptr != entry))
        {
            if (name == entry->name)
            {
                isFound = true;
            }
            else if (false == itPluginReg.next())
            {
                entry = nullptr;
            }
            else
            {
                entry = *itPluginReg.current();
            }
        }

        /* Plugin type found? */
        if ((true == isFound) &&
            (nullptr != entry))
        {
            /* Produce the plugin object. */
            plugin = entry->createFunc(entry->name, uid);

            if (nullptr != plugin)
            {
                m_plugins.append(plugin);
            }
        }
    }

    return plugin;
}

void PluginFactory::destroyPlugin(IPluginMaintenance* plugin)
{
    if (nullptr != plugin)
    {
        DLinkedListIterator<IPluginMaintenance*> it(m_plugins);

        if (false == it.find(plugin))
        {
            LOG_WARNING("Plugin 0x%X (%s) not found in list.", plugin, plugin->getName());
        }
        else
        {
            it.remove();
            delete plugin;
        }
    }
}

const char* PluginFactory::findFirst()
{
    const char* name = nullptr;

    if (true == m_registryIter.first())
    {
        name = (*m_registryIter.current())->name.c_str();
    }

    return name;
}

const char* PluginFactory::findNext()
{
    const char* name = nullptr;

    if (true == m_registryIter.next())
    {
        name = (*m_registryIter.current())->name.c_str();
    }

    return name;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

uint16_t PluginFactory::generateUID()
{
    uint16_t                                        uid;
    bool                                            isFound;
    DLinkedListConstIterator<IPluginMaintenance*>   it(m_plugins);

    do
    {
        isFound = false;
        uid     = random(UINT16_MAX);

        /* Ensure that UID is really unique. */
        if (true == it.first())
        {
            const IPluginMaintenance* plugin = *it.current();

            while((false == isFound) && (nullptr != plugin))
            {
                if (uid == plugin->getUID())
                {
                    isFound = true;
                }
                else if (false == it.next())
                {
                    plugin = nullptr;
                }
                else
                {
                    plugin = *it.current();
                }
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

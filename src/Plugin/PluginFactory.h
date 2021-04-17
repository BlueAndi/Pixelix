/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle Merkle <web@blue-andi.de>
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
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __PLUGINFACTORY_H__
#define __PLUGINFACTORY_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "IPluginMaintenance.hpp"

#include <LinkedList.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The plugin factory produces plugin object of a given type.
 * All plugin types, which shall be produced, must be registered
 * at runtime.
 */
class PluginFactory
{
public:

    /**
     * Constructs the plugin factory.
     */
    PluginFactory() :
        m_registry(),
        m_registryIter(m_registry),
        m_plugins()
    {
    }

    /**
     * Destroys the plugin factory.
     * Please note, that if the factory is destroyed, all produced plugin objects
     * will survive. This is a feature where a factory could live shorter than its
     * produced plugin objects.
     */
    ~PluginFactory()
    {
    }

    /**
     * Register a plugin, so the factory is able to produce it.
     *
     * @param[in] name          Plugin name
     * @param[in] createFunc    The plugin creation function.
     */
    void registerPlugin(const String& name, IPluginMaintenance::CreateFunc createFunc);

    /**
     * Create a plugin by name.
     * 
     * @param[in] name  Plugin type name
     * 
     * @return If successful, it will return the plugin object pointer otherwise nullptr.
     */
    IPluginMaintenance* createPlugin(const String& name);

    /**
     * Destroy plugin object.
     * 
     * @param[in] plugin    The plugin object pointer.
     */
    void destroyPlugin(IPluginMaintenance* plugin);

    /**
     * Find first registered plugin type.
     *
     * @return If plugin type found, it will return its name otherwise nullptr.
     */
    const char* findFirst();

    /**
     * Find next registered plugin type.
     *
     * @return If plugin type found, it will return its name otherwise nullptr.
     */
    const char* findNext();

private:

    /**
     * Plugin registry entry.
     */
    struct PluginRegEntry
    {
        String                          name;       /**< Plugin name */
        IPluginMaintenance::CreateFunc  createFunc; /**< Plugin creation function */
    };

    DLinkedList<PluginRegEntry*>            m_registry;     /**< Plugin registry, which contains all registered plugin types. */
    DLinkedListIterator<PluginRegEntry*>    m_registryIter; /**< Plugin registry iterator. Exclusive use in findFirst() and findNext()! */
    DLinkedList<IPluginMaintenance*>        m_plugins;      /**< List with all produced plugin objects. */

    PluginFactory(const PluginFactory& factory);
    PluginFactory& operator=(const PluginFactory& factory);

    /**
     * Generate a 16-bit unique id, for a plugin instance.
     *
     * @return Unique id
     */
    uint16_t generateUID();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __PLUGINFACTORY_H__ */

/** @} */
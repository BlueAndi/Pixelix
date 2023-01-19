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
 * @brief  Topic handler interface
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __ITOPICHANDLER_H__
#define __ITOPICHANDLER_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IPluginMaintenance.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The topic handler interface, used by the plugin manager to register/unregister
 * plugin topics.
 */
class ITopicHandler
{
public:

    /**
     * Destroy the interface.
     */
    ~ITopicHandler()
    {
    }

    /**
     * Register all topics of the given plugin.
     * 
     * @param[in] plugin    The plugin, which topics shall be registered.
     */
    virtual void registerTopics(IPluginMaintenance* plugin) = 0;

    /**
     * Unregister all topics of the given plugin.
     * 
     * @param[in] plugin    The plugin, which topics to unregister.
     */
    virtual void unregisterTopics(IPluginMaintenance* plugin) = 0;

protected:

    /**
     * Construct the interface.
     */
    ITopicHandler()
    {
    }

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __ITOPICHANDLER_H__ */

/** @} */
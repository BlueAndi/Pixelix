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

#ifndef ITOPICHANDLER_H
#define ITOPICHANDLER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IPluginMaintenance.hpp>
#include <ArduinoJson.h>

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

    /** Topic accessibility */
    typedef enum
    {
        ACCESS_READ_ONLY = 0,   /**< Read only */
        ACCESS_READ_WRITE,      /**< Read and write */
        ACCESS_WRITE_ONLY       /**< Write only */
    
    } Access;

    /**
     * Destroy the interface.
     */
    ~ITopicHandler()
    {
    }

    /**
     * Start the topic handler.
     */
    virtual void start() = 0;

    /**
     * Stop the topic handler.
     */
    virtual void stop() = 0;

    /**
     * Register a single topic of the given plugin.
     * 
     * @param[in] plugin    The plugin which provides the topic.
     * @param[in] topic     The topic name.
     * @param[in] access    The topic accessibility.
     * @param[in] extra     Extra parameters, which depend on the topic handler.
     */
    virtual void registerTopic(IPluginMaintenance* plugin, const String& topic, Access access, JsonObjectConst& extra) = 0;

    /**
     * Unregister the topic of the given plugin.
     * 
     * @param[in] plugin    The plugin which provides the topic.
     * @param[in] topic     The topic name.
     */
    virtual void unregisterTopic(IPluginMaintenance* plugin, const String& topic) = 0;

    /**
     * Process the topic handler.
     */
    virtual void process() = 0;

    /**
     * Notify that the topic has changed.
     * 
     * @param[in] plugin    The plugin which provides the topic.
     * @param[in] topic     The topic name.
     */
    virtual void notify(IPluginMaintenance* plugin, const String& topic) = 0;

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

#endif  /* ITOPICHANDLER_H */

/** @} */
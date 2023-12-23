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
 * The abstract topic handler interface, which will be realized by different
 * protocols.
 */
class ITopicHandler
{
public:

    /** Function prototype to get topic content. */
    typedef std::function<bool(const String& topic, JsonObject& value)> GetTopicFunc;

    /** Function prototype to set topic content. */
    typedef std::function<bool(const String& topic, const JsonObjectConst& value)> SetTopicFunc;

    /** Function prototype for file upload request. */
    typedef std::function<bool(const String& topic, const String& srcFilename, String& dstFilename)> UploadReqFunc;

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
     * Register the topic.
     * 
     * @param[in] deviceId      The device id which represents the physical device.
     * @param[in] entityId      The entity id which represents the entity of the device.
     * @param[in] topic         The topic name.
     * @param[in] extra         Extra parameters, which depend on the topic handler.
     * @param[in] getTopicFunc  Function to get the topic content.
     * @param[in] setTopicFunc  Function to set the topic content.
     * @param[in] uploadReqFunc Function used for requesting whether an file upload is allowed.
     */
    virtual void registerTopic(const String& deviceId, const String& entityId, const String& topic, JsonObjectConst& extra, GetTopicFunc getTopicFunc, SetTopicFunc setTopicFunc, UploadReqFunc uploadReqFunc) = 0;

    /**
     * Unregister the topic.
     * 
     * @param[in] deviceId  The device id which represents the physical device.
     * @param[in] entityId  The entity id which represents the entity of the device.
     * @param[in] topic     The topic name.
     */
    virtual void unregisterTopic(const String& deviceId, const String& entityId, const String& topic) = 0;

    /**
     * Process the topic handler.
     */
    virtual void process() = 0;

    /**
     * Notify that the topic has changed.
     * 
     * @param[in] deviceId  The device id which represents the physical device.
     * @param[in] entityId  The entity id which represents the entity of the device.
     * @param[in] topic     The topic name.
     */
    virtual void notify(const String& deviceId, const String& entityId, const String& topic) = 0;

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
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
 * @brief  Plugin maintenance interface
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __IPLUGIN_MAINTENANCE_HPP__
#define __IPLUGIN_MAINTENANCE_HPP__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IGfx.hpp>
#include <HttpStatus.h>
#include <ESPAsyncWebServer.h>
#include <Util.h>
#include <ArduinoJson.h>
#include "ISlotPlugin.hpp"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The plugin maintenance interface is used for plugin handling.
 */
class IPluginMaintenance
{
public:

    /**
     * IPluginMaintenance creation function, used by the plugin manager to create a plugin instance.
     */
    typedef IPluginMaintenance* (*CreateFunc)(const String& name, uint16_t uid);

    /**
     * Destroys the interface.
     */
    virtual ~IPluginMaintenance()
    {
    }

    /**
     * Set the slot interface, which the plugin can used to request information
     * from the slot, it is plugged in.
     *
     * @param[in] slotInterf    Slot interface
     */
    virtual void setSlot(const ISlotPlugin* slotInterf) = 0;

    /**
     * Get unique instance id.
     *
     * @return Unique id
     */
    virtual uint16_t getUID() const = 0;

    /**
     * Get plugin topics, which can be get/set via different communication
     * interfaces like REST, websocket, MQTT, etc.
     * 
     * Example:
     * {
     *     "topics": [
     *         "/text"
     *     ]
     * }
     * 
     * @param[out] topics   Topis in JSON format
     */
    virtual void getTopics(JsonArray& topics) const = 0;

    /**
     * Get a topic data.
     * Note, currently only JSON format is supported.
     * 
     * @param[in]   topic   The topic which data shall be retrieved.
     * @param[out]  value   The topic value in JSON format.
     * 
     * @return If successful it will return true otherwise false.
     */
    virtual bool getTopic(const String& topic, JsonObject& value) const = 0;

    /**
     * Set a topic data.
     * Note, currently only JSON format is supported.
     * 
     * @param[in]   topic   The topic which data shall be retrieved.
     * @param[in]   value   The topic value in JSON format.
     * 
     * @return If successful it will return true otherwise false.
     */
    virtual bool setTopic(const String& topic, const JsonObject& value) = 0;

    /**
     * Register web interface, e.g. REST API functionality.
     * Overwrite it, if your plugin provides a web interface.
     *
     * @param[in] srv       Webserver
     * @param[in] baseUri   Base URI, use this and append plugin specific part.
     */
    virtual void registerWebInterface(AsyncWebServer& srv, const String& baseUri) = 0;

    /**
     * Unregister web interface.
     * Overwrite it, if your plugin provides a web interface.
     *
     * @param[in] srv   Webserver
     */
    virtual void unregisterWebInterface(AsyncWebServer& srv) = 0;

    /**
     * Get the plugin name.
     *
     * @return Name of the plugin.
     */
    virtual const char* getName() const = 0;

    /**
     * Is plugin enabled or not?
     *
     * @return If plugin is enabled, it will return true otherwise false.
     */
    virtual bool isEnabled() const = 0;

    /**
     * Enable plugin.
     * Only a enabled plugin will be scheduled.
     */
    virtual void enable() = 0;

    /**
     * Disable plugin.
     * A disabled plugin won't be scheduled in the next cycle.
     * Note, calling this doesn't abort a active phase.
     */
    virtual void disable() = 0;

    /**
     * Start the plugin.
     * Overwrite it if your plugin needs to know that it was installed.
     */
    virtual void start() = 0;

    /**
     * Stop the plugin.
     * Overwrite it if your plugin needs to know that it will be uninstalled.
     */
    virtual void stop() = 0;

    /**
     * Process the plugin.
     * Overwrite it if your plugin has cyclic stuff to do without being in a
     * active slot.
     */
    virtual void process() = 0;

    /**
     * This method will be called in case the plugin is set active, which means
     * it will be shown on the display in the next step.
     * Overwrite it if your plugin needs to know this.
     *
     * @param[in] gfx   Display graphics interface
     */
    virtual void active(IGfx& gfx) = 0;

    /**
     * This method will be called in case the plugin is set inactive, which means
     * it won't be shown on the display anymore.
     * Overwrite it if your plugin needs to know this.
     */
    virtual void inactive() = 0;

    /**
     * Update the display.
     * If the plugin is in active slot, this function will be called cyclic
     * as long as the slot is active.
     *
     * @param[in] gfx   Display graphics interface
     */
    virtual void update(IGfx& gfx) = 0;

protected:

    /**
     * Constructs the interface.
     */
    IPluginMaintenance()
    {
    }

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __IPLUGIN_MAINTENANCE_HPP__ */

/** @} */
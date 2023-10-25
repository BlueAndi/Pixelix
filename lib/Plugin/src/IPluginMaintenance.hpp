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
 * @brief  Plugin maintenance interface
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef IPLUGIN_MAINTENANCE_HPP
#define IPLUGIN_MAINTENANCE_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <YAGfx.h>
#include <ArduinoJson.h>
#include <Fonts.h>
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
     * Set instance alias name, which is more user friendly than the UID.
     * 
     * @param[in] alias Plugin instance alias name
     */
    virtual void setAlias(const String& alias) = 0;

    /**
     * Get instance alias name.
     * 
     * @return Plugin instance alias name
     */
    virtual String getAlias() const = 0;

    /**
     * Get font type.
     * 
     * @return The font type the plugin uses.
     */
    virtual Fonts::FontType getFontType() const = 0;

    /**
     * Set font type.
     * The plugin may skip the font type in case it gets conflicts with the layout.
     * 
     * A font type change will only be considered if it is set before the start()
     * method is called!
     * 
     * @param[in] fontType  The font type which the plugin shall use.
     */
    virtual void setFontType(Fonts::FontType fontType) = 0;

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
     * By default a topic is readable and writeable.
     * This can be set explicit with the "access" key with the following possible
     * values:
     * - Only readable: "r"
     * - Only writeable: "w"
     * - Readable and writeable: "rw"
     * 
     * Example:
     * {
     *     "topics": [{
     *         "name": "/text",
     *         "access": "r"
     *     }]
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
    virtual bool setTopic(const String& topic, const JsonObjectConst& value) = 0;

    /**
     * Has the topic content changed since last time?
     * Every readable volatile topic shall support this. Otherwise the topic
     * handlers might not be able to provide updated information.
     * 
     * @param[in] topic The topic which to check.
     * 
     * @return If the topic content changed since last time, it will return true otherwise false.
     */
    virtual bool hasTopicChanged(const String& topic) = 0;

    /**
     * Is a upload request accepted or rejected?
     * 
     * @param[in] topic         The topic which the upload belongs to.
     * @param[in] srcFilename   Name of the file, which will be uploaded if accepted.
     * @param[in] dstFilename   The destination filename, after storing the uploaded file.
     * 
     * @return If accepted it will return true otherwise false.
     */
    virtual bool isUploadAccepted(const String& topic, const String& srcFilename, String& dstFilename) = 0;

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
     * Start the plugin. This is called only once during plugin lifetime.
     * It can be used as deferred initialization (after the constructor)
     * and provides the canvas size.
     * 
     * If your display layout depends on canvas or font size, calculate it
     * here.
     * 
     * Overwrite it if your plugin needs to know that it was installed.
     * 
     * @param[in] width     Display width in pixel
     * @param[in] height    Display height in pixel
     */
    virtual void start(uint16_t width, uint16_t height) = 0;

    /**
     * Stop the plugin. This is called only once during plugin lifetime.
     * It can be used as a first clean-up, before the plugin will be destroyed.
     * 
     * Overwrite it if your plugin needs to know that it will be uninstalled.
     */
    virtual void stop() = 0;

    /**
     * Process the plugin.
     * Overwrite it if your plugin has cyclic stuff to do without being in a
     * active slot.
     * 
     * @param[in] isConnected   The network connection status. If network
     *                          connection is established, it will be true otherwise false.
     */
    virtual void process(bool isConnected) = 0;

    /**
     * This method will be called in case the plugin is set active, which means
     * it will be shown on the display in the next step.
     * Overwrite it if your plugin needs to know this.
     *
     * @param[in] gfx   Display graphics interface
     */
    virtual void active(YAGfx& gfx) = 0;

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
    virtual void update(YAGfx& gfx) = 0;

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

#endif  /* IPLUGIN_MAINTENANCE_HPP */

/** @} */
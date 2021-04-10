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
 * @brief  Plugin interface
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __PLUGIN_HPP__
#define __PLUGIN_HPP__

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
#include "IPluginMaintenance.hpp"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * A plugin can be plugged into a display slot and will shown.
 */
class Plugin : public IPluginMaintenance
{
public:

    /**
     * Destroys the plugin.
     */
    virtual ~Plugin()
    {
    }

    /**
     * Set the slot interface, which the plugin can used to request information
     * from the slot, it is plugged in.
     *
     * @param[in] slotInterf    Slot interface
     */
    virtual void setSlot(const ISlotPlugin* slotInterf) override
    {
        UTIL_NOT_USED(slotInterf);
        return;
    }

    /**
     * Get unique id.
     *
     * @return Unique id
     */
    uint16_t getUID() const  override
    {
        return m_uid;
    }

    /**
     * Register web interface, e.g. REST API functionality.
     * Overwrite it, if your plugin provides a web interface.
     *
     * @param[in] srv       Webserver
     * @param[in] baseUri   Base URI, use this and append plugin specific part.
     */
    virtual void registerWebInterface(AsyncWebServer& srv, const String& baseUri) override
    {
        UTIL_NOT_USED(srv);
        UTIL_NOT_USED(baseUri);
        return;
    }

    /**
     * Unregister web interface.
     * Overwrite it, if your plugin provides a web interface.
     *
     * @param[in] srv   Webserver
     */
    virtual void unregisterWebInterface(AsyncWebServer& srv) override
    {
        UTIL_NOT_USED(srv);
        return;
    }

    /**
     * Get the plugin name.
     *
     * @return Name of the plugin.
     */
    const char* getName() const override
    {
        return m_name.c_str();
    }

    /**
     * Is plugin enabled or not?
     *
     * @return If plugin is enabled, it will return true otherwise false.
     */
    bool isEnabled() const override
    {
        return m_isEnabled;
    }

    /**
     * Enable plugin.
     * Only a enabled plugin will be scheduled.
     */
    void enable() override
    {
        m_isEnabled = true;
        return;
    }

    /**
     * Disable plugin.
     * A disabled plugin won't be scheduled in the next cycle.
     * Note, calling this doesn't abort a active phase.
     */
    void disable() override
    {
        m_isEnabled = false;
        return;
    }

    /**
     * Start the plugin.
     * Overwrite it if your plugin needs to know that it was installed.
     */
    virtual void start() override
    {
        return;
    }

    /**
     * Stop the plugin.
     * Overwrite it if your plugin needs to know that it will be uninstalled.
     */
    virtual void stop() override
    {
        return;
    }

    /**
     * Process the plugin.
     * Overwrite it if your plugin has cyclic stuff to do without being in a
     * active slot.
     */
    virtual void process() override
    {
        return;
    }

    /**
     * This method will be called in case the plugin is set active, which means
     * it will be shown on the display in the next step.
     * Overwrite it if your plugin needs to know this.
     *
     * @param[in] gfx   Display graphics interface
     */
    virtual void active(IGfx& gfx) override
    {
        UTIL_NOT_USED(gfx);
        return;
    }

    /**
     * This method will be called in case the plugin is set inactive, which means
     * it won't be shown on the display anymore.
     * Overwrite it if your plugin needs to know this.
     */
    virtual void inactive() override
    {
        return;
    }

    /**
     * Update the display.
     * If the plugin is in active slot, this function will be called cyclic
     * as long as the slot is active.
     *
     * @param[in] gfx   Display graphics interface
     */
    virtual void update(IGfx& gfx) = 0;

    /**
     * Path where plugin specific configuration files shall be stored.
     */
    static constexpr const char*    CONFIG_PATH = "/configuration";

protected:

    /**
     * Constructs the plugin.
     * It is disabled by default.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    Plugin(const String& name, uint16_t uid) :
        m_uid(uid),
        m_name(name),
        m_isEnabled(false)
    {
    }

    /**
     * Generate the full path for any plugin instance specific kind of configuration
     * file.
     *
     * @return Full path
     */
    String generateFullPath(const String& extension) const
    {
        return String(CONFIG_PATH) + "/" + getUID() + extension;
    }

    /**
     * Get full path (path + filename) to plugin instance specific configuration
     * in JSON format.
     * 
     * @return Full path to configuration file
     */
    String getFullPathToConfiguration() const
    {
        return generateFullPath(".json");
    }

private:

    uint16_t    m_uid;          /**< Unique id */
    String      m_name;         /**< Plugin name */
    bool        m_isEnabled;    /**< Plugin is enabled or disabled */

    Plugin();
    Plugin(const Plugin& plugin);
    Plugin& operator=(const Plugin& plugin);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __PLUGIN_HPP__ */

/** @} */
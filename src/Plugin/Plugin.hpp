/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
#include <Adafruit_GFX.h>
#include <HttpStatus.h>
#include <ESPAsyncWebServer.h>
#include <Util.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * A plugin can be plugged into a display slot and will shown.
 */
class Plugin
{
public:

    /**
     * Constructs the plugin.
     * It is enabled by default.
     */
    Plugin() :
        m_slotId(UINT8_MAX),
        m_isEnabled(true)
    {
    }

    /**
     * Destroys the plugin.
     */
    virtual ~Plugin()
    {
    }

    /**
     * Set the slot id.
     *
     * @param[in] slotId    Slot id
     */
    void setSlotId(uint8_t slotId)
    {
        m_slotId = slotId;
        return;
    }

    /**
     * Get the unique slot id.
     * 
     * @return Slot id
     */
    uint8_t getSlotId(void) const
    {
        return m_slotId;
    }

    /**
     * Register web interface, e.g. REST API functionality.
     * Overwrite it, if your plugin provides a web interface.
     * 
     * @param[in] srv   Webserver
     */
    virtual void registerWebInterface(AsyncWebServer& srv)
    {
        UTIL_NOT_USED(srv);
        return;
    }

    /**
     * Unregister web interface.
     * Overwrite it, if your plugin provides a web interface.
     * 
     * @param[in] srv   Webserver
     */
    virtual void unregisterWebInterface(AsyncWebServer& srv)
    {
        UTIL_NOT_USED(srv);
        return;
    }

    /**
     * Get the plugin name.
     *
     * @return Name of the plugin.
     */
    virtual const char* getName(void) const = 0;

    /**
     * Is plugin enabled or not?
     * 
     * @return If plugin is enabled, it will return true otherwise false.
     */
    bool isEnabled(void) const
    {
        return m_isEnabled;
    }

    /**
     * Enable plugin.
     * Only a enabled plugin will be scheduled.
     */
    void enable(void)
    {
        m_isEnabled = true;
        return;
    }

    /**
     * Disable plugin.
     * A disabled plugin won't be scheduled in the next cycle.
     * Note, calling this doesn't abort a active phase.
     */
    void disable(void)
    {
        m_isEnabled = false;
        return;
    }

    /**
     * Start the plugin.
     * If the plugin is scheduled the first time, this method will be called.
     * Overwrite this method, in case you want to do something special at the begin.
     * 
     * @param[out] duration Minimum duration in ms how long the plugin want to show something at least.
     *                      The update() method may extend this time, but can not shorten it.
     */
    virtual void start(uint32_t& duration)
    {
        duration = 30000u; /* 30 s */

        return;
    }

    /**
     * Stop the plugin.
     * If the plugin is removed from scheduler, this method will be called.
     * Overwrite this method, in case you want to do something special at the end.
     */
    virtual void stop(void)
    {
        return;
    }

    /**
     * Update the display.
     * The scheduler will call this method periodically.
     * 
     * @param[in] gfx   Display graphics interface
     * 
     * @return  As long as the plugin want to show something, it will return true otherwise false.
     *          If a minimum duration is given (see start() method), it will run this time at least.
     *          To extend it, the plugin may return true.
     */
    virtual bool update(Adafruit_GFX& gfx) = 0;

private:

    uint8_t m_slotId;       /**< Slot id */
    bool    m_isEnabled;    /**< Plugin is enabled or disabled */

    Plugin(const Plugin& plugin);
    Plugin& operator=(const Plugin& plugin);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __PLUGIN_HPP__ */

/** @} */
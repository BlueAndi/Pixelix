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
 * @brief  WiFi status plugin
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef WIFI_STATUS_PLUGIN_H
#define WIFI_STATUS_PLUGIN_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "Plugin.hpp"

#include <WidgetGroup.h>
#include <TextWidget.h>
#include <SimpleTimer.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Shows the current wifi status on the display.
 */
class WifiStatusPlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    WifiStatusPlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_iconCanvas(),
        m_textCanvas(),
        m_textWidget(),
        m_alertWidget(),
        m_timer(),
        m_toggle(true)
    {
    }

    /**
     * Destroys the plugin.
     */
    ~WifiStatusPlugin()
    {
    }

    /**
     * Plugin creation method, used to register on the plugin manager.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     *
     * @return If successful, it will return the pointer to the plugin instance, otherwise nullptr.
     */
    static IPluginMaintenance* create(const String& name, uint16_t uid)
    {
        return new(std::nothrow)WifiStatusPlugin(name, uid);
    }

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
    void start(uint16_t width, uint16_t height) final;

    /**
     * Stop the plugin. This is called only once during plugin lifetime.
     * It can be used as a first clean-up, before the plugin will be destroyed.
     * 
     * Overwrite it if your plugin needs to know that it will be uninstalled.
     */
    void stop() final;
    
    /**
     * This method will be called in case the plugin is set active, which means
     * it will be shown on the display in the next step.
     *
     * @param[in] gfx   Display graphics interface
     */
    void active(YAGfx& gfx) final;

    /**
     * This method will be called in case the plugin is set inactive, which means
     * it won't be shown on the display anymore.
     */
    void inactive() final;

    /**
     * Update the display.
     * The scheduler will call this method periodically.
     *
     * @param[in] gfx   Display graphics interface
     */
    void update(YAGfx& gfx) final;

private:

    /**
     * Period in ms which is used to update the status information.
     */
    const uint32_t  PERIOD                  = 500U;

    /**
     * Width in pixel of a single signal strength bar.
     */
    const uint16_t  WIFI_BAR_WIDTH          = 2U;

    /**
     * Width in pixel of a single signal strength bar.
     */
    const uint16_t  WIFI_BAR_SPACE_WIDTH    = 1U;

    /**
     * Height in pixel of the lowest signal strength bar.
     */
    const uint16_t  WIFI_BAR_HEIGHT         = 2U;

    /**
     * Number of signal signal strength bars.
     */
    const uint8_t   WIFI_BARS               = 4U;

    /**
     * Width in pixel of the whole signal strength icon.
     * Between each bar is a short space.
     */
    const uint16_t  WIFI_ICON_WIDTH         = (WIFI_BARS * WIFI_BAR_WIDTH) + ((WIFI_BARS - 1U) * WIFI_BAR_SPACE_WIDTH);

    /**
     * Height in pixel of the whole signal strength icon.
     */
    const uint16_t  WIFI_ICON_HEIGHT        = 8U;

    WidgetGroup m_iconCanvas;       /**< Drawing area of the wifi icon */
    WidgetGroup m_textCanvas;       /**< Drawing area of the text */
    TextWidget  m_textWidget;       /**< Text widget, used for showing the text. */
    TextWidget  m_alertWidget;      /**< Text widget, used for showing alert (wifi disconnected). */
    SimpleTimer m_timer;            /**< Timer for periodic stuff */
    bool        m_toggle;           /**< Toggles the alert in case wifi is disconnected */

    /**
     * Update wifi status on display.
     *
     * @param[in] gfx       Graphic operations
     * @param[in] quality   Signal quality in percent [0; 100].
     */
    void updateWifiStatus(YAGfx& gfx, uint8_t quality);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* WIFI_STATUS_PLUGIN_H */

/** @} */
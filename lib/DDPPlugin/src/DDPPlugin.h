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
 * @brief  Distributed Display Protocol plugin
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef DDPPLUGIN_H
#define DDPPLUGIN_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <Plugin.hpp>
#include <YAGfxBitmap.h>
#include "DDPServer.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Plugin to handle Distributed Display Protocol (DDP) traffic as display server.
 * http://www.3waylabs.com/ddp/
 */
class DDPPlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    DDPPlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_server(),
        m_mutex(),
        m_framebuffer(),
        m_isUpdated(false)
    {
        (void)m_mutex.create();
    }

    /**
     * Destroys the plugin.
     */
    ~DDPPlugin()
    {
        m_mutex.destroy();
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
        return new(std::nothrow) DDPPlugin(name, uid);
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

    DDPServer           m_server;       /**< DDP server */
    Mutex               m_mutex;        /**< Mutex to protect against concurrent access */
    YAGfxDynamicBitmap  m_framebuffer;  /**< Framebuffer used for synchronization */
    bool                m_isUpdated;    /**< Is framebuffer updated and ready to show? */

    /**
     * On data reception, this method will be called from a different context.
     * 
     * @param[in] format                Format of the payload data
     * @param[in] offset                Byte offset in display framebuffer where to continue
     * @param[in] bitsPerPixelElement   Bits per pixel in payload data
     * @param[in] payload               Payload data
     * @param[in] payloadSize           Payload data size in byte
     * @param[in] isFinal               If final, its the last data and display shall show it. Otherwise more data will come.
     */
    void onData(DDPServer::Format format, uint32_t offset, uint8_t bitsPerPixelElement, uint8_t* payload, uint16_t payloadSize, bool isFinal);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* DDPPLUGIN_H */

/** @} */
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
 * @brief  Gets Bitcoin USD exchange rate
 * @author Flavio Curti <fcu-github@no-way.org>
 * @addtogroup plugin
 *
 * @{
 */

#ifndef BTCQUOTEPLUGIN_H
#define BTCQUOTEPLUGIN_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "AsyncHttpClient.h"
#include "Plugin.hpp"

#include <WidgetGroup.h>
#include <BitmapWidget.h>
#include <stdint.h>
#include <TextWidget.h>
#include <SimpleTimer.hpp>
#include <TaskProxy.hpp>
#include <Mutex.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Loads the current exchange rate from Bitcoin to USD. If price is properly loaded,
 * it will be updated every 15 minutes.
 *
 * Powered by CoinDesk - https://www.coindesk.com/price/bitcoin
 */
class BTCQuotePlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    BTCQuotePlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_fontType(Fonts::FONT_TYPE_DEFAULT),
        m_textCanvas(),
        m_iconCanvas(),
        m_bitmapWidget(),
        m_textWidget("\\calign?"),
        m_relevantResponsePart(""),
        m_client(),
        m_mutex(),
        m_requestTimer(),
        m_taskProxy()
    {
        (void)m_mutex.create();
    }

    /**
     * Destroys the plugin.
     */
    ~BTCQuotePlugin()
    {
        m_client.regOnResponse(nullptr);
        m_client.regOnClosed(nullptr);
        m_client.regOnError(nullptr);

        /* Abort any pending TCP request to avoid getting a callback after the
         * object is destroyed.
         */
        m_client.end();
        
        clearQueue();

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
        return new(std::nothrow) BTCQuotePlugin(name, uid);
    }

    /**
     * Get font type.
     * 
     * @return The font type the plugin uses.
     */
    Fonts::FontType getFontType() const final
    {
        return m_fontType;
    }

    /**
     * Set font type.
     * The plugin may skip the font type in case it gets conflicts with the layout.
     * 
     * A font type change will only be considered if it is set before the start()
     * method is called!
     * 
     * @param[in] fontType  The font type which the plugin shall use.
     */
    void setFontType(Fonts::FontType fontType) final
    {
        m_fontType = fontType;
        return;
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
     * Process the plugin.
     * Overwrite it if your plugin has cyclic stuff to do without being in a
     * active slot.
     * 
     * @param[in] isConnected   The network connection status. If network
     *                          connection is established, it will be true otherwise false.
     */
    void process(bool isConnected) final;

    /**
     * Update the display.
     * The scheduler will call this method periodically.
     *
     * @param[in] gfx   Display graphics interface
     */
    void update(YAGfx& gfx) final;

private:

    /**
     * Icon width in pixels.
     */
    static const int16_t    ICON_WIDTH          = 8;

    /**
     * Icon height in pixels.
     */
    static const int16_t    ICON_HEIGHT         = 8;

    /**
     * Image path within the filesystem.
     */
    static const char*      BTC_USD_IMAGE_PATH;
    /**
     * Period in ms for requesting quotes from Server (15 Minutes) (1 for testing!)
     * This is used in case the last request to the server was successful.
     */
    static const uint32_t   UPDATE_PERIOD       = SIMPLE_TIMER_MINUTES(15U);

    /**
     * Short period in ms for requesting quote from server.
     * This is used in case the request to the server failed.
     */
    static const uint32_t   UPDATE_PERIOD_SHORT = SIMPLE_TIMER_MINUTES(1U);

    Fonts::FontType     m_fontType;                 /**< Font type which shall be used if there is no conflict with the layout. */
    WidgetGroup         m_textCanvas;               /**< Canvas used for the text widget. */
    WidgetGroup         m_iconCanvas;               /**< Canvas used for the bitmap widget. */
    BitmapWidget        m_bitmapWidget;             /**< Bitmap widget, used to show the icon. */
    TextWidget          m_textWidget;               /**< Text widget, used for showing the text. */
    String              m_relevantResponsePart;     /**< String used for the relevant part of the HTTP response. */
    AsyncHttpClient     m_client;                   /**< Asynchronous HTTP client. */
    MutexRecursive      m_mutex;                    /**< Mutex to protect against concurrent access. */
    SimpleTimer         m_requestTimer;             /**< Timer is used for cyclic weather http request. */

    /**
     * Defines the message types, which are necessary for HTTP client/server handling.
     */
    enum MsgType
    {
        MSG_TYPE_INVALID = 0,   /**< Invalid message type. */
        MSG_TYPE_RSP            /**< A response, caused by a previous request. */
    };

    /**
     * A message for HTTP client/server handling.
     */
    struct Msg
    {
        MsgType                 type;   /**< Message type */
        DynamicJsonDocument*    rsp;    /**< Response, only valid if message type is a response. */

        /**
         * Constructs a message.
         */
        Msg() :
            type(MSG_TYPE_INVALID),
            rsp(nullptr)
        {
        }
    }; 

    /**
     * Task proxy used to decouple server responses, which happen in a different task context.
     */
    TaskProxy<Msg, 2U, 0U> m_taskProxy;

    /**
     * Request new data.
     *
     * @return If successful it will return true otherwise false.
     */
    bool startHttpRequest(void);

    /**
     * Register callback function on response reception.
     */
    void initHttpClient(void);

    /**
     * Handle asynchronous web response from the server.
     * This will be called in LwIP context! Don't modify any member here directly!
     * 
     * @param[in] jsonDoc   Web response as JSON document
     */
    void handleAsyncWebResponse(const HttpResponse& rsp);

    /**
     * Handle a web response from the server.
     * 
     * @param[in] jsonDoc   Web response as JSON document
     */
    void handleWebResponse(DynamicJsonDocument& jsonDoc);

    /**
     * Clear the task proxy queue.
     */
    void clearQueue();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* BTCQUOTEPLUGIN_H */

/** @} */

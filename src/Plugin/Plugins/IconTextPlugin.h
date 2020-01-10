/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Icon and text plugin
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __ICONTEXTPLUGIN_H__
#define __ICONTEXTPLUGIN_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "Plugin.hpp"

#include <Canvas.h>
#include <BitmapWidget.h>
#include <TextWidget.h>
#include <SPIFFS.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Shows a icon (bitmap) on the left side in 8 x 8 and text on the right side.
 * If the text is too long for the display width, it automatically scrolls.
 * 
 * Change icon or text via REST API:
 * Icon: POST \c "<base-uri>/bitmap" with multipart/form-data file upload.
 * Text: POST \c "<base-uri>/text?show=<text>"
 */
class IconTextPlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     * 
     * @param[in] name  Plugin name
     */
    IconTextPlugin(const String& name) :
        Plugin(name),
        m_textCanvas(nullptr),
        m_iconCanvas(nullptr),
        m_bitmapWidget(),
        m_textWidget(),
        m_urlIcon(),
        m_urlText(),
        m_callbackWebHandlerIcon(nullptr),
        m_callbackWebHandlerText(nullptr),
        m_fd(),
        m_isUploadError(false)
    {
    }

    /**
     * Destroys the plugin.
     */
    ~IconTextPlugin()
    {
        uint16_t        bitmapWidth     = 0U;
        uint16_t        bitmapHeight    = 0U;
        const uint16_t* bitmapBuffer    = m_bitmapWidget.get(bitmapWidth, bitmapHeight);

        if (nullptr != m_iconCanvas)
        {
            delete m_iconCanvas;
            m_iconCanvas = nullptr;
        }

        if (nullptr != m_textCanvas)
        {
            delete m_textCanvas;
            m_textCanvas = nullptr;
        }

        if (nullptr != bitmapBuffer)
        {
            delete[] bitmapBuffer;
            bitmapBuffer = nullptr;
        }
    }

    /**
     * Plugin creation method, used to register on the plugin manager.
     * 
     * @param[in] name  Plugin name
     * 
     * @return If successful, it will return the pointer to the plugin instance, otherwise nullptr.
     */
    static Plugin* create(const String& name)
    {
        return new IconTextPlugin(name);
    }
    
    /**
     * This method will be called in case the plugin is set active, which means
     * it will be shown on the display in the next step.
     * 
     * @param[in] gfx   Display graphics interface
     */
    void active(IGfx& gfx) override;

    /**
     * This method will be called in case the plugin is set inactive, which means
     * it won't be shown on the display anymore.
     */
    void inactive() override;

    /**
     * Register web interface, e.g. REST API functionality.
     * 
     * @param[in] srv       Webserver
     * @param[in] baseUri   Base URI, use this and append plugin specific part.
     */
    void registerWebInterface(AsyncWebServer& srv, const String& baseUri) override;

    /**
     * Unregister web interface.
     * 
     * @param[in] srv   Webserver
     */
    void unregisterWebInterface(AsyncWebServer& srv) override;

    /**
     * Update the display.
     * The scheduler will call this method periodically.
     * 
     * @param[in] gfx   Display graphics interface
     */
    void update(IGfx& gfx);

    /**
     * Set text, which may contain format tags.
     * 
     * @param[in] formatText    Text, which may contain format tags.
     */
    void setText(const String& formatText);

    /**
     * Set bitmap in raw RGB565 format.
     * 
     * @param[in] bitmap    Bitmap buffer
     * @param[in] width     Bitmap width in pixel
     * @param[in] height    Bitmap height in pixel
     */
    void setBitmap(const uint16_t* bitmap, uint16_t width, uint16_t height);

    /**
     * Load bitmap from filesystem.
     * 
     * @param[in] filename  Bitmap filename
     * 
     * @return If successul, it will return true otherwise false.
     */
    bool loadBitmap(const String& filename);
    
private:

    /**
     * Icon width in pixels.
     */
    static const int16_t ICON_WIDTH     = 8;

    /**
     * Icon height in pixels.
     */
    static const int16_t ICON_HEIGHT    = 8;

    /**
     * Image upload path.
     */
    static const char*  UPLOAD_PATH;

    Canvas*                     m_textCanvas;               /**< Canvas used for the text widget. */
    Canvas*                     m_iconCanvas;               /**< Canvas used for the bitmap widget. */
    BitmapWidget                m_bitmapWidget;             /**< Bitmap widget, used to show the icon. */
    TextWidget                  m_textWidget;               /**< Text widget, used for showing the text. */
    String                      m_urlIcon;                  /**< REST API URL for updating the icon */
    String                      m_urlText;                  /**< REST API URL for updating the text */
    AsyncCallbackWebHandler*    m_callbackWebHandlerIcon;   /**< Callback web handler for updating the icon */
    AsyncCallbackWebHandler*    m_callbackWebHandlerText;   /**< Callback web handler for updating the text */
    File                        m_fd;                       /**< File descriptor, used for bitmap file upload. */
    bool                        m_isUploadError;            /**< Flag to signal a upload error. */

    /**
     * Instance specific web request handler, called by the static web request
     * handler. It will really handle the request.
     * 
     * @param[in] request   Web request
     */
    void webReqHandlerText(AsyncWebServerRequest *request);

    /**
     * Instance specific web request handler, called by the static web request
     * handler. It will really handle the request.
     * 
     * @param[in] request   Web request
     */
    void webReqHandlerIcon(AsyncWebServerRequest *request);

    /**
     * File upload handler.
     * 
     * @param[in] request   HTTP request.
     * @param[in] filename  Name of the uploaded file.
     * @param[in] index     Current file offset.
     * @param[in] data      Next data part of file, starting at offset.
     * @param[in] len       Data part size in byte.
     * @param[in] final     Is final packet or not.
     */
    void iconUploadHandler(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);

    /**
     * Get image filename with path.
     * 
     * @return Image filename with path.
     */
    String getFileName(void);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __ICONTEXTPLUGIN_H__ */

/** @} */
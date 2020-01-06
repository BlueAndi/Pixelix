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
 * @brief  Icon, text and lamp plugin
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __ICONTEXTLAMPPLUGIN_H__
#define __ICONTEXTLAMPPLUGIN_H__

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
#include <LampWidget.h>
#include <LinkedList.hpp>
#include <SPIFFS.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Shows a icon (bitmap) on the left side in 8 x 8, text on the right side and
 * under the text a bar with lamps.
 * If the text is too long for the display width, it automatically scrolls.
 * 
 * Change icon, text or lamps via REST API:
 * Icon: POST \c "<base-uri>/bitmap" with multipart/form-data file upload.
 * Text: POST \c "<base-uri>/text?show=<text>"
 * Lamp: POST \c "<base-uri>/lamp/<lamp-id>?set=<on/off>"
 */
class IconTextLampPlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     */
    IconTextLampPlugin() :
        Plugin(),
        m_iconCanvas(NULL),
        m_textCanvas(NULL),
        m_lampCanvas(NULL),
        m_bitmapWidget(),
        m_textWidget(),
        m_lampWidgets(),
        m_urlIcon(),
        m_urlText(),
        m_urlLamp(),
        m_callbackWebHandlerIcon(NULL),
        m_callbackWebHandlerText(NULL),
        m_callbackWebHandlerLamp(NULL),
        m_fd(),
        m_isUploadError(false)
    {
    }

    /**
     * Destroys the plugin.
     */
    ~IconTextLampPlugin()
    {
        uint16_t        bitmapWidth     = 0u;
        uint16_t        bitmapHeight    = 0u;
        const uint16_t* bitmapBuffer    = m_bitmapWidget.get(bitmapWidth, bitmapHeight);

        if (NULL != m_iconCanvas)
        {
            delete m_iconCanvas;
            m_iconCanvas = NULL;
        }

        if (NULL != m_textCanvas)
        {
            delete m_textCanvas;
            m_textCanvas = NULL;
        }

        if (NULL != m_lampCanvas)
        {
            delete m_lampCanvas;
            m_lampCanvas = NULL;
        }

        if (NULL != bitmapBuffer)
        {
            delete[] bitmapBuffer;
            bitmapBuffer = NULL;
        }
    }

    /**
     * Get the plugin name.
     *
     * @return Name of the plugin.
     */
    const char* getName(void) const
    {
        return "IconTextLampPlugin";
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
    void inactive(void) override;

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
     * Set lamp state.
     * 
     * @param[in] lampId    Lamp id
     * @param[in] state     Lamp state (true = on / false = off)
     */
    void setLamp(uint8_t lampId, bool state);

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
     * Max. number of lamps.
     */
    static const uint8_t MAX_LAMPS      = 4u;

    /**
     * Image upload path.
     */
    static const char*  UPLOAD_PATH;

    Canvas*                     m_iconCanvas;               /**< Canvas used for the bitmap widget. */
    Canvas*                     m_textCanvas;               /**< Canvas used for the text widget. */
    Canvas*                     m_lampCanvas;               /**< Canvas used for the lamp widget. */
    BitmapWidget                m_bitmapWidget;             /**< Bitmap widget, used to show the icon. */
    TextWidget                  m_textWidget;               /**< Text widget, used for showing the text. */
    LampWidget                  m_lampWidgets[MAX_LAMPS];   /**< Lamp widgets, used to signal different things. */
    String                      m_urlIcon;                  /**< REST API URL for updating the icon */
    String                      m_urlText;                  /**< REST API URL for updating the text */
    String                      m_urlLamp;                  /**< REST API URL for updating the lamps */
    AsyncCallbackWebHandler*    m_callbackWebHandlerIcon;   /**< Callback web handler for updating the icon */
    AsyncCallbackWebHandler*    m_callbackWebHandlerText;   /**< Callback web handler for updating the text */
    AsyncCallbackWebHandler*    m_callbackWebHandlerLamp;   /**< Callback web handler for updating the lamps */
    File                        m_fd;                       /**< File descriptor, used for bitmap file upload. */
    bool                        m_isUploadError;            /**< Flag to signal a upload error. */

    /** List of all instances and used to find the web request related instance later. */
    static DLinkedList<IconTextLampPlugin*> m_instances;

    /**
     * Static web request handler, used to register by the webserver.
     * It will find the request related instance and call the specific
     * request handler.
     * 
     * @param[in] request   Web request
     */
    static void staticWebReqHandler(AsyncWebServerRequest *request);

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
    static void staticUploadHandler(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);

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
     * Instance specific web request handler, called by the static web request
     * handler. It will really handle the request.
     * 
     * @param[in] request   Web request
     */
    void webReqHandlerLamp(AsyncWebServerRequest *request);

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

#endif  /* __ICONTEXTLAMPPLUGIN_H__ */

/** @} */
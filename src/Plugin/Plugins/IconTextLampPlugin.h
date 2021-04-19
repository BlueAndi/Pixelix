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

#include <FS.h>
#include <Canvas.h>
#include <BitmapWidget.h>
#include <TextWidget.h>
#include <LampWidget.h>

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
 */
class IconTextLampPlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    IconTextLampPlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_iconCanvas(nullptr),
        m_textCanvas(nullptr),
        m_lampCanvas(nullptr),
        m_bitmapWidget(),
        m_textWidget(),
        m_lampWidgets(),
        m_xMutex(nullptr)
    {
        m_xMutex = xSemaphoreCreateMutex();
    }

    /**
     * Destroys the plugin.
     */
    ~IconTextLampPlugin()
    {
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

        if (nullptr != m_lampCanvas)
        {
            delete m_lampCanvas;
            m_lampCanvas = nullptr;
        }

        if (nullptr != m_xMutex)
        {
            vSemaphoreDelete(m_xMutex);
            m_xMutex = nullptr;
        }
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
        return new IconTextLampPlugin(name, uid);
    }

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
    void getTopics(JsonArray& topics) const final;

    /**
     * Get a topic data.
     * Note, currently only JSON format is supported.
     * 
     * @param[in]   topic   The topic which data shall be retrieved.
     * @param[out]  value   The topic value in JSON format.
     * 
     * @return If successful it will return true otherwise false.
     */
    bool getTopic(const String& topic, JsonObject& value) const final;

    /**
     * Set a topic data.
     * Note, currently only JSON format is supported.
     * 
     * @param[in]   topic   The topic which data shall be retrieved.
     * @param[in]   value   The topic value in JSON format.
     * 
     * @return If successful it will return true otherwise false.
     */
    bool setTopic(const String& topic, const JsonObject& value) final;

    /**
     * Is a upload request accepted or rejected?
     * 
     * @param[in] topic         The topic which the upload belongs to.
     * @param[in] srcFilename   Name of the file, which will be uploaded if accepted.
     * @param[in] dstFilename   The destination filename, after storing the uploaded file.
     * 
     * @return If accepted it will return true otherwise false.
     */
    bool isUploadAccepted(const String& topic, const String& srcFilename, String& dstFilename) final;

    /**
     * This method will be called in case the plugin is set active, which means
     * it will be shown on the display in the next step.
     *
     * @param[in] gfx   Display graphics interface
     */
    void active(IGfx& gfx) final;

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
    void update(IGfx& gfx) final;

    /**
     * Get text.
     * 
     * @return Formatted text
     */
    String getText() const;

    /**
     * Set text, which may contain format tags.
     *
     * @param[in] formatText    Text, which may contain format tags.
     */
    void setText(const String& formatText);

    /**
     * Set bitmap in raw RGB888 format.
     *
     * @param[in] bitmap    Bitmap buffer
     * @param[in] width     Bitmap width in pixel
     * @param[in] height    Bitmap height in pixel
     */
    void setBitmap(const Color* bitmap, uint16_t width, uint16_t height);

    /**
     * Load bitmap from filesystem.
     *
     * @param[in] filename  Bitmap filename
     *
     * @return If successul, it will return true otherwise false.
     */
    bool loadBitmap(const String& filename);

    /**
     * Get lamp state (true = on / false = off).
     * 
     * @param[in] lampId    Lamp id
     * 
     * @return Lamp state
     */
    bool getLamp(uint8_t lampId) const;

    /**
     * Set lamp state.
     *
     * @param[in] lampId    Lamp id
     * @param[in] state     Lamp state (true = on / false = off)
     */
    void setLamp(uint8_t lampId, bool state);

private:

    /**
     * Plugin topic, used for parameter exchange.
     */
    static const char*  TOPIC_TEXT;

    /**
     * Plugin topic, used for parameter exchange.
     */
    static const char*  TOPIC_LAMPS;

    /**
     * Plugin topic, used for parameter exchange.
     */
    static const char*  TOPIC_LAMP;

    /**
     * Plugin topic, used for parameter exchange.
     */
    static const char*  TOPIC_ICON;

    /**
     * Icon width in pixels.
     */
    static const uint16_t   ICON_WIDTH  = 8U;

    /**
     * Icon height in pixels.
     */
    static const uint16_t   ICON_HEIGHT = 8U;

    /**
     * Max. number of lamps.
     */
    static const uint8_t    MAX_LAMPS   = 4U;

    Canvas*                     m_iconCanvas;               /**< Canvas used for the bitmap widget. */
    Canvas*                     m_textCanvas;               /**< Canvas used for the text widget. */
    Canvas*                     m_lampCanvas;               /**< Canvas used for the lamp widget. */
    BitmapWidget                m_bitmapWidget;             /**< Bitmap widget, used to show the icon. */
    TextWidget                  m_textWidget;               /**< Text widget, used for showing the text. */
    LampWidget                  m_lampWidgets[MAX_LAMPS];   /**< Lamp widgets, used to signal different things. */
    SemaphoreHandle_t           m_xMutex;                   /**< Mutex to protect against concurrent access. */

    /**
     * Get image filename with path.
     *
     * @return Image filename with path.
     */
    String getFileName(void);

    /**
     * Protect against concurrent access.
     */
    void lock(void) const;

    /**
     * Unprotect against concurrent access.
     */
    void unlock(void) const;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __ICONTEXTLAMPPLUGIN_H__ */

/** @} */
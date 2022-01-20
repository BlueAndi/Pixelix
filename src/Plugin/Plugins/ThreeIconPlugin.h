/* MIT License
 *
 * Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>
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
 * @brief Three icon plugin
 * @author Yann Le Glaz <yann_le@web.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __ThreeIconPlugin_H__
#define __ThreeIconPlugin_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "Plugin.hpp"

#include <FS.h>
#include <WidgetGroup.h>
#include <BitmapWidget.h>
#include <TextWidget.h>
#include <Mutex.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Shows three separate icons (bitmaps).
 */
class ThreeIconPlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name.
     * @param[in] uid   Unique id.
     */
    ThreeIconPlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_threeIconCanvas(),
        m_bitmapWidget(),
        m_isUploadError(false),
        m_mutex()
    {
        (void)m_mutex.create();
    }

    /**
     * Destroys the plugin.
     */
    ~ThreeIconPlugin()
    {
        m_mutex.destroy();
    }

    /**
     * Plugin creation method, used to register on the plugin manager.
     *
     * @param[in] name  Plugin name.
     * @param[in] uid   Unique id.
     *
     * @return If successful, it will return the pointer to the plugin instance, otherwise nullptr.
     */
    static IPluginMaintenance* create(const String& name, uint16_t uid)
    {
        return new ThreeIconPlugin(name, uid);
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
     * @param[out] topics   Topis in JSON format.
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
     * Start the plugin.
     * Overwrite it if your plugin needs to know that it was installed.
     * 
     * @param[in] width     Display width in pixel.
     * @param[in] height    Display height in pixel.
     */
    void start(uint16_t width, uint16_t height) final;
    
    /**
     * Stop the plugin.
     */
    void stop() final;

    /**
     * Update the display.
     * The scheduler will call this method periodically.
     *
     * @param[in] gfx   Display graphics interface.
     */
    void update(YAGfx& gfx) final;

    /**
     * Load bitmap from filesystem.
     *
     * @param[in] filename  Bitmap filename.
     * @param[in] iconId    The icon id.
     *
     * @return If successul, it will return true otherwise false.
     */
    bool loadBitmap(const String& filename, uint8_t iconId);

private:

    /**
     * Plugin topic, used for parameter exchange.
     */
    static const char*      TOPIC_ICON;

    /**
     * Max. number of icons.
     */
    static const uint8_t    MAX_ICONS   = 3U;

    /**
     * Icon width in pixels.
     */
    static const uint16_t   ICON_WIDTH    = 8U;

    /**
     * Icon height in pixels.
     */
    static const uint16_t   ICON_HEIGHT   = 8U;
    
    /**
     * Filename extension of bitmap image file.
     */
    static const char*      FILE_EXT_BITMAP;

    /**
     * Filename extension of sprite sheet parameter file.
     */
    static const char*      FILE_EXT_SPRITE_SHEET;


    WidgetGroup             m_threeIconCanvas;          /**< Canvas used for the bitmap widget. */
    BitmapWidget            m_bitmapWidget[MAX_ICONS];  /**< Bitmap widget, used to show the icon. */
    bool                    m_isUploadError;            /**< Flag to signal a upload error. */
    mutable MutexRecursive  m_mutex;                    /**< Mutex to protect against concurrent access. */

    /**
     * Get image filename with path.
     * 
     * @param[in] iconId    The icon id.
     * @param[in] ext       File extension.
     * 
     * @return Image filename with path.
     */
    String getFileName(uint8_t iconId, const String& ext);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __ThreeIconPlugin_H__ */

/** @} */
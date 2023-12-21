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
 * @brief  Three icon plugin
 * @author Yann Le Glaz <yann_le@web.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef ThreeIconPlugin_H
#define ThreeIconPlugin_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "Plugin.hpp"

#include <WidgetGroup.h>
#include <BitmapWidget.h>
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
        m_bitmapWidgets(),
        m_iconPaths(),
        m_spriteSheetPaths(),
        m_isUploadError(false),
        m_mutex(),
        m_hasTopicChanged()
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
        return new(std::nothrow)ThreeIconPlugin(name, uid);
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
    bool setTopic(const String& topic, const JsonObjectConst& value) final;
    
    /**
     * Is the topic content changed since last time?
     * Every readable volatile topic shall support this. Otherwise the topic
     * handlers might not be able to provide updated information.
     * 
     * @param[in] topic The topic which to check.
     * 
     * @return If the topic content changed since last time, it will return true otherwise false.
     */
    bool hasTopicChanged(const String& topic) final;
    
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
     * Load bitmap image from filesystem. If a sprite sheet is available, the
     * bitmap will be automatically used as texture for animation.
     *
     * @param[in] iconId    The icon id.
     * @param[in] filename  Bitmap image filename.
     *
     * @return If successul, it will return true otherwise false.
     */
    bool loadBitmap(uint8_t iconId, const String& filename);

    /**
     * Load sprite sheet from filesystem. If a bitmap is available, it will
     * be automatically used as texture for animation.
     *
     * @param[in] iconId    The icon id.
     * @param[in] filename  Sprite sheet filename.
     *
     * @return If successul, it will return true otherwise false.
     */
    bool loadSpriteSheet(uint8_t iconId, const String& filename);

    /**
     * Get the state of the FORWARD control flag of an icon.
     *
     * @param[in] iconId    The icon Id.
     *
     * @return The state of the isForward animation control flag.
     */  
    bool getIsForward(uint8_t iconId) const;

    /**
     * Set the state of the FORWARD control flag of an icon.
     *
     * @param[in] iconId    The icon Id.
     * @param[in] state     The state to be set.
     */  
    void setIsForward(uint8_t iconId, bool state);

    /**
     * Get the state of the REPEAT control flag of an icon.
     *
     * @param[in] iconId    The icon Id.
     * 
     * @return The state of the isRepeat animation control flag.
     */  
    bool getIsRepeat(uint8_t iconId) const;

    /**
     * Set the state of the REPEAT control flag of an icon.
     *
     * @param[in] iconId    The icon Id.
     * @param[in] state     The state to be set.
     */  
    void setIsRepeat(uint8_t iconId, bool state);

    /**
     * Clear icon bitmap by icon id.
     * 
     * @param iconId The icon id. 
     */
    void clearBitmap(uint8_t iconId);

    /**
     * Clear sprite sheet by icon id.
     * 
     * @param[in] iconId The icon id. 
     */
    void clearSpriteSheet(uint8_t iconId);

    /**
     * Get icon file path by icon id.
     * 
     * @param[in]   iconId      The icon id. 
     * @param[out]  fullPath    Path to icon file.
     */
    void getIconFilePath(uint8_t iconId, String& fullPath) const;

    /**
     * Get sprite sheet file path by icon id.
     * 
     * @param[in]   iconId      The icon id. 
     * @param[out]  fullPath    Path to sprite sheet file.
     */
    void getSpriteSheetFilePath(uint8_t iconId, String& fullPath) const;

private:

    /**
     * Plugin topic, used for bitmap/spritesheet upload and control.
     */
    static const char*      TOPIC_BITMAP;

    /**
     * Plugin topic, used for parameter exchange.
     */
    static const char*      TOPIC_SPRITESHEET;

   /**
     * Plugin topic, used for only for animation control in case a spritesheet
     * with texture is loaded.
     */
    static const char*      TOPIC_ANIMATION;

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


    WidgetGroup             m_threeIconCanvas;              /**< Canvas used for the bitmap widget. */
    BitmapWidget            m_bitmapWidgets[MAX_ICONS];     /**< Bitmap widgets, used to show the icon. */
    String                  m_iconPaths[MAX_ICONS];         /**< Full path to icons. */
    String                  m_spriteSheetPaths[MAX_ICONS];  /**< Full path to spritesheets. */
    bool                    m_isUploadError;                /**< Flag to signal a upload error. */
    mutable MutexRecursive  m_mutex;                        /**< Mutex to protect against concurrent access. */
    bool                    m_hasTopicChanged[MAX_ICONS];  /**< Has the topic content changed? */

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

#endif  /* ThreeIconPlugin_H */

/** @} */
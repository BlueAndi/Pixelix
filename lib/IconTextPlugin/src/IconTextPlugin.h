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
 * @brief  Icon and text plugin
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef ICONTEXTPLUGIN_H
#define ICONTEXTPLUGIN_H

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
 * Shows an icon (bitmap) on the left side in 8 x 8 and text on the right side.
 * If the text is too long for the display width, it automatically scrolls.
 */
class IconTextPlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    IconTextPlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_fontType(Fonts::FONT_TYPE_DEFAULT),
        m_textCanvas(),
        m_iconCanvas(),
        m_bitmapWidget(),
        m_textWidget(),
        m_iconPath(),
        m_spriteSheetPath(),
        m_isUploadError(false),
        m_mutex(),
        m_hasTopicChanged(false)
    {
        (void)m_mutex.create();
    }

    /**
     * Destroys the plugin.
     */
    ~IconTextPlugin()
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
        return new(std::nothrow)IconTextPlugin(name, uid);
    }

    /**
     * Is plugin enabled or not?
     *
     * @return If plugin is enabled, it will return true otherwise false.
     */
    bool isEnabled() const final;

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
     * Stop the plugin. This is called only once during plugin lifetime.
     */
    void stop() final;

    /**
     * Update the display.
     * The scheduler will call this method periodically.
     *
     * @param[in] gfx   Display graphics interface
     */
    void update(YAGfx& gfx) final;

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
     * Load bitmap image from filesystem. If a sprite sheet is available, the
     * bitmap will be automatically used as texture for animation.
     *
     * @param[in] filename  Bitmap image filename
     *
     * @return If successul, it will return true otherwise false.
     */
    bool loadBitmap(const String& filename);

    /**
     * Load sprite sheet from filesystem. If a bitmap is available, it will
     * be automatically used as texture for animation.
     *
     * @param[in] filename  Sprite sheet filename
     *
     * @return If successul, it will return true otherwise false.
     */
    bool loadSpriteSheet(const String& filename);

    /**
     * Clear bitmap icon.
     */
    void clearBitmap();

    /**
     * Clear sprite sheet.
     */
    void clearSpriteSheet();

    /**
     * Get icon file path.
     * 
     * @param[out] Path to icon file.
     */
    void getIconFilePath(String& fullPath) const;

    /**
     * Get sprite sheet file path.
     * 
     * @param[out] Path to sprite sheet file.
     */
    void getSpriteSheetFilePath(String& fullPath) const;

private:

    /**
     * Plugin topic, used for parameter exchange.
     */
    static const char*      TOPIC_TEXT;

    /**
     * Plugin topic, used for parameter exchange.
     */
    static const char*      TOPIC_ICON;

    /**
     * Plugin topic, used for parameter exchange.
     */
    static const char*      TOPIC_SPRITESHEET;

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

    Fonts::FontType         m_fontType;         /**< Font type which shall be used if there is no conflict with the layout. */
    WidgetGroup             m_textCanvas;       /**< Canvas used for the text widget. */
    WidgetGroup             m_iconCanvas;       /**< Canvas used for the bitmap widget. */
    BitmapWidget            m_bitmapWidget;     /**< Bitmap widget, used to show the icon. */
    TextWidget              m_textWidget;       /**< Text widget, used for showing the text. */
    String                  m_iconPath;         /**< Full path to icon. */
    String                  m_spriteSheetPath;  /**< Full path to spritesheet. */
    bool                    m_isUploadError;    /**< Flag to signal a upload error. */
    mutable MutexRecursive  m_mutex;            /**< Mutex to protect against concurrent access. */
    bool                    m_hasTopicChanged;  /**< Has the topic text content changed? */

    /**
     * Get filename with path.
     * 
     * @param[in] ext   File extension
     *
     * @return Filename with path.
     */
    String getFileName(const String& ext);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* ICONTEXTPLUGIN_H */

/** @} */
/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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

#ifndef ICONTEXTLAMPPLUGIN_H
#define ICONTEXTLAMPPLUGIN_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "./internal/View.h"

#include <stdint.h>
#include <PluginWithConfig.hpp>
#include <Mutex.hpp>
#include <FileSystem.h>
#include <FileMgrService.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Shows an icon on the left side in 8 x 8, text on the right side and
 * under the text a bar with lamps.
 * If the text is too long for the display width, it automatically scrolls.
 */
class IconTextLampPlugin : public PluginWithConfig
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name (must exist over lifetime)
     * @param[in] uid   Unique id
     */
    IconTextLampPlugin(const char* name, uint16_t uid) :
        PluginWithConfig(name, uid, FILESYSTEM),
        m_view(),
        m_iconFileId(FileMgrService::FILE_ID_INVALID),
        m_formatTextStored(),
        m_iconFileIdStored(FileMgrService::FILE_ID_INVALID),
        m_mutex(),
        m_hasTopicTextChanged(false),
        m_hasTopicLampsChanged(false),
        m_hasTopicLampChanged{false, false, false, false}
    {
        (void)m_mutex.create();
    }

    /**
     * Destroys the plugin.
     */
    ~IconTextLampPlugin()
    {
        m_mutex.destroy();
    }

    /**
     * Plugin creation method, used to register on the plugin manager.
     *
     * @param[in] name  Plugin name (must exist over lifetime)
     * @param[in] uid   Unique id
     *
     * @return If successful, it will return the pointer to the plugin instance, otherwise nullptr.
     */
    static IPluginMaintenance* create(const char* name, uint16_t uid)
    {
        return new(std::nothrow)IconTextLampPlugin(name, uid);
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
     * @param[in] storeFlag     Store the text persistent or not.
     */
    void setText(const String& formatText, bool storeFlag);

    /**
     * Load icon by file id.
     *
     * @param[in] fileId    File id
     * @param[in] storeFlag Store the text persistent or not.
     *
     * @return If successul, it will return true otherwise false.
     */
    bool loadIcon(FileMgrService::FileId fileId, bool storeFlag);

    /**
     * Clear icon from view and remove it from filesytem.
     * 
     * @param[in] storeFlag Store the text persistent or not.
     */
    void clearIcon(bool storeFlag);


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
    static const char*      TOPIC_TEXT;

    /**
     * Plugin topic, used for parameter exchange.
     */
    static const char*      TOPIC_LAMPS;

    /**
     * Plugin topic, used for parameter exchange.
     */
    static const char*      TOPIC_LAMP;


    _IconTextLampPlugin::View   m_view;                 /**< View with all widgets. */
    FileMgrService::FileId      m_iconFileId;           /**< Icon file id, used to retrieve the full path to the icon from the file manager. */
    String                      m_formatTextStored;     /**< It contains the format text, which is persistent stored. */
    FileMgrService::FileId      m_iconFileIdStored;     /**< Icon file id, which is persistent stored. */
    mutable MutexRecursive      m_mutex;                /**< Mutex to protect against concurrent access. */
    bool                        m_hasTopicTextChanged;  /**< Has the topic text content changed? Used to notify the TopicHandlerService about changes. */
    bool                        m_hasTopicLampsChanged; /**< Has the topic lamps content changed? Used to notify the TopicHandlerService about changes. */
    bool                        m_hasTopicLampChanged[_IconTextLampPlugin::View::MAX_LAMPS];  /**< Has the topic lamp content changed? Used to notify the TopicHandlerService about changes. */

    /**
     * Get actual configuration in JSON.
     * 
     * @param[out] cfg  Configuration
     */
    void getActualConfiguration(JsonObject& cfg) const;

    /**
     * Set actual configuration in JSON.
     * It will not be stored to configuration file.
     * 
     * @param[in] cfg   Configuration
     * 
     * @return If successful set, it will return true otherwise false.
     */
    bool setActualConfiguration(const JsonObjectConst& jsonCfg);

    /**
     * Get persistent configuration in JSON.
     * 
     * @param[out] cfg  Configuration
     */
    void getConfiguration(JsonObject& jsonCfg) const final;

    /**
     * Set persistent configuration in JSON.
     * 
     * @param[in] cfg   Configuration
     * 
     * @return If successful set, it will return true otherwise false.
     */
    bool setConfiguration(const JsonObjectConst& jsonCfg) final;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* ICONTEXTLAMPPLUGIN_H */

/** @} */
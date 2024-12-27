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
 * @brief  Multiple icon plugin
 * @author Yann Le Glaz <yann_le@web.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef MULTI_ICON_PLUGIN_H
#define MULTI_ICON_PLUGIN_H

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
 * Shows multiple separate icons, depended on the available display size.
 */
class MultiIconPlugin : public PluginWithConfig
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name (must exist over lifetime).
     * @param[in] uid   Unique id.
     */
    MultiIconPlugin(const char* name, uint16_t uid) :
        PluginWithConfig(name, uid, FILESYSTEM),
        m_view(),
        m_slots(),
        m_mutex(),
        m_hasTopicSlotsChanged(false)
    {
        (void)m_mutex.create();
    }

    /**
     * Destroys the plugin.
     */
    ~MultiIconPlugin()
    {
        m_mutex.destroy();
    }

    /**
     * Plugin creation method, used to register on the plugin manager.
     *
     * @param[in] name  Plugin name (must exist over lifetime).
     * @param[in] uid   Unique id.
     *
     * @return If successful, it will return the pointer to the plugin instance, otherwise nullptr.
     */
    static IPluginMaintenance* create(const char* name, uint16_t uid)
    {
        return new(std::nothrow)MultiIconPlugin(name, uid);
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
     * Homeassistant MQTT discovery support can be added with the "ha" key.
     * {
     *     "topics": [{
     *         "name": "/text",
     *         "ha": {
     *             <everything here will be used for MQTT discovery>
     *         }
     *     }]
     * }
     * 
     * Additional information can be loaded from a file too. It will be appended
     * to the topic data (parallel to "name" and "access"). If a file is used,
     * any other key than "name" and "access" will be ignored.
     * {
     *     "topics": [{
     *         "name": "/text",
     *         "fileName": "haText.json"
     *    }]
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
     * Get the file id of the given slot.
     *
     * @param[in] iconId    The slot id.
     *
     * @return The icon file id.
     */  
    uint8_t getIconFileId(uint8_t slotId) const;

    /**
     * Load the icon in a slot by file id.
     *
     * @param[in] slotId    The slot id.
     * @param[in] fileId    The icon file id.
     * 
     * @return If successful it will return true otherwise false.
     */  
    bool loadIcon(uint8_t slotId, FileMgrService::FileId fileId);

    /**
     * Clear icon from view and remove it from filesytem.
     * 
     * @param[in] slotId    The slot id.
     */
    void clearIcon(uint8_t slotId);

private:

    /**
     * Plugin topic, used to control which icon is shown in which slot or
     * to change a icon in a slot.
     */
    static const char*          TOPIC_SLOT;

    /**
     * Plugin topic, used to get the number of slots and their configuration.
     * Can be used to set a complete configuration too.
     */
    static const char*          TOPIC_SLOTS;

    /**
     * The slot data required for management.
     */
    struct IconSlot
    {
        FileMgrService::FileId  fileId;         /**< File id of the icon. */
        bool                    hasSlotChanged; /**< Has slot changed since last time? */

        /**
         * Construct icon slot.
         */
        IconSlot() :
            fileId(FileMgrService::FILE_ID_INVALID),
            hasSlotChanged(false)
        {
        }

        /**
         * Destroy icon slot.
         */
        ~IconSlot()
        {
        }
    };

    _MultiIconPlugin::View  m_view;                                             /**< View with all widgets. */
    IconSlot                m_slots[_MultiIconPlugin::View::MAX_ICON_SLOTS];    /**< Icon slots. */
    mutable MutexRecursive  m_mutex;                                            /**< Mutex to protect against concurrent access. */
    bool                    m_hasTopicSlotsChanged;                             /**< Has the topic content changed? Used to notify the TopicHandlerService about changes. */

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

    /**
     * Get slot id from topic.
     * 
     * @param[out]  slotId  The retrieved slot id.
     * @param[in]   topic   The topic, which contains the slot id.
     * 
     * @return If successful, it will return true otherwise false.
     */
    bool getSlotIdFromTopic(uint8_t& slotId, const String& topic) const;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* MULTI_ICON_PLUGIN_H */

/** @} */
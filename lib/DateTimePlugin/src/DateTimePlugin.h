/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @brief  DateTime plugin
 * @author Yann Le Glaz <yann_le@web.de>
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef DATETIMEPLUGIN_H
#define DATETIMEPLUGIN_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "./internal/View.h"

#include <stdint.h>
#include <time.h>
#include <PluginWithConfig.hpp>
#include <Mutex.hpp>
#include <FileSystem.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Shows the current data and time (alternately) over the whole display.
 * It can be configured to show only the date or only the time as well.
 */
class DateTimePlugin : public PluginWithConfig
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name (must exist over lifetime)
     * @param[in] uid   Unique id
     */
    DateTimePlugin(const char* name, uint16_t uid) :
        PluginWithConfig(name, uid, FILESYSTEM),
        m_view(),
        m_mode(MODE_DATE_TIME),
        m_checkUpdateTimer(),
        m_durationCounter(0u),
        m_shownSecond(-1),
        m_shownDayOfTheYear(-1),
        m_timeFormat(TIME_FORMAT_DEFAULT),
        m_dateFormat(DATE_FORMAT_DEFAULT),
        m_timeZone(),
        m_slotInterf(nullptr),
        m_mutex(),
        m_hasTopicChanged(false)
    {
        (void)m_mutex.create();
    }

    /**
     * Destroys the plugin.
     */
    ~DateTimePlugin()
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
        return new(std::nothrow) DateTimePlugin(name, uid);
    }

    /**
     * Get font type.
     * 
     * @return The font type the plugin uses.
     */
    Fonts::FontType getFontType() const final
    {
        return m_view.getFontType();
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
        m_view.setFontType(fontType);
    }

    /**
     * Get plugin topics, which can be get/set via different communication
     * interfaces like REST, websocket, MQTT, etc.
     * 
     * Example:
     * <code>{.json}
     * {
     *     "topics": [
     *         "text"
     *     ]
     * }
     * </code>
     * 
     * By default a topic is readable and writeable.
     * This can be set explicit with the "access" key with the following possible
     * values:
     * - Only readable: "r"
     * - Only writeable: "w"
     * - Readable and writeable: "rw"
     * 
     * Example:
     * <code>{.json}
     * {
     *     "topics": [{
     *         "name": "text",
     *         "access": "r"
     *     }]
     * }
     * </code>
     * 
     * Home Assistant MQTT discovery support can be added with the "ha" JSON object inside
     * the "extra" JSON object. The Home Assistant extension supports only loading by file.
     * <code>{.json}
     * {
     *     "topics": [{
     *         "name": "text",
     *         "extra": {
     *             "ha": "myHomeAssistantConfig.json"
     *         }
     *     }]
     * }
     * </code>
     * 
     * Extra information can be loaded from a file too. This is useful for complex
     * configurations and to keep program memory usage low.
     * <code>{.json}
     * {
     *     "topics": [{
     *         "name": "text",
     *         "extra": "extra.json"
     *    }]
     * }
     * </code>
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
     * Set the slot interface, which the plugin can used to request information
     * from the slot, it is plugged in.
     *
     * @param[in] slotInterf    Slot interface
     */
    void setSlot(const ISlotPlugin* slotInterf) final;

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

    /**
     * The plugin provides several modes, which influences whats shown on the
     * display.
     */
    enum Mode
    {
        MODE_DATE_TIME = 0,  /**< Show date and time */
        MODE_DATE_ONLY,      /**< Show only the date */
        MODE_TIME_ONLY,      /**< Show only the time */
        MODE_MAX             /**< Number of configurations */
    };

    /**
     * Plugin topic, used to read/write the configuration.
     */
    static const char      TOPIC_CONFIG[];

    /** Time to check date update period in ms */
    static const uint32_t   CHECK_UPDATE_PERIOD     = SIMPLE_TIMER_SECONDS(1U);

    /** Divider to convert ms in s */
    static const uint32_t   MS_TO_SEC_DIVIDER       = 1000U;

    /** Default time format according to strftime(). */
    static const char       TIME_FORMAT_DEFAULT[];

    /** Default date format according to strftime(). */
    static const char       DATE_FORMAT_DEFAULT[];

    /**
     * If the slot duration is infinite (0s), the default duration of 30s shall be assumed as base
     * for toggling between time and date on the display.
     * 
     * The default duration is in ms.
     */
    static const uint32_t   DURATION_DEFAULT        = SIMPLE_TIMER_SECONDS(30U);

    _DateTimePlugin::View   m_view;                 /**< The layout with all used widgets. */
    Mode                    m_mode;                 /**< Display mode about what shall be shown. */
    SimpleTimer             m_checkUpdateTimer;     /**< Timer, used for cyclic check if date/time update is necessary. */
    uint8_t                 m_durationCounter;      /**< Variable to count the Plugin duration in CHECK_UPDATE_PERIOD ticks . */
    int                     m_shownSecond;          /**< Used to trigger a display update in case the time shall be shown. [0; 59] */
    int                     m_shownDayOfTheYear;    /**< Used to trigger a display update in case the date shall be shown. [0; 365] */
    String                  m_timeFormat;           /**< Time format according to strftime(). */
    String                  m_dateFormat;           /**< Date format according to strftime(). */
    String                  m_timeZone;             /**< Timezone of the time to show. If empty, the local time is used. */
    const ISlotPlugin*      m_slotInterf;           /**< Slot interface */
    mutable MutexRecursive  m_mutex;                /**< Mutex to protect against concurrent access. */
    bool                    m_hasTopicChanged;      /**< Has the topic content changed? */

    /**
     * Get configuration in JSON.
     * 
     * @param[out] cfg  Configuration
     */
    void getConfiguration(JsonObject& jsonCfg) const final;

    /**
     * Set configuration in JSON.
     * 
     * @param[in] cfg   Configuration
     * 
     * @return If successful set, it will return true otherwise false.
     */
    bool setConfiguration(const JsonObjectConst& jsonCfg) final;

    /**
     * Merge JSON configuration with local settings to create a complete set.
     *
     * The received configuration may not contain all single key/value pair.
     * Therefore create a complete internal configuration and overwrite it
     * with the received one.
     *  
     * @param[out] jsonMerged  The complete config set with merge content from jsonSource.
     * @param[in]  jsonSource  The recevied congi set, which may not cover all keys.
     * @return     true        Keys needed merging.
     * @return     false       Nothing needed merging.
     */
    bool mergeConfiguration(JsonObject& jsonMerged, const JsonObjectConst& jsonSource);

    /**
     * Get current date/time and update the text, which to be displayed.
     * The update takes only place, if the date changed.
     *
     * @param[in] force Force update independent of date.
     */
    void updateDateTime(bool force);

    /**
     * Get the current time as formatted string.
     * The format is equal to strftime(), please have a look there.
     * 
     * Use getTimeFormat() or getDateFormat() for the user configured format.
     * 
     * @param[out]  time            The formatted time string.
     * @param[in]   format          The format according to strftime().
     * @param[in]   currentTime     The current time (optional).
     * 
     * @return If successful, it will return true otherwise false.
     */
    bool getTimeAsString(String& time, const String& format, const tm *currentTime = nullptr);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* DATETIMEPLUGIN_H */

/** @} */
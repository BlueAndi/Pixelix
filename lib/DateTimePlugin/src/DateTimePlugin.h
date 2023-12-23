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
 * @brief  DateTime plugin
 * @author Yann Le Glaz <yann_le@web.de>
 *
 * @addtogroup plugin
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
#include <stdint.h>
#include <time.h>
#include "Plugin.hpp"

#include <LampWidget.h>
#include <TextWidget.h>
#include <WidgetGroup.h>
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
class DateTimePlugin : public Plugin, private PluginConfigFsHandler
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    DateTimePlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        PluginConfigFsHandler(uid, FILESYSTEM),
        m_textWidget("\\calignNo NTP"),
        m_textCanvas(),
        m_lampCanvas(),
        m_lampWidgets(),
        m_mode(MODE_DATE_TIME),
        m_checkUpdateTimer(),
        m_durationCounter(0u),
        m_shownSecond(-1),
        m_shownDayOfTheYear(-1),
        m_isUpdateAvailable(false),
        m_timeFormat(TIME_FORMAT_DEFAULT),
        m_dateFormat(DATE_FORMAT_DEFAULT),
        m_timeZone(),
        m_dayOnColor(DAY_ON_COLOR),
        m_dayOffColor(DAY_OFF_COLOR),
        m_slotInterf(nullptr),
        m_mutex(),
        m_cfgReloadTimer(),
        m_storeConfigReq(false),
        m_reloadConfigReq(false),
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
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     *
     * @return If successful, it will return the pointer to the plugin instance, otherwise nullptr.
     */
    static IPluginMaintenance* create(const String& name, uint16_t uid)
    {
        return new(std::nothrow) DateTimePlugin(name, uid);
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
    static const char*      TOPIC_CONFIG;

    /** Max. number of lamps. */
    static const uint8_t    MAX_LAMPS               = 7U;

    /** Time to check date update period in ms */
    static const uint32_t   CHECK_UPDATE_PERIOD     = SIMPLE_TIMER_SECONDS(1U);

    /** Divider to convert ms in s */
    static const uint32_t   MS_TO_SEC_DIVIDER       = 1000U;

    /** Default time format according to strftime(). */
    static const char*      TIME_FORMAT_DEFAULT;

    /** Default date format according to strftime(). */
    static const char*      DATE_FORMAT_DEFAULT;

    /** Color of the current day shown in the day of the week bar. */
    static const Color      DAY_ON_COLOR;

    /** Color of the other days (not the current one) shown in the day of the week bar. */
    static const Color      DAY_OFF_COLOR;

    /**
     * If the slot duration is infinite (0s), the default duration of 30s shall be assumed as base
     * for toggling between time and date on the display.
     * 
     * The default duration is in ms.
     */
    static const uint32_t   DURATION_DEFAULT        = SIMPLE_TIMER_SECONDS(30U);

    /**
     * The configuration in the persistent memory shall be cyclic loaded.
     * This mechanism ensure that manual changes in the file are considered.
     * This is the reload period in ms.
     */
    static const uint32_t   CFG_RELOAD_PERIOD       = SIMPLE_TIMER_SECONDS(30U);

    TextWidget              m_textWidget;               /**< Text widget, used for showing the text. */
    WidgetGroup             m_textCanvas;               /**< Canvas used for the text widget. */
    WidgetGroup             m_lampCanvas;               /**< Canvas used for the lamp widget. */
    LampWidget              m_lampWidgets[MAX_LAMPS];   /**< Lamp widgets, used to signal the day of week. */
    Mode                    m_mode;                     /**< Display mode about what shall be shown. */
    SimpleTimer             m_checkUpdateTimer;         /**< Timer, used for cyclic check if date/time update is necessary. */
    uint8_t                 m_durationCounter;          /**< Variable to count the Plugin duration in CHECK_UPDATE_PERIOD ticks . */
    int                     m_shownSecond;              /**< Used to trigger a display update in case the time shall be shown. [0; 59] */
    int                     m_shownDayOfTheYear;        /**< Used to trigger a display update in case the date shall be shown. [0; 365] */
    bool                    m_isUpdateAvailable;        /**< Flag to indicate an updated date value. */
    String                  m_timeFormat;               /**< Time format according to strftime(). */
    String                  m_dateFormat;               /**< Date format according to strftime(). */
    String                  m_timeZone;                 /**< Timezone of the time to show. If empty, the local time is used. */
    Color                   m_dayOnColor;               /**< Color of current day in the day of the week bar. */
    Color                   m_dayOffColor;              /**< Color of the other days in the day of the week bar. */
    const ISlotPlugin*      m_slotInterf;               /**< Slot interface */
    mutable MutexRecursive  m_mutex;                    /**< Mutex to protect against concurrent access. */
    SimpleTimer             m_cfgReloadTimer;           /**< Timer is used to cyclic reload the configuration from persistent memory. */
    bool                    m_storeConfigReq;           /**< Is requested to store the configuration in persistent memory? */
    bool                    m_reloadConfigReq;          /**< Is requested to reload the configuration from persistent memory? */
    bool                    m_hasTopicChanged;          /**< Has the topic content changed? */

    /**
     * Request to store configuration to persistent memory.
     */
    void requestStoreToPersistentMemory();

    /**
     * Get configuration in JSON.
     * 
     * @param[out] cfg  Configuration
     */
    void getConfiguration(JsonObject& cfg) const final;

    /**
     * Set configuration in JSON.
     * 
     * @param[in] cfg   Configuration
     * 
     * @return If successful set, it will return true otherwise false.
     */
    bool setConfiguration(JsonObjectConst& cfg) final;

    /**
     * Get current date/time and update the text, which to be displayed.
     * The update takes only place, if the date changed.
     *
     * @param[in] force Force update independent of date.
     */
    void updateDateTime(bool force);

    /**
     * Set weekday indicator depended on the given time info.
     *
     *
     * @param[in] timeInfo the current time info.
     */
    void setWeekdayIndicator(tm timeInfo);

    /**
     * Calculates the optimal layout for several elements, which shall be aligned.
     * 
     * @param[in]   width           Max. available width in pixel.
     * @param[in]   cnt             Number of elements in a row.
     * @param[in]   minDistance     The minimal distance in pixel between each element.
     * @param[in]   minBorder       The minimal border left and right of all elements.
     * @param[out]  elementWidth    The calculated optimal element width in pixel.
     * @param[out]  elementDistance The calculated optimal element distance in pixel.
     * 
     * @return If the calculation is successful, it will return true otherwise false.
     */
    bool calcLayout(uint16_t width, uint16_t cnt, uint16_t minDistance, uint16_t minBorder, uint16_t& elementWidth, uint16_t& elementDistance);

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

    /**
     * Convert color to HTML format.
     * 
     * @param[in] color Color
     * 
     * @return Color in HTML format
     */
    String colorToHtml(const Color& color) const;

    /**
     * Convert color from HTML format.
     * 
     * @param[in] htmlColor Color in HTML format
     * 
     * @return Color
     */
    Color colorFromHtml(const String& htmlColor) const;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* DATETIMEPLUGIN_H */

/** @} */
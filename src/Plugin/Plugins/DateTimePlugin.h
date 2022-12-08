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
 * @brief  DateTime plugin
 * @author Yann Le Glaz <yann_le@web.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __DATETIMEPLUGIN_H__
#define __DATETIMEPLUGIN_H__

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
class DateTimePlugin : public Plugin
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
        m_textWidget("\\calignNo NTP"),
        m_textCanvas(),
        m_lampCanvas(),
        m_lampWidgets(),
        m_cfg(CFG_DATE_TIME),
        m_checkUpdateTimer(),
        m_durationCounter(0u),
        m_shownSecond(-1),
        m_shownDayOfTheYear(-1),
        m_isUpdateAvailable(false),
        m_slotInterf(nullptr),
        m_mutex()

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
        return new DateTimePlugin(name, uid);
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

    /** Plugin configuration possibilities. */
    enum Cfg
    {
        CFG_DATE_TIME = 0,  /**< Show date and time */
        CFG_DATE_ONLY,      /**< Show only the date */
        CFG_TIME_ONLY,      /**< Show only the time */
        CFG_MAX             /**< Number of configurations */
    };

    /**
     * Get configuration about what shall be shown.
     * 
     * @return Configuration
     */
    Cfg getCfg() const
    {
        MutexGuard<MutexRecursive>  guard(m_mutex);
        Cfg                         cfg             = m_cfg;

        return cfg;
    }

    /**
     * Set configuration about what shall be shown.
     * 
     * @param[in] cfg   Configuration
     */
    void setCfg(Cfg cfg)
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        if ((cfg != m_cfg) &&
            (CFG_MAX > cfg))
        {
            m_cfg = cfg;

            (void)saveConfiguration();
        }
    }

private:

    /**
     * Plugin topic, used for configuration.
     */
    static const char*      TOPIC_CFG;

    /** Max. number of lamps. */
    static const uint8_t    MAX_LAMPS               = 7U;

    /** Time to check date update period in ms */
    static const uint32_t   CHECK_UPDATE_PERIOD     = 1000U;

    /** Divider to convert ms in s */
    static const uint32_t   MS_TO_SEC_DIVIDER       = 1000U;

    /**
     * If the slot duration is infinite (0s), the default duration of 30s shall be assumed as base
     * for toggling between time and date on the display.
     * 
     * The default duration is in ms.
     */
    static const uint32_t   DURATION_DEFAULT        = 30U * 1000U;

    TextWidget              m_textWidget;               /**< Text widget, used for showing the text. */
    WidgetGroup             m_textCanvas;               /**< Canvas used for the text widget. */
    WidgetGroup             m_lampCanvas;               /**< Canvas used for the lamp widget. */
    LampWidget              m_lampWidgets[MAX_LAMPS];   /**< Lamp widgets, used to signal the day of week. */
    Cfg                     m_cfg;                      /**< Configuration about what shall be shown. */
    SimpleTimer             m_checkUpdateTimer;         /**< Timer, used for cyclic check if date/time update is necessary. */
    uint8_t                 m_durationCounter;          /**< Variable to count the Plugin duration in CHECK_UPDATE_PERIOD ticks . */
    int                     m_shownSecond;              /**< Used to trigger a display update in case the time shall be shown. [0; 59] */

    int                     m_shownDayOfTheYear;        /**< Used to trigger a display update in case the date shall be shown. [0; 365] */
    bool                    m_isUpdateAvailable;        /**< Flag to indicate an updated date value. */
    const ISlotPlugin*      m_slotInterf;               /**< Slot interface */
    mutable MutexRecursive  m_mutex;                    /**< Mutex to protect against concurrent access. */

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
     * Saves current configuration to JSON file.
     */
    bool saveConfiguration() const;

    /**
     * Load configuration from JSON file.
     */
    bool loadConfiguration();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __DATETIMEPLUGIN_H__ */

/** @} */
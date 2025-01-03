
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
 * @brief  Countdown plugin
 * @author Yann Le Glaz <yann_le@web.de>
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef COUNTDOWNPLUGIN_H
#define COUNTDOWNPLUGIN_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "./internal/View.h"

#include <time.h>
#include <PluginWithConfig.hpp>
#include <stdint.h>
#include <SimpleTimer.hpp>
#include <Mutex.hpp>
#include <FileSystem.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Shows the remaining days until a configured target date.
 *
 * At the first installation a json document is generated to the /configuration/UUID.json
 * in the filesystem, where the target date has to be configured.
 *
 */
class CountdownPlugin : public PluginWithConfig
{
public:

    /** Date data. */
    struct DateDMY
    {
        uint8_t day;    /**< Day of month. */
        uint8_t month;  /**< Month of year. */
        uint16_t year;  /**< Year. */

        /** Initialize structure. */
        DateDMY() :
            day(0U),
            month(0U),
            year(0U)
        {
        }

        /** Destroy structure */
        ~DateDMY()
        {
        }

        /**
         * Initialize structure by assignment.
         *
         * @param[in] desc  Target day description, which to assign.
         */
        DateDMY(const DateDMY& desc) :
            day(desc.day),
            month(desc.month),
            year(desc.year)
        {
        }

        /**
         * Assign structure.
         *
         * @param[in] desc  Target day description, which to assign.
         * 
         * @return Date data
         */
        DateDMY& operator=(const DateDMY& desc)
        {
            if (this != &desc)
            {
                day     = desc.day;
                month   = desc.month;
                year    = desc.year;
            }

            return *this;
        }
    };

    /** The target date description. */
    struct TargetDayDescription
    {
        String plural;      /**< The description in plural form e.g. Days. */
        String singular;    /**< The description in singular form e.g. day */

        /** Initialize structure. */
        TargetDayDescription() :
            plural(),
            singular()
        {
        }

        /** Destroy structure. */
        ~TargetDayDescription()
        {
        }

        /**
         * Initialize structure by assignment.
         *
         * @param[in] desc  Target day description, which to assign.
         */
        TargetDayDescription(const TargetDayDescription& desc) :
            plural(desc.plural),
            singular(desc.singular)
        {
        }

        /**
         * Assign structure.
         *
         * @param[in] desc  Target day description, which to assign.
         * 
         * @return Target day description
         */
        TargetDayDescription& operator=(const TargetDayDescription& desc)
        {
            if (this != &desc)
            {
                plural      = desc.plural;
                singular    = desc.singular;
            }

            return *this;
        }
    };

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name (must exist over lifetime)
     * @param[in] uid   Unique id
     */
    CountdownPlugin(const char* name, uint16_t uid) :
        PluginWithConfig(name, uid, FILESYSTEM),
        m_view(),
        m_currentDate(),
        m_targetDate(),
        m_targetDateInformation(),
        m_remainingDays(""),
        m_mutex(),
        m_hasTopicChanged(false)
    {
        /* Example data, used to generate the very first configuration file. */
        m_targetDate.day                    = 1U;
        m_targetDate.month                  = 8U;
        m_targetDate.year                   = 2024U;
        m_targetDateInformation.plural      = "DAYS";
        m_targetDateInformation.singular    = "DAY";

        (void)m_mutex.create();
    }

    /**
     * Destroys the plugin.
     */
    ~CountdownPlugin()
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
        return new(std::nothrow) CountdownPlugin(name, uid);
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
     *         "/text"
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
     *         "name": "/text",
     *         "access": "r"
     *     }]
     * }
     * </code>
     * 
     * Homeassistant MQTT discovery support can be added with the "ha" JSON object inside
     * the "extra" JSON object.
     * <code>{.json}
     * {
     *     "topics": [{
     *         "name": "/text",
     *         "extra": {
     *             "ha": {
     *                 ... everything here will be used for MQTT discovery ...
     *             }
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
     *         "name": "/text",
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
     * Update the display.
     * The scheduler will call this method periodically.
     *
     * @param[in] gfx   Display graphics interface
     */
    void update(YAGfx& gfx) final;

private:

    /**
     * Plugin topic, used to read/write the configuration.
     */
    static const char*      TOPIC_CONFIG;

   /**
    * Offset to translate the month of the tm struct (time.h)
    * to a human readable value, since months since January are used (0-11).
    */
    static const int16_t    TM_OFFSET_MONTH = 1;

   /**
    * Offset to translate the year of the tm struct (time.h)
    * to a human readable value, since years since 1900 are used.
    */
    static const int16_t    TM_OFFSET_YEAR  = 1900;

    _CountdownPlugin::View  m_view;                     /**< View with all widgets. */
    DateDMY                 m_currentDate;              /**< Date structure to hold the current date. */
    DateDMY                 m_targetDate;               /**< Date structure to hold the target date from the configuration data. */
    TargetDayDescription    m_targetDateInformation;    /**< String used for configured additional target date information. */
    String                  m_remainingDays;            /**< String used for displaying the remaining days untril the target date. */
    mutable MutexRecursive  m_mutex;                    /**< Mutex to protect against concurrent access. */
    bool                    m_hasTopicChanged;          /**< Has the topic content changed? */

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
     * Calculates the remaining days between m_targetTime and m_currentTime in days and
     * update m_remainingDays.
     */
    void calculateRemainingDays(void);

    /**
     * Counts the number of leap years.
     *
     * @param[in] date  Date
     *
     * @return Number of leap years.
     */
    uint16_t countLeapYears(const DateDMY& date) const;

    /**
     * Convert a given date in days starting from year 0.
     *
     * @param[in] date  Date
     *
     * @return Number of days since 0.
     */
    uint32_t dateToDays(const DateDMY& date) const;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* COUNTDOWNPLUGIN_H */

/** @} */

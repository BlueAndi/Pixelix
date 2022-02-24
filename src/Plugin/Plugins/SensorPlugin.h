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
 * @brief  Sensor plugin
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __SENSOR_PLUGIN_H__
#define __SENSOR_PLUGIN_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "Plugin.hpp"

#include <TextWidget.h>
#include <ISensorChannel.hpp>
#include <SimpleTimer.hpp>
#include <Mutex.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The sensor plugin can show a provided value by any connected sensor.
 */
class SensorPlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    SensorPlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_textWidget(),
        m_mutex(),
        m_sensorIdx(0U),
        m_channelIdx(0U),
        m_sensorChannel(nullptr)
    {
        /* Move the text widget one line lower for better look. */
        m_textWidget.move(0, 1);

        (void)m_mutex.create();
    }

    /**
     * Destroys the plugin.
     */
    ~SensorPlugin()
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
        return new SensorPlugin(name, uid);
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
     * Start the plugin. This is called only once during plugin lifetime.
     * It can be used as deferred initialization (after the constructor)
     * and provides the canvas size.
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

    /**
     * Get selected sensor and channel, which data is shown.
     * 
     * @param[out] sensorIdx    Sensor index
     * @param[out] channelIdx   Sensor channel index
     * 
     * @return If selected sensor is available, it will return true otherwise false.
     */
    bool getSensorChannel(uint8_t& sensorIdx, uint8_t& channelIdx) const;

    /**
     * Select sensor and channel, which data to show.
     * 
     * @param[in] sensorIdx     Sensor index
     * @param[in] channelIdx    Sensor channel index
     * 
     * @return If sensor is available, it will return true otherwise false.
     */
    bool setSensorChannel(uint8_t sensorIdx, uint8_t channelIdx);

private:

    /**
     * Plugin topic, used for parameter exchange.
     */
    static const char*      TOPIC_CHANNEL;

    /** Sensor value update period in ms. */
    static const uint32_t   UPDATE_PERIOD   = 2000U;

    TextWidget              m_textWidget;       /**< Text widget, used for showing the text. */
    mutable MutexRecursive  m_mutex;            /**< Mutex to protect against concurrent access. */
    uint8_t                 m_sensorIdx;        /**< Index of selected sensor. */
    uint8_t                 m_channelIdx;       /**< Index of selected channel. */
    ISensorChannel*         m_sensorChannel;    /**< Values of this channel will be shown. */
    SimpleTimer             m_updateTimer;      /**< Sensor value update timer. */

    /**
     * Update shown information.
     */
    void update();

    /**
     * Get channel by indizes.
     * 
     * @param[in] sensorIdx     Sensor index
     * @param[in] channelIdx    Sensor channel index
     * 
     * @return Sensor channel
     */
    ISensorChannel* getChannel(uint8_t sensorIdx, uint8_t channelIdx);

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

#endif  /* __SENSOR_PLUGIN_H__ */

/** @} */
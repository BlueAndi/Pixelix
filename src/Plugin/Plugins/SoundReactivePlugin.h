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
 * @brief  Sound reactive plugin
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __SOUND_REACTIVE_PLUGIN_H__
#define __SOUND_REACTIVE_PLUGIN_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "Plugin.hpp"

#include <SimpleTimer.hpp>
#include <Mutex.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The sound reactive plugin shows a bar graph, which represents the frequency
 * bands of audio input.
 */
class SoundReactivePlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    SoundReactivePlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_mutex(),
        m_barHeight{0U},
        m_peakHeight{0U},
        m_numOfFreqBands(NUM_OF_BANDS_16),
        m_decayPeakTimer(),
        m_maxHeight(0U),
        m_freqBins(nullptr)
    {
        (void)m_mutex.create();
    }

    /**
     * Destroys the plugin.
     */
    ~SoundReactivePlugin()
    {
        if (nullptr != m_freqBins)
        {
            delete[] m_freqBins;
        }

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
        return new SoundReactivePlugin(name, uid);
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
     * Process the plugin.
     * Overwrite it if your plugin has cyclic stuff to do without being in a
     * active slot.
     */
    void process(void) final;

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

    /* Supported number of frequency bands. */
    enum NumOfBands
    {
        NUM_OF_BANDS_8  = 8,    /**< 8 bands */
        NUM_OF_BANDS_16 = 16    /**< 16 bands */
    };

    /**
     * Plugin topic, used for parameter exchange.
     */
    static const char*      TOPIC_CHANNEL;

    /**
     * The max. number of frequency bands, which the plugin is able to show.
     */
    static const uint8_t    MAX_FREQ_BANDS              = 16U;

    /**
     * Period in which the peak of a bar will be decayed in ms.
     */
    static const uint32_t   DECAY_PEAK_PERIOD           = 100U;

    /* Commonly used reference sound pressure: 20 uPa */
    static const constexpr float    ABS_THRESHOLD_OF_HEARING    = 20.0f;

    /* IMP441 nominal sensitivity is -26 dbFS (from datasheet) at 1 kHz.
     * Full scale: 2^23 - 1
     * 
     * => (2^23 - 1) * 10 ^ (-26/20) = 420426
     *
     * A pure acoustic tone at 1 kHz having 1 Pa RMS amplitude results in
     * 420426 digital peak amplitude.
     * 
     * => 420426 <-> 1 Pa RMS
     * => 0.420426 <-> 1 uPa RMS
     */
    static const constexpr float    VALUE_PER_1_UPA             = 0.420426f;

    /* INMP441 noise floor -87 dBFS (from datasheet) + 10%
     * Full scale: 2^23 - 1
     *
     * => (2^23 - 1) * 10 ^ (-87/20) = 374.71
     * => + 10 % = 412.18
     */
    static const constexpr double   NOISE_LEVEL                 = 412.18f;

    /**
     * List with the high edge frequency bin of the center band frequency.
     * This list is valid for 8 bands.
     */
    static const uint16_t   LIST_8_BAND_HIGH_EDGE_FREQ_BIN[NUM_OF_BANDS_8];

    /**
     * List with the high edge frequency bin of the center band frequency.
     * This list is valid for 16 bands.
     */
    static const uint16_t   LIST_16_BAND_HIGH_EDGE_FREQ_BIN[NUM_OF_BANDS_16];

    mutable MutexRecursive  m_mutex;                        /**< Mutex to protect against concurrent access. */
    uint16_t                m_barHeight[MAX_FREQ_BANDS];    /**< The current height of every bar, which represents a frequency band. */
    uint16_t                m_peakHeight[MAX_FREQ_BANDS];   /**< The peak of every bar, which represents the peak in the frequency band. */
    NumOfBands              m_numOfFreqBands;               /**< Current configured number of frequency bands, which to show. 8/16 are supported. */
    SimpleTimer             m_decayPeakTimer;               /**< Periodically decays the peak of a bar. */
    uint16_t                m_maxHeight;                    /**< Max. height of a bar in pixel. */
    double*                 m_freqBins;                     /**< List of frequency bins, calculated from the spectrum analyzer results. On the heap to avoid stack overflow. */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __SOUND_REACTIVE_PLUGIN_H__ */

/** @} */
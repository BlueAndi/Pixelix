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
 * @brief  Sound reactive plugin
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef SOUND_REACTIVE_PLUGIN_H
#define SOUND_REACTIVE_PLUGIN_H

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
#include <math.h>

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
        m_freqBins(nullptr),
        m_corrFactors(),
        m_peak(INMP441_MAX_SPL)
    {
        uint8_t bandIdx = 0U;

        (void)m_mutex.create();

        for(bandIdx = 0U; bandIdx < MAX_FREQ_BANDS; ++bandIdx)
        {
            m_corrFactors[bandIdx] = 1.0f;
        }
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

    /* Supported number of frequency bands. */
    enum NumOfBands
    {
        NUM_OF_BANDS_8  = 8,    /**< 8 bands */
        NUM_OF_BANDS_16 = 16    /**< 16 bands */
    };

    /**
     * Get current number of shown frequency bands.
     * 
     * @return Number of frequency bands
     */
    NumOfBands getFreqBandLen() const
    {
        NumOfBands                  freqBandLen     = NUM_OF_BANDS_8;
        MutexGuard<MutexRecursive>  guard(m_mutex);

        freqBandLen = m_numOfFreqBands;

        return freqBandLen;
    }

    /**
     * Set number of shown frequency bands.
     * 
     * @param[in] freqBandLen   Number of frequency bands to show
     */
    void setFreqBandLen(NumOfBands freqBandLen)
    {
        MutexGuard<MutexRecursive>  guard(m_mutex);

        if (freqBandLen != m_numOfFreqBands)
        {
            m_numOfFreqBands = freqBandLen;

            (void)saveConfiguration();
        }
    }

private:

    /**
     * Plugin topic, used for parameter exchange.
     */
    static const char*      TOPIC_CHANNEL;

    /**
     * The max. number of frequency bands, the plugin supports.
     * If you change this, the number of frequency bins which to sum up
     * must be calculated again.
     */
    static const uint8_t    MAX_FREQ_BANDS                      = 16U;

    /**
     * Period in which the peak of a bar will be decayed in ms.
     */
    static const uint32_t   DECAY_PEAK_PERIOD                   = 100U;

    /**
     * INMP441 data word bit width.
     */
    static const constexpr uint8_t  INMP441_DATA_WORD_BITS      = 24U;

    /**
     * INMP441 nominal sensitivity in dbFS at 1 kHz.
     */
    static const constexpr float    INMP441_SENSITIVITY         = -26.0f;

    /**
     * INMP441 the applied sound pressure level by measuring the sensitivity
     * at 1 kHz.
     */
    static const constexpr float    INMP441_SENSITIVITY_SPL     = 94.0f;

    /**
     * INMP441 the noise floor in dbFS.
     */
    static const constexpr float    INMP441_NOISE_FLOOR         = -87.0f;

    /**
     * The calculated full scale value of the INMP441.
     */
    static const constexpr int32_t  INMP441_FULL_SCALE          = (1 << (INMP441_DATA_WORD_BITS - 1)) - 1;

    /**
     * INMP441 the nominal sensitivity as digital value.
     * = 10^(sensitivity [dbFS] / 20) * full scale
     */
    static const constexpr int32_t  IMMP441_SENSITIVITY_DIGITAL = powf(10.0f, INMP441_SENSITIVITY / 20.0f) * INMP441_FULL_SCALE;

    /**
     * INMP441 the noise floor as digital value.
     * = 10^(noise floor [dbFS] / 20) * full scale
     */
    static const constexpr int32_t  INMP441_NOISE_FLOOR_DIGITAL = powf(10.0f, INMP441_NOISE_FLOOR / 20.0f) * INMP441_FULL_SCALE;

    /**
     * INMP441 the max. sound pressure level in db SPL.
     * = sensitivity [db SPL] + 20 * log10(full scale / sensitivity digital)
     */
    static const constexpr int32_t  INMP441_MAX_SPL             = INMP441_SENSITIVITY_SPL + 20.0f * log10f((1.0f * INMP441_FULL_SCALE) / IMMP441_SENSITIVITY_DIGITAL);

    /**
     * INMP441 the equivalent input noise in db SPL.
     * = sensitivity [db SPL] + 20 * log10(noise floor digital / sensitivity digital)
     */
    static const constexpr int32_t  INMP441_NOISE_SPL           = INMP441_SENSITIVITY_SPL + 20.0f * log10f((1.0f * INMP441_NOISE_FLOOR_DIGITAL) / IMMP441_SENSITIVITY_DIGITAL);

    /**
     * The human hearing threshold in dB SPL.
     */
    static const constexpr float    HEARING_THRESHOLD           = 0.0f;

    /**
     * Minimum dynamic range in dB SPL, on the y-axis.
     */
    static const constexpr float    MIN_DYNAMIC_RANGE           = 40.0f;

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
    float*                  m_freqBins;                     /**< List of frequency bins, calculated from the spectrum analyzer results. On the heap to avoid stack overflow. */
    float                   m_corrFactors[MAX_FREQ_BANDS];  /**< Correction factors per frequency band. The factors are calculated if the signal average is lower than the microphone noise floor. */
    float                   m_peak;                         /**< Determined signal peak over all frequency bands in dB SPL, used for AGC. */

    /**
     * Saves current configuration to JSON file.
     */
    bool saveConfiguration() const;

    /**
     * Load configuration from JSON file.
     */
    bool loadConfiguration();

    /**
     * Decay graphical signal peak periodically.
     */
    void decayPeak();

    /**
     * Handle frequency bins.
     * 
     * @param[out]  freqBins    Frequency bin buffer
     * @param[in]   freqBinLen  Length of frequency bin buffer in elements.
     * 
     * @return If successful, it will return true otherwise false.
     */
    void handleFreqBins(float* freqBins, size_t freqBinLen);

    void convertToOctaveFreqBands(float* octaveFreqBands, size_t octaveFreqBandsLen, float* freqBins, size_t freqBinLen);
    float calculateAmplitudeAverage(float* octaveFreqBands, size_t octaveFreqBandsLen);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* SOUND_REACTIVE_PLUGIN_H */

/** @} */
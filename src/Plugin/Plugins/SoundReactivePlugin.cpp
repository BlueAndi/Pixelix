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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "SoundReactivePlugin.h"
#include "SpectrumAnalyzer.h"

#include <Logging.h>
#include <FileSystem.h>
#include <JsonFile.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/* Initialize plugin topic. */
const char*     SoundReactivePlugin::TOPIC_CHANNEL                      = "/cfg";

/* Initialize the list with the high edge frequency bin of the center band frequency. */
const uint16_t  SoundReactivePlugin::LIST_16_BAND_HIGH_EDGE_FREQ_BIN[]  =
{
    4U,
    5U,
    7U,
    9U,
    12U,
    16U,
    21U,
    27U,
    36U,
    48U,
    63U,
    84U,
    111U,
    146U,
    193U,
    255U
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void SoundReactivePlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC_CHANNEL);
}

bool SoundReactivePlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CHANNEL))
    {
        value["freqBandLen"] = getFreqBandLen();

        isSuccessful = true;
    }

    return isSuccessful;
}

bool SoundReactivePlugin::setTopic(const String& topic, const JsonObject& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CHANNEL))
    {
        JsonVariant jsonFreqBandLen = value["freqBandLen"];

        if (false == jsonFreqBandLen.isNull())
        {
            uint8_t freqBandLen = jsonFreqBandLen.as<uint8_t>();

            if (NUM_OF_BANDS_8 == freqBandLen)
            {
                setFreqBandLen(NUM_OF_BANDS_8);
                isSuccessful = true;
            }
            else if (NUM_OF_BANDS_16 == freqBandLen)
            {
                setFreqBandLen(NUM_OF_BANDS_16);
                isSuccessful = true;
            }
            else
            {
                ;
            }
        }
    }

    return isSuccessful;
}

void SoundReactivePlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    UTIL_NOT_USED(width);

    m_freqBins = new(std::nothrow) double[SpectrumAnalyzer::getInstance().getFreqBinsLen()];

    if (nullptr == m_freqBins)
    {
        LOG_ERROR("Couldn't get memory for frequency bins.");
    }
    else
    {
        SpectrumAnalyzer::getInstance().start();
    }

    m_decayPeakTimer.start(DECAY_PEAK_PERIOD);
    m_maxHeight = height;

    /* Try to load configuration. If there is no configuration available, a default configuration
     * will be created.
     */
    if (false == loadConfiguration())
    {
        if (false == saveConfiguration())
        {
            LOG_WARNING("Failed to create initial configuration file %s.", getFullPathToConfiguration().c_str());
        }
    }

    return;
}

void SoundReactivePlugin::stop()
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    String                      configurationFilename   = getFullPathToConfiguration();

    m_decayPeakTimer.stop();

    SpectrumAnalyzer::getInstance().stop();

    if (nullptr != m_freqBins)
    {
        delete[] m_freqBins;
        m_freqBins = nullptr;
    }

    if (false != FILESYSTEM.remove(configurationFilename))
    {
        LOG_INFO("File %s removed", configurationFilename.c_str());
    }

    return;
}

void SoundReactivePlugin::process(bool isConnected)
{
    uint8_t                     bandIdx         = 0U;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    UTIL_NOT_USED(isConnected);

    if (true == SpectrumAnalyzer::getInstance().areFreqBinsReady())
    {
        const size_t freqBinLen = SpectrumAnalyzer::getInstance().getFreqBinsLen();

        /* Decay peak periodically */
        if (true == m_decayPeakTimer.isTimeout())
        {
            for(bandIdx = 0U; bandIdx < m_numOfFreqBands; ++bandIdx)
            {
                if (0U < m_peakHeight[bandIdx])
                {
                    --m_peakHeight[bandIdx];
                }
            }

            m_decayPeakTimer.restart();
        }

        if (nullptr != m_freqBins)
        {
            /* Copy frequency bins from spectrum analyzer. */
            if (true == SpectrumAnalyzer::getInstance().getFreqBins(m_freqBins, freqBinLen))
            {
                float           octaveFreqBands[MAX_FREQ_BANDS];
                uint16_t        freqBinIdx                      = 0U;
                int32_t         divisor                         = 0;
                float           peak                            = 0.0f;
                float           avgDigital                      = 0.0f;

                /* Sum up the frequency bin results of the spectrum analyzer and
                 * create the octave frequency bands.
                 */
                freqBinIdx  = 1U; /* Don't use the first frequency bin, because it contains the DC part. */
                bandIdx     = 0U;
                octaveFreqBands[bandIdx] = 0.0f;
                while((freqBinLen > freqBinIdx) && (MAX_FREQ_BANDS > bandIdx))
                {
                    octaveFreqBands[bandIdx] += static_cast<float>(m_freqBins[freqBinIdx]);
                    ++divisor; /* Count number of added frequency bins. */

                    /* If the current frequency bin is equal than the current
                     * high edge frequency of the band, the following frequency
                     * bin's will be assigned to the next band.
                     */
                    if ((MAX_FREQ_BANDS > bandIdx) &&
                        (LIST_16_BAND_HIGH_EDGE_FREQ_BIN[bandIdx] == freqBinIdx))
                    {
                        /* Any frequency band added? */
                        if (0 < divisor)
                        {
                            /* Depends on how many frequency bins were added. */
                            octaveFreqBands[bandIdx] /= static_cast<float>(divisor);

                            divisor = 0;
                        }

                        ++bandIdx;

                        if (MAX_FREQ_BANDS > bandIdx)
                        {
                            octaveFreqBands[bandIdx] = static_cast<float>(m_freqBins[freqBinIdx]);
                            ++divisor; /* Count number of added frequency bins. */
                        }
                    }

                    ++freqBinIdx;
                }

                /* Calculate the amplitude average over the spectrum. */
                for(bandIdx = 0U; bandIdx < MAX_FREQ_BANDS; ++bandIdx)
                {
                    avgDigital += octaveFreqBands[bandIdx];
                }
                avgDigital /= static_cast<float>(MAX_FREQ_BANDS);

                for(bandIdx = 0U; bandIdx < MAX_FREQ_BANDS; ++bandIdx)
                {
                    /* If the ampltiude average is lower than the equivalent input noise (from datasheet),
                     * the correction factors will be calculated. The amplitude average is used to detect
                     * silence, which is necessary for this automatic calibration.
                     */
                    if (INMP441_NOISE_FLOOR_DIGITAL > static_cast<int32_t>(avgDigital))
                    {
                        constexpr const float   WEIGHT_NEW_VALUE    = 0.1f;
                        constexpr const float   WEIGHT_OLD_VALUE    = 1.0f - WEIGHT_NEW_VALUE;
                        constexpr const float   NOISE_FLOOR         = static_cast<float>(INMP441_NOISE_FLOOR_DIGITAL);

                        /* Calculate with weighted average to avoid jumping. */
                        m_corrFactors[bandIdx] = WEIGHT_OLD_VALUE * m_corrFactors[bandIdx] + WEIGHT_NEW_VALUE * (NOISE_FLOOR / octaveFreqBands[bandIdx]);
                    }

                    /* Normalize */
                    octaveFreqBands[bandIdx] *= m_corrFactors[bandIdx];

                    /* Calculate the spectrum amplitude in dB SPL
                     * The shown frequency spectrum amplitudes consider now the silent and loud parts better.
                     *
                     * = sensitivity [dB SPL] + 20 * log10(frequency amplitude digital / sensitivity digital)
                     */
                    octaveFreqBands[bandIdx] = INMP441_SENSITIVITY_SPL + 20.0f * log10f(octaveFreqBands[bandIdx] / static_cast<float>(IMMP441_SENSITIVITY_DIGITAL));

                    /* The amplitude shall consider only the dynamic range
                     * by removing the equivalent input noise level.
                     */
                    if (INMP441_NOISE_SPL >= octaveFreqBands[bandIdx])
                    {
                        octaveFreqBands[bandIdx] = HEARING_THRESHOLD;
                    }
                    else
                    {
                        octaveFreqBands[bandIdx] -= INMP441_NOISE_SPL;
                    }

                    /* Determine peak over all frequency bands for automatic gain control. */
                    if (octaveFreqBands[bandIdx] > peak)
                    {
                        peak = octaveFreqBands[bandIdx];
                    }
                }

                /* Adapt the dynamic range on the y-axis, but limit it to a minimum,
                 * otherwise the bar's will jump driven by silent tones.
                 */
                {
                    constexpr const float   WEIGHT_NEW_VALUE    = 0.25f;
                    constexpr const float   WEIGHT_OLD_VALUE    = 1.0f - WEIGHT_NEW_VALUE;

                    m_peak = WEIGHT_NEW_VALUE * peak + WEIGHT_OLD_VALUE * m_peak;

                    if (MIN_DYNAMIC_RANGE > m_peak)
                    {
                        m_peak = MIN_DYNAMIC_RANGE;
                    }
                }

                /* Downscale to the bar height in relation to dynamic range.
                 * If less frequency bands are shown, they will be simply averaged.
                 */
                freqBinIdx = 0U;
                for(bandIdx = 0U; bandIdx < MAX_FREQ_BANDS; ++bandIdx)
                {
                    float       avg         = 0.0f;
                    uint16_t    barHeight   = 0U;
                    const float MAX_HEIGHT  = static_cast<float>(m_maxHeight);

                    if (NUM_OF_BANDS_8 == m_numOfFreqBands)
                    {
                        avg = (octaveFreqBands[freqBinIdx] + octaveFreqBands[freqBinIdx + 1U]) / 2.0f;
                        freqBinIdx += 2U;
                    }
                    else
                    {
                        avg = octaveFreqBands[freqBinIdx];
                        freqBinIdx += 1U;
                    }

                    barHeight = static_cast<uint16_t>((avg * MAX_HEIGHT) / m_peak);

                    if (m_maxHeight < barHeight)
                    {
                        barHeight = m_maxHeight;
                    }

                    m_barHeight[bandIdx] = barHeight;

                    /* Move peak up, if necessary. */
                    if (m_barHeight[bandIdx] > m_peakHeight[bandIdx])
                    {
                        m_peakHeight[bandIdx] = m_barHeight[bandIdx];
                    }
                }
            }
        }
    }

    return;
}

void SoundReactivePlugin::update(YAGfx& gfx)
{
    int8_t                      bandIdx         = 0U;
    uint16_t                    barWidth        = gfx.getWidth() / m_numOfFreqBands;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    gfx.fillScreen(ColorDef::BLACK);
    
    for(bandIdx = 0U; bandIdx < m_numOfFreqBands; ++bandIdx)
    {
        int16_t peakY = 0;

        /* The bar shall have a minium height of 2, otherwise it will be
         * overdrawn by the peak. The peak will be always on the top of
         * the bar.
         */
        if (2U <= m_barHeight[bandIdx])
        {
            Color   barColor;

            barColor.turnColorWheel((255U / (m_numOfFreqBands + 1U) * bandIdx));

            gfx.fillRect(   bandIdx * barWidth,
                            gfx.getHeight() - m_barHeight[bandIdx] + 1,
                            barWidth,
                            m_barHeight[bandIdx] - 1U,
                            barColor);
        }

        /* A peak height of 0 and 1 is handled equal, because is shall always
         * be shown at the bottom in these cases. In all others the peak is
         * on top of the bar.
         */
        peakY = gfx.getHeight() - m_peakHeight[bandIdx] - 1;
        if (0U < m_peakHeight[bandIdx])
        {
            peakY++;
        }
        gfx.drawHLine(  bandIdx * barWidth,
                        peakY,
                        barWidth,
                        ColorDef::WHITE);
    }

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool SoundReactivePlugin::saveConfiguration() const
{
    bool                status                  = true;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    String              configurationFilename   = getFullPathToConfiguration();

    jsonDoc["freqBandLen"] = m_numOfFreqBands;
    
    if (false == jsonFile.save(configurationFilename, jsonDoc))
    {
        LOG_WARNING("Failed to save file %s.", configurationFilename.c_str());
        status = false;
    }
    else
    {
        LOG_INFO("File %s saved.", configurationFilename.c_str());
    }

    return status;
}

bool SoundReactivePlugin::loadConfiguration()
{
    bool                status                  = true;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    String              configurationFilename   = getFullPathToConfiguration();

    if (false == jsonFile.load(configurationFilename, jsonDoc))
    {
        LOG_WARNING("Failed to load file %s.", configurationFilename.c_str());
        status = false;
    }
    else
    {
        JsonVariant jsonFreqBandLen = jsonDoc["freqBandLen"];

        if (false == jsonFreqBandLen.is<uint8_t>())
        {
            LOG_WARNING("freqBandLen not found or invalid type.");
            status = false;
        }
        else
        {
            m_numOfFreqBands = static_cast<NumOfBands>(jsonFreqBandLen.as<uint8_t>());
        }
    }

    return status;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

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
const uint16_t  SoundReactivePlugin::LIST_8_BAND_HIGH_EDGE_FREQ_BIN[]  =
{
    3U,
    6U,
    14U,
    29U,
    62U,
    132U,
    281U,
    598U
};

/* Initialize the list with the high edge frequency bin of the center band frequency. */
const uint16_t  SoundReactivePlugin::LIST_16_BAND_HIGH_EDGE_FREQ_BIN[]  =
{
    2U,
    3U,
    5U,
    7U,
    10U,
    14U,
    20U,
    29U,
    41U,
    59U,
    84U,
    119U,
    169U,
    241U,
    343U,
    489U
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

    m_freqBins = new double[SpectrumAnalyzer::getInstance().getFreqBinsLen()];

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

void SoundReactivePlugin::process()
{
    uint8_t                     bandIdx         = 0U;
    MutexGuard<MutexRecursive>  guard(m_mutex);

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

    if (true == SpectrumAnalyzer::getInstance().areFreqBinsReady())
    {
        const size_t freqBinLen = SpectrumAnalyzer::getInstance().getFreqBinsLen();

        if (nullptr != m_freqBins)
        {
            /* Copy frequency bins from spectrum analyzer. */
            if (true == SpectrumAnalyzer::getInstance().getFreqBins(m_freqBins, freqBinLen))
            {
                float           octaveFreqBands[m_numOfFreqBands]   = { 0.0f };
                uint16_t        freqBinIdx                          = 0U;
                const uint16_t* bandHighEdgeFreqs                   = nullptr;
                int32_t         divisor                             = 0;

                /* Choose the right list of high edge frequency bins. */
                if (NUM_OF_BANDS_8 == m_numOfFreqBands)
                {
                    bandHighEdgeFreqs = LIST_8_BAND_HIGH_EDGE_FREQ_BIN;
                }
                else
                {
                    bandHighEdgeFreqs = LIST_16_BAND_HIGH_EDGE_FREQ_BIN;
                }

                /* Analyze the frequency bin results of the spectrum analyzer and
                 * create the octave frequency bands.
                 *
                 * Don't use the first frequency bin, because it contains the DC part.
                 */
                bandIdx = 0U;
                for(freqBinIdx = 1U; freqBinIdx < freqBinLen; ++freqBinIdx)
                {
                    octaveFreqBands[bandIdx] += static_cast<float>(m_freqBins[freqBinIdx]);
                    ++divisor; /* Count number of added frequency bins. */

                    /* If the current frequency bin is equal than the current
                     * high edge frequency of the band, the following frequency
                     * bin's will be assigned to the next band.
                     */
                    if ((m_numOfFreqBands > bandIdx) &&
                        (bandHighEdgeFreqs[bandIdx] == freqBinIdx))
                    {
                        /* Any frequency band added? */
                        if (0 < divisor)
                        {
                            /* Depends on how many frequency bins were added. */
                            octaveFreqBands[bandIdx] /= static_cast<float>(divisor);

                            divisor = 0;
                        }

                        ++bandIdx;
                    }
                }

                if (0 < divisor)
                {
                    /* Depends on how many frequency bins were added. */
                    octaveFreqBands[bandIdx] /= static_cast<float>(divisor);
                }

                /* Calculate the amplitude in dB SPL.
                 * The shown frequency spectrum amplitudes consider now the silent and loud parts better.
                 *
                 * = sensitivity [db SPL] + 20 * log10(frequency amplitude digital / sensitivity digital)
                 */
                for(bandIdx = 0U; bandIdx < m_numOfFreqBands; ++bandIdx)
                {
                    octaveFreqBands[bandIdx] = INMP441_SENSITIVITY_SPL + 20.0f * log10f(abs(octaveFreqBands[bandIdx]) / IMMP441_SENSITIVITY_DIGITAL);

                    /* Remove noise */
                    if (INMP441_NOISE_SPL > octaveFreqBands[bandIdx])
                    {
                        octaveFreqBands[bandIdx] = 0.0f;
                    }

                    printf("f%u = %f\n", bandIdx, octaveFreqBands[bandIdx]);
                }

                /* Downscale to the bar height in relation to 120 dB.
                 *
                 * Note, there is currently no behaviour like automatic gain control.
                 */
                for(bandIdx = 0U; bandIdx < m_numOfFreqBands; ++bandIdx)
                {
                    uint16_t barHeight = static_cast<uint16_t>((octaveFreqBands[bandIdx] * m_maxHeight) / 120.0f);

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

        if ((true == jsonFreqBandLen.isNull()) ||
            (false == jsonFreqBandLen.is<uint8_t>()))
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

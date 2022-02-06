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
const char* SoundReactivePlugin::TOPIC_CHANNEL = "/channel";

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
        isSuccessful = true;
    }
    else
    {
        ;
    }

    return isSuccessful;
}

bool SoundReactivePlugin::setTopic(const String& topic, const JsonObject& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CHANNEL))
    {
    }
    else
    {
        ;
    }

    return isSuccessful;
}

void SoundReactivePlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_decayPeakTimer.start(DECAY_PEAK_PERIOD);
    m_maxHeight = height;

    return;
}

void SoundReactivePlugin::stop()
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_decayPeakTimer.stop();

    return;
}

void SoundReactivePlugin::process()
{
    uint8_t                     bandIdx         = 0U;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    /* Decay peak periodically */
    if (true == m_decayPeakTimer.isTimeout())
    {
        for(bandIdx = 0U; bandIdx < m_bars; ++bandIdx)
        {
            if (0U < m_peakHeight[bandIdx])
            {
                --m_peakHeight[bandIdx];
            }
        }

        m_decayPeakTimer.restart();
    }

    /* Update bar height */
    for(bandIdx = 0U; bandIdx < m_bars; ++bandIdx)
    {
        /* TODO */
        if (0U == bandIdx)
        {
            m_barHeight[bandIdx] = 0U;
        }
        else if (1U == bandIdx)
        {
            m_barHeight[bandIdx] = m_maxHeight / 2U;
        }
        else if (2U == bandIdx)
        {
            m_barHeight[bandIdx] = m_maxHeight;
        }
        else
        {
            m_barHeight[bandIdx] += random(m_maxHeight);
            m_barHeight[bandIdx] /= 2U;
        }

        /* Move peak up */
        if (m_barHeight[bandIdx] > m_peakHeight[bandIdx])
        {
            m_peakHeight[bandIdx] = m_barHeight[bandIdx];
        }
    }

    return;
}

void SoundReactivePlugin::active(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    UTIL_NOT_USED(gfx);

    return;
}

void SoundReactivePlugin::inactive()
{
    /* Nothing to do. */
    return;
}

void SoundReactivePlugin::update(YAGfx& gfx)
{
    int8_t                      bandIdx         = 0U;
    uint16_t                    barWidth        = gfx.getWidth() / m_bars;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    gfx.fillScreen(ColorDef::BLACK);
    
    for(bandIdx = 0U; bandIdx < m_bars; ++bandIdx)
    {
        int16_t peakY = 0;

        /* The bar shall have a minium height of 2, otherwise it will be
         * overdrawn by the peak. The peak will be always on the top of
         * the bar.
         */
        if (2U <= m_barHeight[bandIdx])
        {
            Color   barColor;

            barColor.turnColorWheel((255U / (m_bars + 1U) * bandIdx));

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
                        gfx.getHeight() - m_peakHeight[bandIdx] + 1,
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

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

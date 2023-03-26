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
 * @brief  Audio tone detector by using Goertzel algorithm
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "AudioToneDetector.h"
#include <math.h>
#include <Logging.h>

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

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void AudioToneDetector::notify(int32_t* data, size_t size)
{
    /* If the target frequency is around 0 Hz, no calculation takes place. */
    if (((EPSILON < m_targetFreq) || (-EPSILON > m_targetFreq)) &&
        (nullptr != data))
    {
        size_t  index           = 0U;
        float   q0              = 0.0F;
        float   q1              = 0.0F;
        float   q2              = 0.0F;
        float   realValue       = 0.0F;
        float   imagValue       = 0.0F;
        float   magnitude       = 0.0F;
        float   scalingFactor   = static_cast<float>(size) / 2.0F;

        while(size > index)
        {
            float fData = static_cast<float>(data[index]);

            q0  = m_coeff * q1 - q2 + applyHanningWindow(fData, index, AudioDrv::SAMPLES);
            q2  = q1;
            q1  = q0;

            ++index;
        }

        realValue = q1 - q2 * m_cosValue;
        realValue /= scalingFactor;

        imagValue = q2 * m_sinValue;
        imagValue /= scalingFactor;

        magnitude = sqrtf(realValue * realValue + imagValue * imagValue);
        magnitude = applyHanningMagnitudeCorrection(magnitude);

        if (m_threshold < magnitude)
        {
            /* Still detected? */
            if (true == m_isDetected)
            {
                /* Wait until the application has read it. */
                ;
            }
            /* The target frequency must be detected over a specific duration. */
            else if (false == m_timer.isTimerRunning())
            {
                m_timer.start(m_minDuration);
            }
            else if (true == m_timer.isTimeout())
            {
                m_isDetected = true;
            }
            else
            {
                ;
            }

            m_lastMagntiude = magnitude;
        }
        else
        {
            m_timer.stop();
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void AudioToneDetector::preCompute()
{
    float       fIndex  = 0.5F + ((AudioDrv::SAMPLES * m_targetFreq) / AudioDrv::SAMPLE_RATE);
    uint32_t    index   = static_cast<uint32_t>(fIndex);

    m_omega = (2.0F * M_PI) / AudioDrv::SAMPLES;
    m_omega *= index;

    m_cosValue = cos(m_omega);
    m_sinValue = sin(m_omega);

    m_coeff = 2.0F * m_cosValue;
}

float AudioToneDetector::applyHanningWindow(float data, uint32_t sampleIndex, uint32_t samples)
{
    float fSamples = static_cast<float>(samples);

    return data * (0.54F - 0.46F * cos( 2.0F * M_PI * sampleIndex / fSamples));
}

float AudioToneDetector::applyHanningMagnitudeCorrection(float data)
{
    return data * 2.0F;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

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
 * 
 * @addtogroup audio_service
 *
 * @{
 */

#ifndef AUDIO_TONE_DETECTOR_H
#define AUDIO_TONE_DETECTOR_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <Mutex.hpp>
#include <SimpleTimer.hpp>

#include "AudioDrv.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Audio tone detection by using the Goertzel algorithm.
 * 
 * https://en.wikipedia.org/wiki/Goertzel_algorithm
 */
class AudioToneDetector : public IAudioObserver
{
public:

    /**
     * Constructs the audio tone detector instance.
     */
    AudioToneDetector() :
        m_mutex(),
        m_targetFreq(0.0f),
        m_omega(0.0f),
        m_cosValue(0.0f),
        m_sinValue(0.0f),
        m_coeff(0.0f),
        m_minDuration(0U),
        m_threshold(0.0f),
        m_isDetected(false),
        m_timer(),
        m_lastMagntiude(0.0f)
    {
    }

    /**
     * Destroys the audio tone detector instance.
     */
    ~AudioToneDetector()
    {
        /* Never called. */
    }

    /**
     * Get the target frequency.
     * 
     * @return Target frequency in Hz
     */
    float getTargetFreq() const
    {
        return m_targetFreq;
    }

    /**
     * Set the target frequency.
     * 
     * @param[in] freq  Target frequency in Hz
     */
    void setTargetFreq(float freq)
    {
        if (m_targetFreq != freq)
        {
            m_targetFreq = freq;
            preCompute();
        }
    }

    /**
     * Get the min. duration which the target frequency must be present.
     * 
     * @return Min. duration in ms
     */
    uint32_t getMinDuration() const
    {
        return m_minDuration;
    }

    /**
     * Set the min. duration which the target frequency must be present.
     * 
     * @param[in] duration  Min. duration in ms
     */
    void setMinDuration(uint32_t duration)
    {
        m_minDuration = duration;
    }

    /**
     * Get the magntiude threshold.
     * 
     * @return Magnitude threshold
     */
    float getThreshold() const
    {
        return m_threshold;
    }

    /**
     * Set the magnitude threshold. The audio signal must be greater than the
     * threshold to be recognized.
     * 
     * @param[in] threshold Magnitude threshold
     */
    void setThreshold(float threshold)
    {
        m_threshold = threshold;
    }

    /**
     * Is the target frequency detected?
     * 
     * @return If audio signal is detected, it will return true otherwise false.
     */
    bool isTargetFreqDetected()
    {
        bool isDetected = m_isDetected;

        /* Reset detection flag.
         * This is done here to ensure the application doesn't miss it.
         */
        m_isDetected = false;

        return isDetected;
    }

    /**
     * Get the last magnitude which was greater than the threshold.
     * 
     * @return Last magnitude
     */
    float getLastMagnitude() const
    {
        return m_lastMagntiude;
    }

    /**
     * The audio driver will call this method to notify about a complete available
     * number of samples.
     * 
     * @param[in]   data    Audio sample data buffer
     * @param[in]   size    Number of audio samples
     */
    void notify(int32_t* data, size_t size) final;

    /**
     * The epsilon is used to determine a 0 floating value.
     */
    static constexpr float  EPSILON         = 0.0001f;

private:

    mutable Mutex   m_mutex;            /**< Mutex used for concurrent access protection. */
    float           m_targetFreq;       /**< Target frequency in Hz */
    float           m_omega;            /**< Precomputed angle velocity */
    float           m_cosValue;         /**< Precomputed cosinus value */
    float           m_sinValue;         /**< Precomputed sinus value */
    float           m_coeff;            /**< Precomputed coefficient */
    float           m_threshold;        /**< Threshold for target frequency detection. */
    uint32_t        m_minDuration;      /**< The min. duration the target frequency must be active in ms.*/
    bool            m_isDetected;       /**< Is target frequency detected? */
    SimpleTimer     m_timer;            /**< Timer used for target frequency detection. */
    float           m_lastMagntiude;    /**< Last magnitude which was greater than the threshold. */

    AudioToneDetector(const AudioToneDetector& drv);
    AudioToneDetector& operator=(const AudioToneDetector& drv);

    /**
     * Precompute some values for faster recognization phase.
     */
    void preCompute();

    /**
     * Apply Hanning windowing to sample data.
     * 
     * @param[in] data          Sample data
     * @param[in] sampleIndex   Sample index
     * @param[in] samples       Number of samples
     * 
     * @return Sample data after windowing
     */
    float applyHanningWindow(float data, uint32_t sampleIndex, uint32_t samples);

    /**
     * Apply Hanning windowing correction factor for the magnitude.
     * 
     * @param[in] data Sample data
     * 
     * @return Sample data after correction
     */
    float applyHanningMagnitudeCorrection(float data);
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* AUDIO_TONE_DETECTOR_H */

/** @} */
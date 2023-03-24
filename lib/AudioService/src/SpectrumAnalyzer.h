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
 * @brief  Spectrum analyzer
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup audio_service
 *
 * @{
 */

#ifndef SPECTRUM_ANALYZER_H
#define SPECTRUM_ANALYZER_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <arduinoFFT.h>
#include <Mutex.hpp>

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
 * A spectrum analyzer, which transforms time discrete samples to
 * frequency spectrum bands.
 */
class SpectrumAnalyzer : public IAudioObserver
{
public:

    /**
     * Constructs the spectrum analyzer instance.
     */
    SpectrumAnalyzer() :
        m_mutex(),
        m_real{0.0f},
        m_imag{0.0f},
        m_fft(m_real, m_imag, AudioDrv::SAMPLES, AudioDrv::SAMPLE_RATE),
        m_freqBins{0.0f},
        m_freqBinsAreReady(false)
    {
    }

    /**
     * Destroys the spectrum analyzer instance.
     */
    ~SpectrumAnalyzer()
    {
        /* Never called. */
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
     * Get the number of frequency bins.
     * 
     * @return Number of frequency bins
     */
    size_t getFreqBinsLen() const
    {
        return FREQ_BINS;
    }

    /**
     * Get frequency bins by copy.
     * 
     * @param[out]  freqBins    Frequency bin buffer, where to write.
     * @param[in]   len         Length of frequency bin buffer in elements.
     * 
     * @return If successful, it will return true otherwise false.
     */
    bool getFreqBins(float* freqBins, size_t len);

    /**
     * Are the frequency bins updated and ready?
     * 
     * @return If the frequency bins are ready, it will return true otherwise false.
     */
    bool areFreqBinsReady() const
    {
        MutexGuard<Mutex>   guard(m_mutex);

        return m_freqBinsAreReady;
    }

private:

    /**
     * The number of frequency bins over the spectrum. Note, this is always
     * half of the samples, because they are symmetrical around DC.
     */
    static const uint32_t   FREQ_BINS   = AudioDrv::SAMPLES / 2U;

    mutable Mutex       m_mutex;                    /**< Mutex used for concurrent access protection. */
    float               m_real[AudioDrv::SAMPLES];  /**< The real values. */
    float               m_imag[AudioDrv::SAMPLES];  /**< The imaginary values. */
    ArduinoFFT<float>   m_fft;                      /**< The FFT algorithm. */
    float               m_freqBins[FREQ_BINS];      /**< The frequency bins as result of the FFT, with linear magnitude. */
    bool                m_freqBinsAreReady;         /**< Are the frequency bins ready for the application? */

    SpectrumAnalyzer(const SpectrumAnalyzer& drv);
    SpectrumAnalyzer& operator=(const SpectrumAnalyzer& drv);

    /**
     * Transform from discrete time to frequency spectrum.
     * Note, the magnitude will be calculated linear and not in dB.
     */
    void calculateFFT();

    /**
     * Copy FFT result to frequency bins.
     * This function is protected against concurrent access.
     */
    void copyFreqBins();
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* SPECTRUM_ANALYZER_H */

/** @} */
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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "SpectrumAnalyzer.h"

#include <Logging.h>
#include <Board.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

#define SPECTRUM_ANALYZER_SIM_SIN_EN    0

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/**
 * Provides the FFT window correction factor.
 * See the National Instruments application note 041:
 * The Fundamentals of FFT-Based Signal Analysis and Measurement
 * 
 * @tparam windowType The window type, see arduinoFFT.
 */
template < FFTWindow windowType >
struct WindowCorrection
{
    /* No implementation to force a compile error. */
};

/**
 * The FFT rectangle window correction factor. 
 */
template <>
struct WindowCorrection<FFTWindow::Rectangle>
{
    static constexpr const float factor = 1.0F;
};

/**
 * The FFT hamming window correction factor. 
 */
template <>
struct WindowCorrection<FFTWindow::Hamming>
{
    static constexpr const float factor = 0.54F;
};

/**
 * The FFT hann window correction factor. 
 */
template <>
struct WindowCorrection<FFTWindow::Hann>
{
    static constexpr const float factor = 0.50F;
};

/**
 * The FFT blackman-harris window correction factor. 
 */
template <>
struct WindowCorrection<FFTWindow::Blackman_Harris>
{
    static constexpr const float factor = 0.42F;
};

/**
 * The FFT flat top window correction factor. 
 */
template <>
struct WindowCorrection<FFTWindow::Flat_top>
{
    static constexpr const float factor = 0.22F;
};

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void SpectrumAnalyzer::notify(int32_t* data, size_t size)
{
    if (nullptr != data)
    {
        size_t index = 0U;

#if (SPECTRUM_ANALYZER_SIM_SIN_EN != 0)

        /* Simulate the sampling of a sinusoidal 1000 Hz signal
         * with an amplitude of 94 db SPL, sampled at 40000 Hz.
         */
        {
            float signalFrequency   = 1000.0F;
            float cycles            = (((AudioDrv::SAMPLES - 1U) * signalFrequency) / AudioDrv::SAMPLE_RATE);   /* Number of signal cycles that the sampling will read. */
            float amplitude         = 420426.0F; /* 94 db SPL */

            for (uint16_t sampleIdx = 0U; sampleIdx < AudioDrv::SAMPLES; ++sampleIdx)
            {
                /* Build data with positive and negative values. */
                m_real[sampleIdx] = (amplitude * (sin((sampleIdx * (twoPi * cycles)) / AudioDrv::SAMPLES))) / 2.0F;
                
                /* Imaginary part must be zeroed in case of looping to avoid wrong calculations and overflows. */
                m_imag[sampleIdx] = 0.0F;
            }

            m_isMicAvailable = true;
        }

#else /* (SPECTRUM_ANALYZER_SIM_SIN_EN != 0) */

        while(index < size)
        {
            m_real[index] = static_cast<float>(data[index]);
            m_imag[index] = 0.0F;

            ++index;
        }

#endif  /* (SPECTRUM_ANALYZER_SIM_SIN_EN == 0) */

        /* Transform the time discrete values to the frequency spectrum. */
        calculateFFT();

        /* Store the frequency bins and provide it to the application. */
        copyFreqBins();
    }
}

bool SpectrumAnalyzer::getFreqBins(float* freqBins, size_t len)
{
    bool                isSuccessful    = false;
    MutexGuard<Mutex>   guard(m_mutex);

    if ((nullptr != freqBins) &&
        (0U < len) &&
        ((FREQ_BINS >= len)))
    {
        size_t idx = 0U;

        for(idx = 0U; idx < len; ++idx)
        {
            freqBins[idx] = m_freqBins[idx];
        }

        m_freqBinsAreReady = false;

        isSuccessful = true;
    }

    return isSuccessful;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void SpectrumAnalyzer::calculateFFT()
{
    static const constexpr  float       HALF_SPECTRUM_ENERGY_CORRECTION_FACTOR  = 2.0F;
    static const constexpr  FFTWindow   WINDOW_TYPE                             = FFTWindow::Hamming;
    uint16_t                            idx                                     = 0U;

    /* Note, current arduinoFFT version has a wrong Hann window calculation! */
    m_fft.windowing(WINDOW_TYPE, FFTDirection::Forward);
    m_fft.compute(FFTDirection::Forward);
    m_fft.complexToMagnitude();

    /* In a two-sided spectrum, half the energy is displayed at the positive
     * frequency, and half the energy is displayed at the negative frequency.
     * Therefore, to convert from a two-sided spectrum to a single-sided
     * spectrum, discard the second half of the array and multiply every
     * point except for DC by two.
     * 
     * Depended on the kind of window, it is compensated by multiplication of
     * the corresponding correction factor.
     * 
     * Result is the amplitude spectrum.
     */
    for(idx = 1U; idx < FREQ_BINS; ++idx)
    {
        m_real[idx] *= HALF_SPECTRUM_ENERGY_CORRECTION_FACTOR;
        m_real[idx] /= AudioDrv::SAMPLES * WindowCorrection<WINDOW_TYPE>::factor;
    }
}

void SpectrumAnalyzer::copyFreqBins()
{
    uint16_t            idx             = 0U;
    MutexGuard<Mutex>   guard(m_mutex);

    for(idx = 0U; idx < FREQ_BINS; ++idx)
    {
        m_freqBins[idx] = m_real[idx];
    }

    m_freqBinsAreReady = true;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

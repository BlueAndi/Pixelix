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
 * @brief  Spectrum analyzer
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup hal
 *
 * @{
 */

#ifndef __SPECTRUM_ANALYZER_H__
#define __SPECTRUM_ANALYZER_H__

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <arduinoFFT.h>
#include <driver/i2s.h>
#include <Queue.hpp>
#include <Mutex.hpp>

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
class SpectrumAnalyzer
{
public:

    /**
     * Get spectrum analyzer instance.
     * 
     * @return Spectrum analyzer instance
     */
    static SpectrumAnalyzer& getInstance()
    {
        static SpectrumAnalyzer instance;   /* idiom */

        return instance;
    }

    /**
     * Start the spectrum analyzer.
     * If it is already started, nothing happens.
     * 
     * @return If successful started, it will return true otherwise false.
     */
    bool start();

    /**
     * Stop the spectrum analyzer.
     */
    void stop();

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
    bool getFreqBins(double* freqBins, size_t len)
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

    /** Task stack size in bytes */
    static const uint32_t               TASK_STACK_SIZE         = 8096U;

    /** MCU core where the task shall run */
    static const BaseType_t             TASK_RUN_CORE           = 0;

    /** Task priority. */
    static const UBaseType_t            TASK_PRIORITY           = 1U;

    /**
     * The sample rate in Hz. According to the Nyquist theorem, it shall be
     * twice as the max. audio frequency, which to support.
     */
    static const uint32_t               SAMPLE_RATE             = 40000;

    /**
     * The number of samples over the spectrum. This shall be always a power of 2!
     */
    static const uint32_t               SAMPLES                 = 1024U;

    /**
     * The number of frequency bins over the spectrum. Note, this is always
     * half of the samples.
     */
    static const uint32_t               FREQ_BINS               = SAMPLES / 2U;

    /**
     * The I2S port, which to use for the audio input.
     */
    static const i2s_port_t             I2S_PORT                = I2S_NUM_0;

    /**
     * I2S event queue size in number of events.
     */
    static const size_t                 I2S_EVENT_QUEUE_SIZE    = 4U;

    /**
     * I2S bits per sample.
     * If you change this, consider to change the sample datatypes at the
     * place where i2s_read() is used.
     * 
     * The INMP441 microphone provides 24-bit sample with MSB first by
     * 32 clock cycles. This means we have to configure here a 32 bit
     * sample and shift it down after its received.
     */
    static const i2s_bits_per_sample_t  I2S_BITS_PER_SAMPLE     = I2S_BITS_PER_SAMPLE_32BIT;

    /**
     * The INMP441 microphone provides 24-bit sample with MSB first by
     * 32 clock cycles. This means we have to configure here a 32 bit
     * sample and shift it down after its received.
     */
    static const uint32_t               I2S_SAMPLE_SHIFT        = 8U;

    /**
     * I2S DMA block size in bytes.
     */
    static const int32_t                DMA_BLOCK_SIZE          = 128;

    /**
     * I2S DMA number of blocks.
     */
    static const int32_t                DMA_BLOCKS              = 4;

    /**
     * Calculated number of samples per DMA block.
     */
    static const uint32_t               SAMPLES_PER_DMA_BLOCK   = DMA_BLOCK_SIZE / (I2S_BITS_PER_SAMPLE / 8);

    /**
     * Calculated the up rounded wait time in ms, till one DMA block is complete.
     */
    static const uint32_t               DMA_BLOCK_TIMEOUT       = ((SAMPLES_PER_DMA_BLOCK * 1000U) + (SAMPLE_RATE / 2U)) / SAMPLE_RATE;

    mutable Mutex               m_mutex;                /**< Mutex used for concurrent access protection. */
    TaskHandle_t                m_taskHandle;           /**< Task handle */
    bool                        m_taskExit;             /**< Flag to signal the task to exit. */
    SemaphoreHandle_t           m_xSemaphore;           /**< Binary semaphore used to signal the task exit. */
    double                      m_real[SAMPLES];        /**< The real values. */
    double                      m_imag[SAMPLES];        /**< The imaginary values. */
    arduinoFFT                  m_fft;                  /**< The FFT algorithm. */
    Queue<i2s_event_type_t>     m_i2sEventQueue;        /**< The I2S event queue, used for rx done notification. */
    uint16_t                    m_sampleWriteIndex;     /**< The current sample write index to the input buffer. */
    double                      m_freqBins[FREQ_BINS];  /**< The frequency bins as result of the FFT, with linear magnitude. */
    bool                        m_freqBinsAreReady;     /**< Are the frequency bins ready for the application? */

    /**
     * Constructs the spectrum analyzer instance.
     */
    SpectrumAnalyzer() :
        m_mutex(),
        m_taskHandle(nullptr),
        m_taskExit(false),
        m_xSemaphore(nullptr),
        m_real{0.0f},
        m_imag{0.0f},
        m_fft(m_real, m_imag, SAMPLES, SAMPLE_RATE),
        m_i2sEventQueue(),
        m_sampleWriteIndex(0U),
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

    SpectrumAnalyzer(const SpectrumAnalyzer& drv);
    SpectrumAnalyzer& operator=(const SpectrumAnalyzer& drv);

    /**
     * Processing task.
     *
     * @param[in]   parameters  Task pParameters
     */
    static void processTask(void* parameters);

    /**
     * Process the main part in the processing task.
     */
    void process();

    /**
     * Setup the I2S driver.
     * 
     * @return If successful, it will return true otherwise false.
     */
    bool initI2S();

    /**
     * De-initialize the I2S driver.
     */
    void deInitI2S();

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

#endif  /* __SPECTRUM_ANALYZER_H__ */

/** @} */
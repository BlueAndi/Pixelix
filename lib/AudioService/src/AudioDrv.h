/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   AudioDrv.h
 * @brief  Audio driver
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup HAL
 *
 * @{
 */

#ifndef AUDIO_DRV_H
#define AUDIO_DRV_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <driver/i2s_std.h>
#include <Mutex.hpp>
#include <Task.hpp>

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
 * The audio observer will be notified for every complete number of available
 * samples.
 */
class IAudioObserver
{
public:

    /**
     * Destroy the audio observer interface.
     */
    virtual ~IAudioObserver()
    {
    }

    /**
     * The audio driver will call this method to notify about a complete available
     * number of samples.
     *
     * @param[in]   data    Audio sample data buffer
     * @param[in]   size    Number of audio samples
     */
    virtual void notify(int32_t* data, size_t size) = 0;

protected:

    /**
     * Construct the audio oberserver interface.
     */
    IAudioObserver()
    {
    }
};

/**
 * The audio driver supports the I2S interface. It will configure the DMA
 * for receicing samples and provides them.
 */
class AudioDrv
{
public:

    /**
     * Get audio driver instance.
     *
     * @return Audio driver instance
     */
    static AudioDrv& getInstance()
    {
        static AudioDrv instance; /* idiom */

        return instance;
    }

    /**
     * Start the audio driver.
     *
     * @return If successful started, it will return true otherwise false.
     */
    bool start();

    /**
     * Stop the audio driver.
     */
    void stop();

    /**
     * Register an audio observer.
     *
     * @param[in] observer The audio observer which to register.
     *
     * @return If successful it will return true otherwise false.
     */
    bool registerObserver(IAudioObserver& observer)
    {
        uint32_t          index        = 0U;
        bool              isSuccessful = false;
        MutexGuard<Mutex> guard(m_mutex);

        while ((index < MAX_OBSERVERS) && (false == isSuccessful))
        {
            if (nullptr == m_observers[index])
            {
                m_observers[index] = &observer;

                isSuccessful       = true;
            }
            else
            {
                ++index;
            }
        }

        return isSuccessful;
    }

    /**
     * Unregister an audio observer.
     *
     * @param[in] observer The audio observer which to unregister.
     */
    void unregisterObserver(IAudioObserver& observer)
    {
        uint32_t          index = 0U;
        MutexGuard<Mutex> guard(m_mutex);

        while (index < MAX_OBSERVERS)
        {
            if (m_observers[index] == (&observer))
            {
                m_observers[index] = nullptr;
            }

            ++index;
        }
    }

    /**
     * The sample rate in Hz. According to the Nyquist theorem, it shall be
     * twice as the max. audio frequency, which to support.
     */
    static const uint32_t SAMPLE_RATE = 14080U;

    /**
     * The number of samples over the spectrum. This shall be always a power of 2!
     */
    static const uint32_t SAMPLES     = 512U;

private:

    /** Task stack size in bytes */
    static const uint32_t TASK_STACK_SIZE                 = 4096U;

    /** MCU core where the task shall run */
    static const BaseType_t TASK_RUN_CORE                 = PRO_CPU_NUM;

    /** Task priority. */
    static const UBaseType_t TASK_PRIORITY                = 1U;

    /**
     * The I2S port, which to use for the audio input.
     * A dedicated port is used on purpose instead of I2S_NUM_AUTO, to keep the
     * port assignment deterministic and to avoid a clash with other I2S users,
     * like e.g. a HUB75 panel driver.
     */
    static const i2s_port_t I2S_PORT                      = I2S_NUM_0;

    /**
     * I2S data bit width per sample.
     * If you change this, consider to change the sample datatypes at the
     * place where i2s_channel_read() is used.
     *
     * The INMP441 microphone provides 24-bit sample with MSB first by
     * 32 clock cycles. This means we have to configure here a 32 bit
     * sample and shift it down after its received.
     */
    static const i2s_data_bit_width_t I2S_BITS_PER_SAMPLE = I2S_DATA_BIT_WIDTH_32BIT;

    /**
     * Calculated number of bytes per sample in the DMA buffer.
     */
    static const uint32_t I2S_BYTES_PER_SAMPLE            = I2S_BITS_PER_SAMPLE / 8U;

    /**
     * The INMP441 microphone provides 24-bit sample with MSB first by
     * 32 clock cycles. This means we have to configure here a 32 bit
     * sample and shift it down after its received.
     */
    static const uint32_t I2S_SAMPLE_SHIFT                = 8U;

    /**
     * I2S DMA block size in bytes.
     * Note, the I2S driver limits one DMA block to max. 4092 bytes.
     */
    static const uint32_t DMA_BLOCK_SIZE                  = 256U;

    /**
     * I2S DMA number of blocks.
     */
    static const uint32_t DMA_BLOCKS                      = 4U;

    /**
     * Calculated number of samples per DMA block. Because the I2S channel is
     * used in mono mode, one frame contains exactly one sample.
     */
    static const uint32_t SAMPLES_PER_DMA_BLOCK           = DMA_BLOCK_SIZE / I2S_BYTES_PER_SAMPLE;

    /**
     * Calculated the up rounded wait time in ms, till one DMA block is complete.
     */
    static const uint32_t DMA_BLOCK_TIMEOUT               = ((SAMPLES_PER_DMA_BLOCK * 1000U) + (SAMPLE_RATE / 2U)) / SAMPLE_RATE;

    /**
     * Max. time in ms to wait for received audio samples. Twice the time a DMA
     * block needs to be filled, to avoid unnecessary timeouts. But still short
     * enough to be able to react on a task stop request.
     */
    static const uint32_t I2S_READ_TIMEOUT                = 2U * DMA_BLOCK_TIMEOUT;

    /**
     * Maximum number of observers which can be registered.
     */
    static const uint32_t MAX_OBSERVERS                   = 3U;

    mutable Mutex         m_mutex;                                 /**< Mutex used for concurrent access protection. */
    Task<AudioDrv>        m_task;                                  /**< The audio driver task, which will process the audio samples. */
    i2s_chan_handle_t     m_i2sRxChannelHandle;                    /**< The I2S RX channel handle, used to receive the audio samples. */
    bool                  m_isMicAvailable;                        /**< Is a microphone as input device available? */
    int32_t               m_dmaBlockBuffer[SAMPLES_PER_DMA_BLOCK]; /**< One DMA block of received samples, read in one go to increase performance. */
    size_t                m_dmaBlockBufferWriteIndex;              /**< The current DMA block buffer write index in byte, used to handle partial reads. */
    int32_t               m_sampleBuffer[SAMPLES];                 /**< Sample buffer */
    uint16_t              m_sampleWriteIndex;                      /**< The current sample write index to the input buffer. */
    IAudioObserver*       m_observers[MAX_OBSERVERS];              /**< A list of registered audio observers. */

    /**
     * Constructs the audio driver instance.
     */
    AudioDrv() :
        m_mutex(),
        m_task("AudioDrvTask", processTask, TASK_STACK_SIZE, TASK_PRIORITY, TASK_RUN_CORE),
        m_i2sRxChannelHandle(nullptr),
        m_isMicAvailable(false),
        m_dmaBlockBuffer(),
        m_dmaBlockBufferWriteIndex(0U),
        m_sampleBuffer(),
        m_sampleWriteIndex(0U),
        m_observers()
    {
    }

    /**
     * Destroys the audio driver instance.
     */
    ~AudioDrv()
    {
        /* Never called. */
    }

    AudioDrv(const AudioDrv& drv);
    AudioDrv& operator=(const AudioDrv& drv);

    /**
     * Processing task.
     *
     * @param[in] self    Pointer to the audio driver instance.
     */
    static void processTask(AudioDrv* self);

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
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* AUDIO_DRV_H */

/** @} */
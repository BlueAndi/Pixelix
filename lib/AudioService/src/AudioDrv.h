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
 * @brief  Audio driver
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup hal
 *
 * @{
 */

#ifndef AUDIO_DRV_H
#define AUDIO_DRV_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <driver/i2s.h>
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
        static AudioDrv instance;   /* idiom */

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
        uint32_t            index           = 0U;
        bool                isSuccessful    = false;
        MutexGuard<Mutex>   guard(m_mutex);

        while((index < MAX_OBSERVERS) && (false == isSuccessful))
        {
            if (nullptr == m_observers[index])
            {
                m_observers[index] = &observer;

                isSuccessful = true;
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
        uint32_t            index           = 0U;
        MutexGuard<Mutex>   guard(m_mutex);

        while(index < MAX_OBSERVERS)
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
    static const uint32_t               SAMPLE_RATE             = 14080U;

    /**
     * The number of samples over the spectrum. This shall be always a power of 2!
     */
    static const uint32_t               SAMPLES                 = 512U;

private:

    /** Task stack size in bytes */
    static const uint32_t               TASK_STACK_SIZE         = 8096U;

    /** MCU core where the task shall run */
    static const BaseType_t             TASK_RUN_CORE           = PRO_CPU_NUM;

    /** Task priority. */
    static const UBaseType_t            TASK_PRIORITY           = 1U;

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
    static const int32_t                DMA_BLOCK_SIZE          = 256;

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

    /**
     * Maximum number of observers which can be registered.
     */
    static const uint32_t               MAX_OBSERVERS           = 3U;

    mutable Mutex       m_mutex;                    /**< Mutex used for concurrent access protection. */
    TaskHandle_t        m_taskHandle;               /**< Task handle */
    bool                m_taskExit;                 /**< Flag to signal the task to exit. */
    SemaphoreHandle_t   m_xSemaphore;               /**< Binary semaphore used to signal the task exit. */
    QueueHandle_t       m_i2sEventQueueHandle;      /**< The I2S event queue handle, used for rx done notification. Note, the queue is created by I2S driver. */
    bool                m_isMicAvailable;           /**< Is a microphone as input device available? */
    int32_t             m_sampleBuffer[SAMPLES];    /**< Sample buffer */
    uint16_t            m_sampleWriteIndex;         /**< The current sample write index to the input buffer. */
    IAudioObserver*     m_observers[MAX_OBSERVERS]; /**< A list of registered audio observers. */

    /**
     * Constructs the audio driver instance.
     */
    AudioDrv() :
        m_mutex(),
        m_taskHandle(nullptr),
        m_taskExit(false),
        m_xSemaphore(nullptr),
        m_i2sEventQueueHandle(nullptr),
        m_isMicAvailable(false),
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
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* AUDIO_DRV_H */

/** @} */
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
 * @file   AudioDrv.cpp
 * @brief  Audio driver
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "AudioDrv.h"

#include <Logging.h>
#include <Board.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

#ifdef CONFIG_IDF_TARGET_ESP32

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(2, 0, 4)

/**
 * Only the left channel is supported.
 * Workaround, see https://github.com/espressif/arduino-esp32/issues/7177
 */
#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_RIGHT

#else

/**
 * Only the left channel is supported.
 */
#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT

#endif

#else

/**
 * Only the left channel is supported.
 */
#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT

#endif

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

bool AudioDrv::start()
{
    bool isSuccessful = true;

    if (false == m_task.isRunning())
    {
        if (false == m_mutex.create())
        {
            isSuccessful = false;
        }
        else
        {
            /* Clear sample write index before the task will start.
             * Otherwise it may happen that the first buffer of the
             * observers will be filled partly.
             */
            m_sampleWriteIndex = 0U;

            /* Initialize I2S first to get a valid queue handle, which
             * the task will use to receive I2S events.
             */
            if (false == initI2S())
            {
                isSuccessful = false;
            }
            else if (false == m_task.start(this))
            {
                isSuccessful = false;
            }
            else
            {
                ;
            }
        }

        /* Any error happened? */
        if (false == isSuccessful)
        {
            (void)m_task.stop();
            m_mutex.destroy();
        }
        else
        {
            LOG_INFO("Audio driver task is up.");
        }
    }

    return isSuccessful;
}

void AudioDrv::stop()
{
    if (true == m_task.isRunning())
    {
        (void)m_task.stop();

        LOG_INFO("Audio driver task is down.");

        deInitI2S();
        m_mutex.destroy();
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void AudioDrv::processTask(AudioDrv* self)
{
    self->process();
}

void AudioDrv::process()
{
    i2s_event_t i2sEvt;

    /* Handle all ready DMA blocks. */
    while (pdPASS == xQueueReceive(m_i2sEventQueueHandle, &i2sEvt, DMA_BLOCK_TIMEOUT * portTICK_PERIOD_MS))
    {
        /* Any DMA error? */
        if (I2S_EVENT_DMA_ERROR == i2sEvt.type)
        {
            LOG_WARNING("DMA error");
        }
        /* One DMA block finished? */
        else if (I2S_EVENT_RX_DONE == i2sEvt.type)
        {
            uint16_t          sampleIdx = 0U;
            MutexGuard<Mutex> guard(m_mutex);

            /* Read the whole DMA block. */
            for (sampleIdx = 0U; sampleIdx < SAMPLES_PER_DMA_BLOCK; ++sampleIdx)
            {
                int32_t sample    = 0U; /* Attention, this datatype must correlate to the configuration, see bits per sample! */
                size_t  bytesRead = 0;

                (void)i2s_read(I2S_PORT, &sample, sizeof(sample), &bytesRead, portMAX_DELAY);

                if (sizeof(sample) == bytesRead)
                {
                    /* Down shift to get the real value. */
                    sample                             >>= I2S_SAMPLE_SHIFT;

                    m_sampleBuffer[m_sampleWriteIndex]   = sample;
                    ++m_sampleWriteIndex;

                    /* Check for ext. microphone */
                    if (false == m_isMicAvailable)
                    {
                        if (0 != sample)
                        {
                            m_isMicAvailable = true;
                        }
                    }

                    /* All samples read? */
                    if (SAMPLES <= m_sampleWriteIndex)
                    {
                        uint32_t observerIndex = 0U;

                        m_sampleWriteIndex     = 0U;

                        while (observerIndex < MAX_OBSERVERS)
                        {
                            IAudioObserver* observer = m_observers[observerIndex];

                            if (nullptr != observer)
                            {
                                observer->notify(m_sampleBuffer, SAMPLES);
                            }

                            ++observerIndex;
                        }
                    }
                }
            }
        }
        else
        {
            /* Should never happen. */
            ;
        }
    }
}

bool AudioDrv::initI2S()
{
    bool         isSuccessful = false;
    esp_err_t    i2sRet       = ESP_OK;
    i2s_config_t i2sConfig    = {
           .mode                 = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_RX),
           .sample_rate          = SAMPLE_RATE,
           .bits_per_sample      = I2S_BITS_PER_SAMPLE,
           .channel_format       = I2S_MIC_CHANNEL,           /* It is assumed, that the I2S device supports the left audio channel only. */
           .communication_format = I2S_COMM_FORMAT_STAND_I2S, /* I2S_COMM_FORMAT_I2S is necessary for Philips Standard format. */
           .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
           .dma_buf_count        = DMA_BLOCKS,
           .dma_buf_len          = DMA_BLOCK_SIZE,
           .use_apll             = false, /* Higher accuracy with APLL is not necessary. */
           .tx_desc_auto_clear   = false, /* In underflow condition, the tx descriptor shall not be cleared automatically. */
           .fixed_mclk           = 0,     /* No fixed MCLK output. */
           .mclk_multiple        = I2S_MCLK_MULTIPLE_DEFAULT,
           .bits_per_chan        = I2S_BITS_PER_CHAN_DEFAULT
    };
    i2s_pin_config_t pinConfig = {
        .mck_io_num   = I2S_PIN_NO_CHANGE,
        .bck_io_num   = Board::Pin::i2sSerialClock,
        .ws_io_num    = Board::Pin::i2sWordSelect,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num  = Board::Pin::i2sSerialDataIn
    };

    i2sRet = i2s_driver_install(I2S_PORT, &i2sConfig, I2S_EVENT_QUEUE_SIZE, &m_i2sEventQueueHandle);

    if (ESP_OK != i2sRet)
    {
        LOG_ERROR("Failed to install I2S driver: %d", i2sRet);
    }
    else
    {
        i2sRet = i2s_set_pin(I2S_PORT, &pinConfig);

        if (ESP_OK != i2sRet)
        {
            LOG_ERROR("Failed set I2S pins: %d", i2sRet);

            (void)i2s_driver_uninstall(I2S_PORT);
        }
        else
        {
            isSuccessful = true;
        }
    }

    return isSuccessful;
}

void AudioDrv::deInitI2S()
{
    (void)i2s_driver_uninstall(I2S_PORT);
    m_i2sEventQueueHandle = nullptr;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

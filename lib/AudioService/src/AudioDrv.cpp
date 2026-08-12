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
            /* Clear the write indizes before the task will start.
             * Otherwise it may happen that the first buffer of the
             * observers will be filled partly.
             */
            m_dmaBlockBufferWriteIndex = 0U;
            m_sampleWriteIndex         = 0U;

            /* Initialize I2S first, the task will only read from the
             * already enabled I2S RX channel.
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
            deInitI2S();
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
    if (nullptr != self)
    {
        self->process();
    }
    else
    {
        LOG_ERROR("Audio driver task instance is invalid.");
    }
}

void AudioDrv::process()
{
    /* The DMA block buffer is read as byte stream, because a single read may
     * be interrupted by a timeout and continued with the next read.
     * Note, the buffer itself is a 32-bit array to ensure the alignment,
     * which is required to access the samples.
     */
    void*     vDmaBlockBuffer = m_dmaBlockBuffer;
    uint8_t*  dmaBlockBuffer  = static_cast<uint8_t*>(vDmaBlockBuffer);
    size_t    bytesToRead     = sizeof(m_dmaBlockBuffer) - m_dmaBlockBufferWriteIndex;
    size_t    bytesRead       = 0U;
    esp_err_t i2sRet          = i2s_channel_read(m_i2sRxChannelHandle, &dmaBlockBuffer[m_dmaBlockBufferWriteIndex], bytesToRead, &bytesRead, I2S_READ_TIMEOUT);

    /* Any error, except a timeout? A timeout is not critical, because the
     * number of read bytes is still valid and the rest will be read next time.
     */
    if ((ESP_OK != i2sRet) &&
        (ESP_ERR_TIMEOUT != i2sRet))
    {
        LOG_WARNING("Failed to read I2S samples: %s", esp_err_to_name(i2sRet));
    }

    m_dmaBlockBufferWriteIndex += bytesRead;

    /* One complete DMA block received? */
    if (sizeof(m_dmaBlockBuffer) <= m_dmaBlockBufferWriteIndex)
    {
        uint32_t          sampleIdx = 0U;
        MutexGuard<Mutex> guard(m_mutex);

        m_dmaBlockBufferWriteIndex = 0U;

        /* Process the whole DMA block. */
        for (sampleIdx = 0U; sampleIdx < SAMPLES_PER_DMA_BLOCK; ++sampleIdx)
        {
            /* Down shift to get the real value. */
            int32_t sample                     = m_dmaBlockBuffer[sampleIdx] >> I2S_SAMPLE_SHIFT;

            m_sampleBuffer[m_sampleWriteIndex] = sample;
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

bool AudioDrv::initI2S()
{
    bool              isSuccessful  = false;
    esp_err_t         i2sRet        = ESP_OK;
    i2s_chan_config_t i2sChanConfig = I2S_CHANNEL_DEFAULT_CONFIG(I2S_PORT, I2S_ROLE_MASTER);

    i2sChanConfig.dma_desc_num      = DMA_BLOCKS;
    i2sChanConfig.dma_frame_num     = SAMPLES_PER_DMA_BLOCK;

    /* Only a RX channel is required, because the audio driver just records. */
    i2sRet                          = i2s_new_channel(&i2sChanConfig, nullptr, &m_i2sRxChannelHandle);

    if (ESP_OK != i2sRet)
    {
        LOG_ERROR("Failed to allocate I2S RX channel: %s", esp_err_to_name(i2sRet));
    }
    else
    {
        /* It is assumed, that the I2S device (microphone) provides one audio
         * channel only, therefore the channel is used in mono mode with the
         * default slot mask, which selects the left slot.
         *
         * Note, the former ESP32 specific "only right" channel format
         * workaround (see https://github.com/espressif/arduino-esp32/issues/7177)
         * is not required anymore. For the left slot in mono mode the I2S HAL
         * selects the same hardware channel, as the legacy I2S driver did with
         * the "only right" channel format.
         */
        i2s_std_config_t i2sStdConfig = {
            .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
            .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_BITS_PER_SAMPLE, I2S_SLOT_MODE_MONO),
            .gpio_cfg = {
                .mclk         = I2S_GPIO_UNUSED, /* No master clock output required. */
                .bclk         = static_cast<gpio_num_t>(Board::Pin::i2sSerialClock),
                .ws           = static_cast<gpio_num_t>(Board::Pin::i2sWordSelect),
                .dout         = I2S_GPIO_UNUSED, /* RX channel only. */
                .din          = static_cast<gpio_num_t>(Board::Pin::i2sSerialDataIn),
                .invert_flags = {
                    .mclk_inv = 0U, /* Do not invert the MCLK output. */
                    .bclk_inv = 0U, /* Do not invert the BCLK input/output. */
                    .ws_inv   = 0U  /* Do not invert the WS input/output. */
                } }
        };

        i2sRet = i2s_channel_init_std_mode(m_i2sRxChannelHandle, &i2sStdConfig);

        if (ESP_OK != i2sRet)
        {
            LOG_ERROR("Failed to initialize I2S RX channel in standard mode: %s", esp_err_to_name(i2sRet));
        }
        else
        {
            i2sRet = i2s_channel_enable(m_i2sRxChannelHandle);

            if (ESP_OK != i2sRet)
            {
                LOG_ERROR("Failed to enable I2S RX channel: %s", esp_err_to_name(i2sRet));
            }
            else
            {
                isSuccessful = true;
            }
        }

        /* Release the channel in case of any error. */
        if (false == isSuccessful)
        {
            deInitI2S();
        }
    }

    return isSuccessful;
}

void AudioDrv::deInitI2S()
{
    if (nullptr != m_i2sRxChannelHandle)
    {
        /* Disabling is only necessary if the channel is running, but its
         * sufficient to ignore the result in the other cases.
         */
        (void)i2s_channel_disable(m_i2sRxChannelHandle);
        (void)i2s_del_channel(m_i2sRxChannelHandle);

        m_i2sRxChannelHandle = nullptr;
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

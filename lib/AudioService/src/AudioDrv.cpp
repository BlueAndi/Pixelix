/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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

    if (nullptr == m_taskHandle)
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

            /* Create binary semaphore to signal task exit. */
            m_xSemaphore = xSemaphoreCreateBinary();

            if (nullptr == m_xSemaphore)
            {
                isSuccessful = false;
            }
            else
            {
                BaseType_t  osRet   = pdFAIL;

                /* Task shall run */
                m_taskExit = false;

                osRet = xTaskCreateUniversal(   processTask,
                                                "audioDrvTask",
                                                TASK_STACK_SIZE,
                                                this,
                                                TASK_PRIORITY,
                                                &m_taskHandle,
                                                TASK_RUN_CORE);

                /* Task creation failed? */
                if (pdPASS != osRet)
                {
                    isSuccessful = false;
                }
                else
                {
                    (void)xSemaphoreGive(m_xSemaphore);
                }
            }
        }

        /* Any error happened? */
        if (false == isSuccessful)
        {
            if (nullptr != m_xSemaphore)
            {
                vSemaphoreDelete(m_xSemaphore);
                m_xSemaphore = nullptr;
            }

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
    if (nullptr != m_taskHandle)
    {
        m_taskExit = true;

        /* Join */
        (void)xSemaphoreTake(m_xSemaphore, portMAX_DELAY);

        LOG_INFO("Audio driver task is down.");

        vSemaphoreDelete(m_xSemaphore);
        m_xSemaphore = nullptr;

        m_mutex.destroy();

        m_taskHandle = nullptr;
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void AudioDrv::processTask(void* parameters)
{
    AudioDrv* tthis = static_cast<AudioDrv*>(parameters);

    if ((nullptr != tthis) &&
        (nullptr != tthis->m_xSemaphore))
    {
        (void)xSemaphoreTake(tthis->m_xSemaphore, portMAX_DELAY);

        if (true == tthis->initI2S())
        {
            LOG_INFO("I2S channel allocated.");

            while(false == tthis->m_taskExit)
            {
                tthis->process();
            }

            tthis->deInitI2S();

            LOG_INFO("I2S channel released.");
        }
        else
        {
            LOG_ERROR("I2S initialization failed, shutdown audio task.");
        }

        (void)xSemaphoreGive(tthis->m_xSemaphore);
    }

    vTaskDelete(nullptr);
}

void AudioDrv::process()
{
    size_t      bytesRead           = 0U;
    uint8_t*    dmaBlockBufferAddr  = &m_dmaBlockBuffer[m_dmaBlockBufferWriteIndex];
    size_t      dmaBlockBufferSize  = sizeof(m_dmaBlockBuffer) - m_dmaBlockBufferWriteIndex;

    if (ESP_OK == i2s_channel_read(m_i2sRxChannelHandle, dmaBlockBufferAddr, dmaBlockBufferSize, &bytesRead, DMA_BLOCK_TIMEOUT))
    {
        m_dmaBlockBufferWriteIndex += bytesRead;

        /* One DMA block read? */
        if (sizeof(m_dmaBlockBuffer) <= m_dmaBlockBufferWriteIndex)
        {
            uint16_t            sampleIdx;
            void*               vDmaBlockBuffer = static_cast<void*>(m_dmaBlockBuffer);
            int32_t*            samples         = static_cast<int32_t*>(vDmaBlockBuffer);
            MutexGuard<Mutex>   guard(m_mutex);

            for(sampleIdx = 0U; sampleIdx < SAMPLES_PER_DMA_BLOCK; ++sampleIdx)
            {
                /* Down shift to get the real value. */
                int32_t sample24bit = samples[sampleIdx] >> I2S_SAMPLE_SHIFT;

                m_sampleBuffer[m_sampleWriteIndex] = sample24bit;
                ++m_sampleWriteIndex;

                /* Check for ext. microphone */
                if (false == m_isMicAvailable)
                {
                    if (0 != sample24bit)
                    {
                        m_isMicAvailable = true;
                    }
                }

                /* All samples read? */
                if (SAMPLES <= m_sampleWriteIndex)
                {
                    uint32_t observerIndex = 0U;

                    m_sampleWriteIndex = 0U;

                    while(observerIndex < MAX_OBSERVERS)
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

            m_dmaBlockBufferWriteIndex = 0U;
        }
    }
}

bool AudioDrv::initI2S()
{
    bool                isSuccessful    = false;
    esp_err_t           i2sRet          = ESP_OK;
    i2s_chan_config_t   i2sChanConfig   =
    {
        I2S_NUM_AUTO,
        I2S_ROLE_MASTER,
        DMA_BLOCKS,
        DMA_BLOCK_SIZE,
        false /* Automatic clearing of DMA tx buffer not necessary, because only used for receiving. */
    };

    i2sRet = i2s_new_channel(&i2sChanConfig, nullptr, &m_i2sRxChannelHandle);

    if (ESP_OK != i2sRet)
    {
        LOG_ERROR("Failed to allocate I2S channel: %s", esp_err_to_name(i2sRet));
    }
    else
    {
        i2s_std_config_t    i2sStdConfig    =
        {
            I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
            I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_BITS_PER_SAMPLE, I2S_SLOT_MODE_MONO),
            {
                I2S_GPIO_UNUSED,
                static_cast<gpio_num_t>(Board::Pin::i2sSerialClock),
                static_cast<gpio_num_t>(Board::Pin::i2sWordSelect),
                I2S_GPIO_UNUSED,
                static_cast<gpio_num_t>(Board::Pin::i2sSerialDataIn),
                {
                    0, /* Do not invert MCLK output. */
                    0, /* Do not invert BCLK input/output. */
                    0 /* Do not invert the WS input/output. */
                }
            }
        };

        /* Default is only receiving left slot in mono mode,
         * therefore update here for using the right slot.
         */
        /* i2sStdConfig.slot_cfg.slot_mask = I2S_STD_SLOT_RIGHT; */

        i2sRet = i2s_channel_init_std_mode(m_i2sRxChannelHandle, &i2sStdConfig);

        if (ESP_OK != i2sRet)
        {
            LOG_ERROR("Failed to initialize I2S channel to standard mode.: %s", esp_err_to_name(i2sRet));
            deInitI2S();
        }
        else if (ESP_OK != i2s_channel_enable(m_i2sRxChannelHandle))
        {
            deInitI2S();
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
    if (nullptr != m_i2sRxChannelHandle)
    {
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

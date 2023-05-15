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
 * @brief  Audio service
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "AudioService.h"
#include "AudioDrv.h"

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

bool AudioService::start()
{
    bool        isSuccessful    = true;
    AudioDrv&   audioDrv        = AudioDrv::getInstance();

    if (false == audioDrv.start())
    {
        LOG_ERROR("Couldn't start the audio driver.");
        isSuccessful = false;
    }
    else
    {
        if (false == audioDrv.registerObserver(m_spectrumAnalyzer))
        {
            LOG_ERROR("Couldn't register spectrum analyzer.");
            isSuccessful = false;
        }
        else
        {
            uint8_t idx = 0U;
            
            while(MAX_TONE_DETECTORS > idx)
            {
                if (false == audioDrv.registerObserver(m_audioToneDetector[idx]))
                {
                    LOG_ERROR("Couldn't register audio tone detector (%u).", idx);
                    isSuccessful = false;
                }

                ++idx;
            }
        }

        if (false == isSuccessful)
        {
            stop();
        }
        else
        {
            LOG_INFO("Audio service started.");
        }
    }

    return isSuccessful;
}

void AudioService::stop()
{
    AudioDrv&   audioDrv    = AudioDrv::getInstance();
    uint8_t     idx         = 0U;

    audioDrv.unregisterObserver(m_spectrumAnalyzer);

    while(MAX_TONE_DETECTORS > idx)
    {
        audioDrv.unregisterObserver(m_audioToneDetector[idx]);

        ++idx;
    }

    AudioDrv::getInstance().stop();

    LOG_INFO("Audio service stopped.");
}

void AudioService::process()
{
    /* Nothing to do. */
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

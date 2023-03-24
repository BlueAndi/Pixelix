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
 * 
 * @addtogroup audio_service
 *
 * @{
 */

#ifndef AUDIO_SERVICE_H
#define AUDIO_SERVICE_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IService.hpp>
#include "AudioDrv.h"
#include "SpectrumAnalyzer.h"
#include "AudioToneDetector.h"

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
 * The audio service provides different audio related functionality.
 */
class AudioService : public IService
{
public:

    /**
     * Get the audio service instance.
     * 
     * @return Audio service instance
     */
    static AudioService& getInstance()
    {
        static AudioService instance; /* idiom */

        return instance;
    }

    /**
     * Start the audio service.
     */
    bool start() final;

    /**
     * Stop the audio service.
     */
    void stop() final;

    /**
     * Process the service.
     */
    void process() final;

    /**
     * Get the spectrum analyzer.
     * 
     * @return Spectrum analyzer instance otherwise nullptr
     */
    SpectrumAnalyzer* getSpectrumAnalyzer()
    {
        return &m_spectrumAnalyzer;
    }

    /**
     * Get the audio tone detector.
     * 
     * @param[in] id    Tone detector id
     * 
     * @return Tone detector instance otherwise nullptr
     */
    AudioToneDetector* getAudioToneDetector(uint8_t id)
    {
        AudioToneDetector* instance = nullptr;

        if (MAX_TONE_DETECTORS > id)
        {
            instance = &m_audioToneDetector[id];
        }

        return instance;
    }

    /**
     * The max. number of tone detectors, which the service
     * can provide.
     */
    static const uint8_t    MAX_TONE_DETECTORS  = 2U;

private:

    SpectrumAnalyzer    m_spectrumAnalyzer;
    AudioToneDetector   m_audioToneDetector[MAX_TONE_DETECTORS];

    AudioService(const AudioService& drv);
    AudioService& operator=(const AudioService& drv);

    /**
     * Constructs the audio service instance.
     */
    AudioService() :
        IService(),
        m_spectrumAnalyzer(),
        m_audioToneDetector()
    {
    }

    /**
     * Destroys the audio service instance.
     */
    ~AudioService()
    {
        /* Never called. */
    }
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* AUDIO_SERVICE_H */

/** @} */
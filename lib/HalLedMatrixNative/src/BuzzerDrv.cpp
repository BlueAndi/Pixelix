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
 * @file   BuzzerDrv.cpp
 * @brief  Buzzer driver for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "BuzzerDrv.h"

#include <SDL3/SDL.h>
#include <math.h>
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

static void SDLCALL audioCallback(void* userdata, SDL_AudioStream* stream, int additionalAmount, int totalAmount);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Amplitude which a melody at the full volume is played with.
 *
 * It is tuned by ear: loud enough to be clearly audible on a desktop at a
 * normal system volume, quiet enough to not be painful.
 */
static const float MASTER_GAIN          = 0.2F;

/** Number of samples the generator produces per call.
 *
 * The audio callback fills its buffer in chunks of this size, so the buffer is
 * a fixed size array on the stack instead of an allocation per callback.
 */
static const size_t AUDIO_CHUNK_SAMPLES = 512U;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

BuzzerDrv::BuzzerDrv() :
    IBuzzerDrv(),
    m_stream(nullptr),
    m_audioState(AUDIO_STATE_CLOSED),
    m_frequency(0.0F),
    m_amplitude(0.0F),
    m_isMuted(false),
    m_dutyCycle(FULL_SCALE_DUTY_CYCLE),
    m_phase(0.0F)
{
}

BuzzerDrv::~BuzzerDrv()
{
    if (AUDIO_STATE_OPEN == m_audioState)
    {
        if (nullptr != m_stream)
        {
            SDL_DestroyAudioStream(m_stream);
            m_stream = nullptr;
        }

        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        m_audioState = AUDIO_STATE_CLOSED;
    }
}

void BuzzerDrv::init(const DOutPin& buzzerOut)
{
    /* There is no buzzer pin on the host.
     *
     * The audio device is not opened here by intention. Board::init() runs in
     * every start of the native application, also in a CI pipeline and in a
     * session which only uses the webinterface. Opening the default playback
     * device here would claim it, and show the application in the mixer of the
     * operating system, for runs which never produce a single tone.
     */
    (void)buzzerOut;
}

void BuzzerDrv::stop()
{
    m_frequency = 0.0F;
    m_amplitude = 0.0F;
}

void BuzzerDrv::play(uint32_t freq)
{
    /* The first audible tone opens the audio device. A frequency of 0 means
     * silence and a duty cycle of 0 means no amplitude, so neither is worth
     * claiming the playback device of the host for. The melody service asks for
     * a duty cycle of 0 whenever a melody is played with volume 0.
     */
    if ((0U < freq) && (0U < m_dutyCycle) && (AUDIO_STATE_CLOSED == m_audioState))
    {
        (void)openAudio();
    }

    m_frequency = static_cast<float>(freq);

    updateAmplitude();
}

void BuzzerDrv::play(uint32_t freq, uint16_t dc)
{
    m_dutyCycle = dc;

    play(freq);
}

void BuzzerDrv::changeDutyCycle(uint16_t dc)
{
    /* This never opens the audio device. Without a frequency there is nothing
     * to play, and the duty cycle alone does not make a tone.
     */
    m_dutyCycle = dc;

    updateAmplitude();
}

void BuzzerDrv::setMuted(bool muted)
{
    m_isMuted = muted;
}

bool BuzzerDrv::isMuted() const
{
    return m_isMuted;
}

bool BuzzerDrv::isAudioAvailable() const
{
    return (AUDIO_STATE_FAILED != m_audioState);
}

void BuzzerDrv::generate(float* buffer, size_t numSamples)
{
    if (nullptr == buffer)
    {
        /* Guard path: no destination to fill. */
    }
    else
    {
        /* Read once per buffer and not per sample. A tone lasts at least a few
         * milliseconds, therefore a change which lands one buffer later is
         * inaudible.
         */
        const float freq      = m_frequency;
        const float amplitude = m_amplitude;
        const bool  isMuted   = m_isMuted;
        size_t      idx       = 0U;

        if ((true == isMuted) || (0.0F >= freq) || (0.0F >= amplitude))
        {
            while (numSamples > idx)
            {
                buffer[idx] = 0.0F;

                ++idx;
            }

            /* The next tone starts at a zero crossing, which avoids a click. */
            m_phase = 0.0F;
        }
        else
        {
            const float phaseInc = freq / static_cast<float>(SAMPLE_RATE);

            while (numSamples > idx)
            {
                /* A square wave: the first half of the period is high, the
                 * second one is low. This is the signal shape of the pin which
                 * drives the piezo on a real board.
                 */
                buffer[idx] = (0.5F > m_phase) ? amplitude : -amplitude;

                /* fmodf() wraps in a single bounded operation, also for a
                 * frequency above the sample rate.
                 */
                m_phase     = fmodf(m_phase + phaseInc, 1.0F);

                ++idx;
            }
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool BuzzerDrv::openAudio()
{
    bool isSuccessful = false;

    if (false == SDL_InitSubSystem(SDL_INIT_AUDIO))
    {
        /* The host may have no audio device at all, for example in a pipeline
         * or in a ssh session. That is not a reason to fail the application, it
         * just stays silent. The state prevents a retry and a second log entry.
         */
        LOG_WARNING("SDL_InitSubSystem(SDL_INIT_AUDIO) failed: %s", SDL_GetError());
        m_audioState = AUDIO_STATE_FAILED;
    }
    else
    {
        SDL_AudioSpec spec;

        spec.format   = SDL_AUDIO_F32;
        spec.channels = 1;
        spec.freq     = SAMPLE_RATE;

        m_stream      = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, audioCallback, this);

        if (nullptr == m_stream)
        {
            LOG_WARNING("SDL_OpenAudioDeviceStream failed: %s", SDL_GetError());

            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            m_audioState = AUDIO_STATE_FAILED;
        }
        else
        {
            (void)SDL_ResumeAudioStreamDevice(m_stream);

            m_audioState = AUDIO_STATE_OPEN;
            isSuccessful = true;
        }
    }

    return isSuccessful;
}

void BuzzerDrv::updateAmplitude()
{
    /* A duty cycle above the full scale value stays at the maximum amplitude
     * instead of clipping. The interface allows up to 1023 digits, the melody
     * service never asks for more than FULL_SCALE_DUTY_CYCLE.
     */
    const uint16_t dutyCycle = (FULL_SCALE_DUTY_CYCLE < m_dutyCycle) ? FULL_SCALE_DUTY_CYCLE : m_dutyCycle;

    m_amplitude              = MASTER_GAIN * static_cast<float>(dutyCycle) / static_cast<float>(FULL_SCALE_DUTY_CYCLE);
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Called by SDL in its audio thread to request further samples.
 *
 * @param[in] userdata          The buzzer driver instance.
 * @param[in] stream            The audio stream to put the samples into.
 * @param[in] additionalAmount  Requested amount of data in byte.
 * @param[in] totalAmount       Total amount of data in byte. Unused.
 */
static void SDLCALL audioCallback(void* userdata, SDL_AudioStream* stream, int additionalAmount, int totalAmount)
{
    BuzzerDrv* buzzerDrv = static_cast<BuzzerDrv*>(userdata);

    (void)totalAmount;

    if ((nullptr == buzzerDrv) || (nullptr == stream) || (0 >= additionalAmount))
    {
        /* Guard path: nothing to generate. */
    }
    else
    {
        float  buffer[AUDIO_CHUNK_SAMPLES];
        size_t remaining = static_cast<size_t>(additionalAmount) / sizeof(float);

        while (0U < remaining)
        {
            const size_t chunkSize = (AUDIO_CHUNK_SAMPLES < remaining) ? AUDIO_CHUNK_SAMPLES : remaining;

            buzzerDrv->generate(buffer, chunkSize);
            (void)SDL_PutAudioStreamData(stream, buffer, static_cast<int>(chunkSize * sizeof(float)));

            remaining -= chunkSize;
        }
    }
}

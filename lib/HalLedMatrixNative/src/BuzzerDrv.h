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
 * @file   BuzzerDrv.h
 * @brief  Buzzer driver for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * There is no buzzer on the host, therefore the tones are played through the
 * audio device of the host instead.
 *
 * @addtogroup HAL
 *
 * @{
 */

#ifndef BUZZER_DRV_H
#define BUZZER_DRV_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <atomic>
#include <IBuzzerDrv.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Opaque SDL audio stream.
 *
 * The SDL header is deliberately not included here. This header is pulled in by
 * Board.h and would carry SDL into the include chain of the whole application.
 * DisplayDrv.h keeps SDL out the same way, by forward declaring LedGridSim.
 */
struct SDL_AudioStream;

/**
 * Buzzer driver for the native environment.
 *
 * A piezo buzzer is driven by a PWM pin, where the frequency sets the pitch and
 * the duty cycle sets how loud it is. On the host the same signal shape is
 * generated as a square wave and played through the audio device.
 *
 * The audio device is opened with the first tone and not before, see init().
 *
 * m_frequency, m_amplitude, m_audioState and m_isMuted are atomic, because they
 * are written by the service task and by the user interface task, while the SDL
 * audio thread reads them. A mutex is deliberately avoided, because taking one
 * in an audio callback is the classic cause of dropouts. There is nothing to
 * protect atomically across these values either: a tone whose frequency lands
 * one buffer before its amplitude is inaudible at these durations.
 */
class BuzzerDrv : public IBuzzerDrv
{
public:

    /**
     * Constructs the buzzer driver.
     */
    BuzzerDrv();

    /**
     * Destroys the buzzer driver. It closes the audio device.
     */
    virtual ~BuzzerDrv();

    /**
     * Initialize the buzzer driver.
     *
     * The pin is ignored, because there is no hardware on the host. The audio
     * device is not opened here by intention, see the class description.
     *
     * @param[in] buzzerOut Pin of the buzzer. Unused.
     */
    void init(const DOutPin& buzzerOut) final;

    /**
     * Stop the buzzer. The audio device stays open.
     */
    void stop() final;

    /**
     * Play a tone with the given frequency. The last duty cycle is used.
     *
     * @param[in] freq  Frequency in Hz. 0 means silence.
     */
    void play(uint32_t freq) final;

    /**
     * Play a tone with the given frequency and duty cycle.
     *
     * @param[in] freq  Frequency in Hz. 0 means silence.
     * @param[in] dc    Duty cycle in digits [0; 1023].
     */
    void play(uint32_t freq, uint16_t dc) final;

    /**
     * Change the duty cycle of the currently played tone.
     *
     * @param[in] dc    Duty cycle in digits [0; 1023].
     */
    void changeDutyCycle(uint16_t dc) final;

    /**
     * Mute or unmute the output.
     *
     * It takes effect immediately, also for a tone which is already playing.
     * Muting never opens the audio device.
     *
     * @param[in] muted If true, the output is muted otherwise not.
     */
    void setMuted(bool muted);

    /**
     * Is the output muted?
     *
     * @return If the output is muted, it will return true otherwise false.
     */
    bool isMuted() const;

    /**
     * Is the audio device usable?
     *
     * It is only false after an attempt to open it failed. As long as no tone
     * was played yet, no attempt was made and it will return true.
     *
     * @return If the audio device is usable, it will return true otherwise false.
     */
    bool isAudioAvailable() const;

    /**
     * Generate the next audio samples.
     *
     * Public only, because the SDL audio callback is a free function in the
     * implementation file. It is not part of the driver interface and must not
     * be called from anywhere else.
     *
     * @param[out] buffer       Destination buffer.
     * @param[in]  numSamples   Number of samples to generate.
     */
    void generate(float* buffer, size_t numSamples);

private:

    /** Duty cycle at which the host plays with the full amplitude.
     *
     * It mirrors RTTTL::MAX_DUTY_CYCLE by value. The interface documents the
     * duty cycle as [0; 1023], but the melody service never asks for more than
     * this, because a higher value sounds strange on a real piezo. Normalizing
     * against 1023 would play a melody at the full volume with 12 % amplitude,
     * which is close to inaudible on a desktop.
     *
     * The HAL must not depend on a service, therefore the value is repeated
     * here instead of being included. If the melody service ever changes its
     * maximum, the host gets louder or quieter than the board.
     */
    static const uint16_t FULL_SCALE_DUTY_CYCLE = 125U;

    /** Sample rate of the generated audio in Hz. */
    static const int32_t SAMPLE_RATE            = 44100;

    /** State of the lazily opened audio device. */
    enum AudioState
    {
        AUDIO_STATE_CLOSED = 0, /**< Not opened yet. */
        AUDIO_STATE_OPEN,       /**< Opened and usable. */
        AUDIO_STATE_FAILED      /**< Opening failed. No retry and no further log. */
    };

    SDL_AudioStream*        m_stream;     /**< SDL audio stream or nullptr. */
    std::atomic<AudioState> m_audioState; /**< State of the audio device. */
    std::atomic<float>      m_frequency;  /**< Tone frequency in Hz. 0 means silence. */
    std::atomic<float>      m_amplitude;  /**< Tone amplitude [0.0; 1.0]. */
    std::atomic<bool>       m_isMuted;    /**< Mute state, set by the user interface. */
    uint16_t                m_dutyCycle;  /**< Last duty cycle in digits. */
    float                   m_phase;      /**< Generator phase [0.0; 1.0). Audio thread only! */

    /**
     * Open the audio device.
     *
     * It is only called if the audio state is AUDIO_STATE_CLOSED. On failure the
     * state becomes AUDIO_STATE_FAILED, which prevents any retry.
     *
     * @return If successful opened, it will return true otherwise false.
     */
    bool openAudio();

    /**
     * Derive the amplitude from the current duty cycle.
     */
    void updateAmplitude();

    BuzzerDrv(const BuzzerDrv& drv);
    BuzzerDrv& operator=(const BuzzerDrv& drv);
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* BUZZER_DRV_H */

/** @} */

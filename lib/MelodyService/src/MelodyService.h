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
 * @file   MelodyService.h
 * @brief  Melody service
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup MELODY_SERVICE
 *
 * @{
 */

#ifndef MELODY_SERVICE_H
#define MELODY_SERVICE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "RTTTL.h"

#include <stdint.h>
#include <IService.hpp>
#include <IBuzzerDrv.h>
#include <WString.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The melody service plays a single melody in RTTTL format on the buzzer.
 *
 * It has no queue by intention. A new melody replaces the running one
 * immediately, because a melody belongs to a event which happens now.
 *
 * The service owns the melody string over the whole playback. This is required,
 * because the player references the melody instead of copying it.
 */
class MelodyService : public IService
{
public:

    /**
     * Get the melody service instance.
     *
     * @return Melody service instance
     */
    static MelodyService& getInstance()
    {
        static MelodyService instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
    }

    /**
     * Start the service. It uses the buzzer of the board.
     *
     * @return If successful started, it will return true otherwise false.
     */
    bool start() final;

    /**
     * Start the service with a specific buzzer driver.
     *
     * This is the seam which makes the service testable without a board.
     *
     * @param[in] buzzerDrv The buzzer driver. It must exist over the service lifetime.
     *
     * @return If successful started, it will return true otherwise false.
     */
    bool startWithBuzzer(IBuzzerDrv& buzzerDrv);

    /**
     * Stop the service. A running melody will be stopped.
     */
    void stop() final;

    /**
     * Process the service. It drives the melody playback and must be called cyclic.
     */
    void process() final;

    /**
     * Play a melody in RTTTL format.
     *
     * A running melody will be replaced immediately. The melody is copied, the
     * caller keeps the ownership of its string.
     *
     * Example: "Entertainer:d=4,o=5,b=140:8d6,16d#6,16e6"
     *
     * @param[in] melody    The melody in RTTTL format.
     * @param[in] volume    The volume (0 to RTTTL::MAX_VOLUME).
     *
     * @return If the melody is accepted, it will return true otherwise false.
     */
    bool playMelody(const String& melody, uint8_t volume);

    /**
     * Stop a running melody. If no melody is running, nothing happens.
     */
    void stopMelody();

    /**
     * Is a melody currently playing?
     *
     * @return If a melody is playing, it will return true otherwise false.
     */
    bool isPlaying() const;

    /**
     * Get the melody which is currently playing.
     *
     * @return The melody in RTTTL format. If no melody is playing, it will be empty.
     */
    const String& getCurrentMelody() const;

private:

    /**
     * Forwards to the real buzzer driver, which is only known after the service
     * is started.
     *
     * The player takes its buzzer driver by reference, which can not be rebound
     * later. Therefore the player always references this proxy. Without a real
     * driver the proxy is silent, which is the behaviour on a board without a
     * buzzer.
     */
    class BuzzerProxy : public IBuzzerDrv
    {
    public:

        /**
         * Constructs the buzzer proxy. It is silent until a driver is set.
         */
        BuzzerProxy() :
            IBuzzerDrv(),
            m_buzzerDrv(nullptr)
        {
        }

        /**
         * Destroys the buzzer proxy.
         */
        ~BuzzerProxy()
        {
        }

        /**
         * Set the buzzer driver to forward to.
         *
         * @param[in] buzzerDrv The buzzer driver. It must exist over the proxy lifetime.
         */
        void setBuzzerDrv(IBuzzerDrv& buzzerDrv)
        {
            m_buzzerDrv = &buzzerDrv;
        }

        /**
         * Initialize the buzzer driver.
         *
         * @param[in] buzzerOut Pin of the buzzer.
         */
        void init(const DOutPin& buzzerOut) final
        {
            if (nullptr != m_buzzerDrv)
            {
                m_buzzerDrv->init(buzzerOut);
            }
        }

        /**
         * Stop the buzzer.
         */
        void stop() final
        {
            if (nullptr != m_buzzerDrv)
            {
                m_buzzerDrv->stop();
            }
        }

        /**
         * Play a tone with the given frequency.
         *
         * @param[in] freq  Frequency in Hz.
         */
        void play(uint32_t freq) final
        {
            if (nullptr != m_buzzerDrv)
            {
                m_buzzerDrv->play(freq);
            }
        }

        /**
         * Play a tone with the given frequency and duty cycle.
         *
         * @param[in] freq  Frequency in Hz.
         * @param[in] dc    Duty cycle in digits.
         */
        void play(uint32_t freq, uint16_t dc) final
        {
            if (nullptr != m_buzzerDrv)
            {
                m_buzzerDrv->play(freq, dc);
            }
        }

        /**
         * Change the duty cycle of the currently played tone.
         *
         * @param[in] dc    Duty cycle in digits.
         */
        void changeDutyCycle(uint16_t dc) final
        {
            if (nullptr != m_buzzerDrv)
            {
                m_buzzerDrv->changeDutyCycle(dc);
            }
        }

    private:

        IBuzzerDrv* m_buzzerDrv; /**< The real buzzer driver or nullptr. */

        BuzzerProxy(const BuzzerProxy& proxy);
        BuzzerProxy& operator=(const BuzzerProxy& proxy);
    };

    BuzzerProxy m_buzzerProxy;   /**< Forwards to the buzzer driver of the board. */
    RTTTL       m_player;        /**< The RTTTL player. */
    String      m_currentMelody; /**< The melody which is currently playing. It is referenced by the player! */
    bool        m_isStarted;     /**< Is the service started? */

    /**
     * Constructs the melody service.
     */
    MelodyService() :
        IService(),
        m_buzzerProxy(),
        m_player(m_buzzerProxy),
        m_currentMelody(),
        m_isStarted(false)
    {
    }

    /**
     * Destroys the melody service.
     */
    ~MelodyService()
    {
    }

    MelodyService(const MelodyService& service);
    MelodyService& operator=(const MelodyService& service);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* MELODY_SERVICE_H */

/** @} */

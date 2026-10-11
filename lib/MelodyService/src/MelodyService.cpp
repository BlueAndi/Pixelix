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
 * @file   MelodyService.cpp
 * @brief  Melody service
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MelodyService.h"

#include <Logging.h>

#if (0 != CONFIG_MELODY_SERVICE_ENABLE)

#include <Board.h>

#endif /* (0 != CONFIG_MELODY_SERVICE_ENABLE) */

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

bool MelodyService::start()
{
    bool isSuccessful = false;

#if (0 != CONFIG_MELODY_SERVICE_ENABLE)

    isSuccessful = startWithBuzzer(Board::getInstance().getBuzzerDrv());

#else /* (0 != CONFIG_MELODY_SERVICE_ENABLE) */

    LOG_WARNING("Melody service is not enabled for this board.");

#endif /* (0 != CONFIG_MELODY_SERVICE_ENABLE) */

    return isSuccessful;
}

bool MelodyService::startWithBuzzer(IBuzzerDrv& buzzerDrv)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (false == m_isStarted)
    {
        m_buzzerProxy.setBuzzerDrv(buzzerDrv);
        m_isStarted = true;

        LOG_INFO("Melody service started.");
    }

    return m_isStarted;
}

void MelodyService::stop()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    stopMelody();

    m_isStarted = false;

    LOG_INFO("Melody service stopped.");
}

void MelodyService::process()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (true == m_isStarted)
    {
        m_player.play();

        /* Release the melody as soon as the playback is over. As long as it is
         * playing, the string must not be touched, because the player
         * references it.
         */
        if ((true == m_player.isDone()) &&
            (false == m_currentMelody.isEmpty()))
        {
            m_currentMelody.clear();
        }
    }
}

bool MelodyService::playMelody(const String& melody, uint8_t volume)
{
    MutexGuard<MutexRecursive> guard(m_mutex);
    bool                       isAccepted = false;

    /* Guard path: without a started service there is no buzzer to play on. */
    if (false == m_isStarted)
    {
        LOG_WARNING("Melody service is not started.");
    }
    else
    {
        /* Stop first! The player references the current melody string, which is
         * replaced in the next step.
         */
        m_player.stop();

        m_currentMelody = melody;

        isAccepted      = m_player.loadMelody(m_currentMelody.c_str(), volume);

        if (false == isAccepted)
        {
            LOG_WARNING("Melody rejected.");

            m_currentMelody.clear();
        }
    }

    return isAccepted;
}

void MelodyService::stopMelody()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_player.stop();
    m_currentMelody.clear();
}

bool MelodyService::isPlaying() const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    return m_player.isPlaying();
}

const String& MelodyService::getCurrentMelody() const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    return m_currentMelody;
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

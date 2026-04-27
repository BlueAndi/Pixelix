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
 * @file   RTTTL.cpp
 * @brief  Ring Tone Text Transfer Language (RTTTL) player
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "RTTTL.h"
#include <BuzzerDrv.h>
#include <cctype>

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

const uint16_t RTTTL::NOTES[] = {
    NOTE_SILENT,
    /* Octave 4 */
    NOTE_C4,
    NOTE_CS4,
    NOTE_D4,
    NOTE_DS4,
    NOTE_E4,
    NOTE_F4,
    NOTE_FS4,
    NOTE_G4,
    NOTE_GS4,
    NOTE_A4,
    NOTE_AS4,
    NOTE_B4,
    /* Octave 5 */
    NOTE_C5,
    NOTE_CS5,
    NOTE_D5,
    NOTE_DS5,
    NOTE_E5,
    NOTE_F5,
    NOTE_FS5,
    NOTE_G5,
    NOTE_GS5,
    NOTE_A5,
    NOTE_AS5,
    NOTE_B5,
    /* Octave 6 */
    NOTE_C6,
    NOTE_CS6,
    NOTE_D6,
    NOTE_DS6,
    NOTE_E6,
    NOTE_F6,
    NOTE_FS6,
    NOTE_G6,
    NOTE_GS6,
    NOTE_A6,
    NOTE_AS6,
    NOTE_B6,
    /* Octave 7 */
    NOTE_C7,
    NOTE_CS7,
    NOTE_D7,
    NOTE_DS7,
    NOTE_E7,
    NOTE_F7,
    NOTE_FS7,
    NOTE_G7,
    NOTE_GS7,
    NOTE_A7,
    NOTE_AS7,
    NOTE_B7
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void RTTTL::loadMelody(const char* melody)
{
    loadMelody(melody, DEFAULT_VOLUME);
}

void RTTTL::loadMelody(const char* melody, uint8_t volume)
{
    if (nullptr != melody)
    {
        uint32_t number;

        m_melody          = melody;
        m_defaultDuration = DEFAULT_DURATION;
        m_defaultOctave   = DEFAULT_OCTAVE;
        m_bpm             = DEFAULT_BPM;
        m_volume          = volume;
        m_noteDelay       = 0U;
        m_isPlaying       = true;

        /* Stop current note. */
        BuzzerDrv::getInstance().stop();

        /* Format: d=N,o=N,b=NNN: */

        /* Skip name. */
        while (*m_melody != ':')
        {
            ++m_melody;
        }

        /* Skip ':' */
        ++m_melody;

        /* Get default duration */
        if ('d' == *m_melody)
        {
            /* Skip "d=" */
            ++m_melody;
            ++m_melody;

            number = getNumber(m_melody, &m_melody);

            if ((0U < number) && (UINT8_MAX >= number))
            {
                m_defaultDuration = number;
            }

            /* Skip comma. */
            ++m_melody;
        }

        /* Get default octave. */
        if ('o' == *m_melody)
        {
            /* Skip "o=" */
            ++m_melody;
            ++m_melody;

            number = getNumber(m_melody, &m_melody);

            if ((OCTAVE_LOWER_LIMIT <= number) && (OCTAVE_UPPER_LIMIT >= number))
            {
                m_defaultOctave = number;
            }

            /* Skip comma. */
            ++m_melody;
        }

        /* Get BPM. */
        if ('b' == *m_melody)
        {
            /* Skip "b=" */
            ++m_melody;
            ++m_melody;

            m_bpm = getNumber(m_melody, &m_melody);

            /* Skip colon. */
            ++m_melody;
        }

        /* BPM = number of quarter notes per minute.
         * This is the time for whole note (in ms).
         */
        m_wholeNoteDuration = (60U * 1000U / m_bpm) * 4U;
    }
}

void RTTTL::play()
{
    /* Are we still playing a note? */
    if (true == m_isPlaying)
    {
        uint32_t timestamp = millis();

        /* Are we ready to play the next note? */
        if (timestamp >= m_noteDelay)
        {
            /* Melody end reached? */
            if ((nullptr == m_melody) ||
                ('\0' == *m_melody))
            {
                stop();
            }
            else
            {
                nextNote();
            }
        }
    }
}

void RTTTL::stop()
{
    if (true == m_isPlaying)
    {
        BuzzerDrv::getInstance().stop();

        m_melody    = nullptr;
        m_isPlaying = false;
    }
}

bool RTTTL::isPlaying()
{
    return m_isPlaying;
}

bool RTTTL::isDone()
{
    return (false == m_isPlaying);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

uint32_t RTTTL::getNumber(const char* str, const char** remaining)
{
    uint32_t number = 0U;

    if (nullptr != str)
    {
        while (true == std::isdigit(*str))
        {
            number = (number * 10U) + ((*str) - '0');
            ++str;
        }

        if (nullptr != remaining)
        {
            *remaining = str;
        }
    }

    return number;
}

void RTTTL::nextNote()
{
    uint32_t duration;
    uint8_t  noteOffset;
    uint32_t scale;
    uint32_t number;

    /* Stop current note */
    BuzzerDrv::getInstance().stop();

    /* First, get note duration, if available. */
    number = getNumber(m_melody, &m_melody);

    if (0U < number)
    {
        duration = m_wholeNoteDuration / number;
    }
    else
    {
        /* We will need to check if we are a dotted note after. */
        duration = m_wholeNoteDuration / m_defaultDuration;
    }

    /* Now get the note offset for the NOTES table. */
    noteOffset = getNoteOffset(*m_melody);
    ++m_melody;

    /* Get optional '#' sharp. */
    if ('#' == *m_melody)
    {
        ++noteOffset;
        ++m_melody;
    }

    /* Get optional '.' dotted note. */
    if ('.' == *m_melody)
    {
        duration += duration / 2U;
        ++m_melody;
    }

    /* Get scale. */
    if (true == std::isdigit(*m_melody))
    {
        scale = *m_melody - '0';
        ++m_melody;
    }
    else
    {
        scale = m_defaultOctave;
    }

    /* Limit scale. */
    if ((OCTAVE_LOWER_LIMIT > scale) ||
        (OCTAVE_UPPER_LIMIT < scale))
    {
        scale = m_defaultOctave;
    }

    if (',' == *m_melody)
    {
        /* Skip comma for next note (or we may be at the end). */
        ++m_melody;
    }

    if (0U < noteOffset)
    {
        uint32_t octaveOffset = (scale - OCTAVE_LOWER_LIMIT) * NOTES_PER_OCTAVE;
        uint32_t freq         = NOTES[octaveOffset + noteOffset];                       /* [Hz] */
        uint16_t dc           = MAX_DUTY_CYCLE * static_cast<uint16_t>(m_volume) / 10U; /* [digits] */

        BuzzerDrv::getInstance().play(freq, dc);

        m_noteDelay = millis() + duration + 1U;
    }
    else
    {
        m_noteDelay = millis() + duration;
    }
}

uint8_t RTTTL::getNoteOffset(char noteChar)
{
    uint8_t noteOffset = 0U;

    switch (std::tolower(noteChar))
    {
    case 'c':
        noteOffset = 1U;
        break;

    case 'd':
        noteOffset = 3U;
        break;

    case 'e':
        noteOffset = 5U;
        break;

    case 'f':
        noteOffset = 6U;
        break;

    case 'g':
        noteOffset = 8U;
        break;

    case 'a':
        noteOffset = 10U;
        break;

    case 'b':
        noteOffset = 12U;
        break;

    case 'p': /* Pause */
    default:
        noteOffset = 0U;
        break;
    }

    return noteOffset;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

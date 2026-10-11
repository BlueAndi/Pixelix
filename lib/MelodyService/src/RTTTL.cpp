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

#include <Util.h>

#include <cctype>
#include <string.h>

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

bool RTTTL::loadMelody(const char* melody)
{
    return loadMelody(melody, DEFAULT_VOLUME);
}

bool RTTTL::loadMelody(const char* melody, uint8_t volume)
{
    bool isAccepted = false;

    /* Guard path: a melody must exist. */
    if (nullptr == melody)
    {
        ;
    }
    /* Guard path: a melody must be limited, because it is parsed character by character. */
    else if (MAX_MELODY_LENGTH < strnlen(melody, MAX_MELODY_LENGTH + 1U))
    {
        ;
    }
    /* Guard path: the name is separated by a colon from the rest. Without it the
     * melody is no RTTTL and the parser would have nothing to synchronize on.
     */
    else if (nullptr == strchr(melody, ':'))
    {
        ;
    }
    else
    {
        uint32_t number;

        m_melody          = melody;
        m_melodyEnd       = melody + strlen(melody);
        m_defaultDuration = DEFAULT_DURATION;
        m_defaultOctave   = DEFAULT_OCTAVE;
        m_bpm             = DEFAULT_BPM;
        m_volume          = (MAX_VOLUME < volume) ? MAX_VOLUME : volume;
        m_isPlaying       = true;

        m_noteTimer.start(0U);

        /* Stop current note. */
        m_buzzerDrv.stop();

        /* Format: d=N,o=N,b=NNN: */

        /* Skip name. The colon is guaranteed to exist, see above. */
        while (':' != *m_melody)
        {
            advance(1U);
        }

        /* Skip ':' */
        advance(1U);

        /* Get default duration */
        if ('d' == *m_melody)
        {
            /* Skip "d=" */
            advance(2U);

            number = getNumber(m_melody, &m_melody);

            if ((0U < number) && (UINT8_MAX >= number))
            {
                m_defaultDuration = static_cast<uint8_t>(number);
            }

            /* Skip comma. */
            advanceIfChar(',');
        }

        /* Get default octave. */
        if ('o' == *m_melody)
        {
            /* Skip "o=" */
            advance(2U);

            number = getNumber(m_melody, &m_melody);

            if ((OCTAVE_LOWER_LIMIT <= number) && (OCTAVE_UPPER_LIMIT >= number))
            {
                m_defaultOctave = static_cast<uint8_t>(number);
            }

            /* Skip comma. */
            advanceIfChar(',');
        }

        /* Get BPM. */
        if ('b' == *m_melody)
        {
            /* Skip "b=" */
            advance(2U);

            m_bpm = getNumber(m_melody, &m_melody);

            /* Skip colon. */
            advanceIfChar(':');
        }

        /* A missing or absurd BPM would lead to a division by zero or to a note
         * duration of zero. Therefore it is limited to a playable range.
         */
        if (BPM_LOWER_LIMIT > m_bpm)
        {
            m_bpm = DEFAULT_BPM;
        }
        else if (BPM_UPPER_LIMIT < m_bpm)
        {
            m_bpm = BPM_UPPER_LIMIT;
        }
        else
        {
            /* BPM is in the valid range. */
            ;
        }

        /* BPM = number of quarter notes per minute.
         * This is the time for whole note (in ms).
         */
        m_wholeNoteDuration = (60U * 1000U / m_bpm) * 4U;

        isAccepted          = true;
    }

    return isAccepted;
}

void RTTTL::play()
{
    /* Are we still playing a note? */
    if (true == m_isPlaying)
    {
        /* Are we ready to play the next note? */
        if (true == m_noteTimer.isTimeout())
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
        m_buzzerDrv.stop();

        m_noteTimer.stop();

        m_melody    = nullptr;
        m_melodyEnd = nullptr;
        m_isPlaying = false;
    }
}

bool RTTTL::isPlaying() const
{
    return m_isPlaying;
}

bool RTTTL::isDone() const
{
    return (false == m_isPlaying);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void RTTTL::advance(size_t count)
{
    if ((nullptr != m_melody) &&
        (nullptr != m_melodyEnd))
    {
        size_t remaining = static_cast<size_t>(m_melodyEnd - m_melody);

        /* Never move behind the terminating null. This guarantees that every
         * dereference of the cursor reads a valid character.
         */
        if (remaining < count)
        {
            count = remaining;
        }

        m_melody += count;
    }
}

void RTTTL::advanceIfChar(char singleChar)
{
    if ((nullptr != m_melody) &&
        (singleChar == *m_melody))
    {
        advance(1U);
    }
}

uint32_t RTTTL::getNumber(const char* str, const char** remaining)
{
    uint32_t number = 0U;

    if (nullptr != str)
    {
        while (0 != std::isdigit(static_cast<unsigned char>(*str)))
        {
            uint32_t digit = static_cast<uint32_t>((*str) - '0');

            /* Limit instead of wrapping around, if the accumulator would overflow. */
            if (((UINT32_MAX - digit) / 10U) < number)
            {
                number = UINT32_MAX;
            }
            else
            {
                number = (number * 10U) + digit;
            }

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
    m_buzzerDrv.stop();

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
    advance(1U);

    /* Get optional '#' sharp. */
    if ('#' == *m_melody)
    {
        /* A sharp note of the last note in a octave, e.g. "b#", is no valid
         * note. It must not be raised, otherwise the NOTES table index would
         * leave the octave and in the upper octave even the table itself.
         */
        if (NOTES_PER_OCTAVE > noteOffset)
        {
            ++noteOffset;
        }

        advance(1U);
    }

    /* Get optional '.' dotted note. */
    if ('.' == *m_melody)
    {
        duration += duration / 2U;
        advance(1U);
    }

    /* Get scale. */
    if (0 != std::isdigit(static_cast<unsigned char>(*m_melody)))
    {
        scale = static_cast<uint32_t>(*m_melody - '0');
        advance(1U);
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

    /* Skip comma for next note (or we may be at the end). */
    advanceIfChar(',');

    if (0U < noteOffset)
    {
        uint32_t octaveOffset = (scale - OCTAVE_LOWER_LIMIT) * NOTES_PER_OCTAVE;
        uint32_t noteIndex    = octaveOffset + noteOffset;

        /* Defensive bound check. The octave and the note offset are limited
         * above, this protects the table access against a future change.
         */
        if (UTIL_ARRAY_NUM(NOTES) > noteIndex)
        {
            uint32_t freq = NOTES[noteIndex];                                              /* [Hz] */
            uint16_t dc   = MAX_DUTY_CYCLE * static_cast<uint16_t>(m_volume) / MAX_VOLUME; /* [digits] */

            m_buzzerDrv.play(freq, dc);
        }

        m_noteTimer.start(duration + 1U);
    }
    else
    {
        m_noteTimer.start(duration);
    }
}

uint8_t RTTTL::getNoteOffset(char noteChar)
{
    uint8_t noteOffset = 0U;

    switch (std::tolower(static_cast<unsigned char>(noteChar)))
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

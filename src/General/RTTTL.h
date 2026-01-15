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
 * @file   RTTTL.h
 * @brief  Ring Tone Text Transfer Language (RTTTL) player
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup TIMER_SERVICE
 *
 * @{
 */

#ifndef RTTTL_H
#define RTTTL_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/**
 * Silent note with pitch frequency 0 Hz.
 */
#define NOTE_SILENT (0U)

/**
 * Note B0 with pitch frequency 31 Hz.
 */
#define NOTE_B0 (31U)

/**
 * Note C1 with pitch frequency 33 Hz.
 */
#define NOTE_C1 (33U)

/**
 * Note C#1 with pitch frequency 35 Hz.
 */
#define NOTE_CS1 (35U)

/**
 * Note D1 with pitch frequency 37 Hz.
 */
#define NOTE_D1 (37U)

/**
 * Note D#1 with pitch frequency 39 Hz.
 */
#define NOTE_DS1 (39U)

/**
 * Note E1 with pitch frequency 41 Hz.
 */
#define NOTE_E1 (41U)

/**
 * Note F1 with pitch frequency 44 Hz.
 */
#define NOTE_F1 (44U)

/**
 * Note F#1 with pitch frequency 46 Hz.
 */
#define NOTE_FS1 (46U)

/**
 * Note G1 with pitch frequency 49 Hz.
 */
#define NOTE_G1 (49U)

/**
 * Note G#1 with pitch frequency 52 Hz.
 */
#define NOTE_GS1 (52U)

/**
 * Note A1 with pitch frequency 55 Hz.
 */
#define NOTE_A1 (55U)

/**
 * Note A#1 with pitch frequency 58 Hz.
 */
#define NOTE_AS1 (58U)

/**
 * Note B1 with pitch frequency 62 Hz.
 */
#define NOTE_B1 (62U)

/**
 * Note C2 with pitch frequency 65 Hz.
 */
#define NOTE_C2 (65U)

/**
 * Note C#2 with pitch frequency 69 Hz.
 */
#define NOTE_CS2 (69U)

/**
 * Note D2 with pitch frequency 73 Hz.
 */
#define NOTE_D2 (73U)

/**
 * Note D#2 with pitch frequency 78 Hz.
 */
#define NOTE_DS2 (78U)

/**
 * Note E2 with pitch frequency 82 Hz.
 */
#define NOTE_E2 (82U)

/**
 * Note F2 with pitch frequency 87 Hz.
 */
#define NOTE_F2 (87U)

/**
 * Note F#2 with pitch frequency 93 Hz.
 */
#define NOTE_FS2 (93U)

/**
 * Note G2 with pitch frequency 98 Hz.
 */
#define NOTE_G2 (98U)

/**
 * Note G#2 with pitch frequency 104 Hz.
 */
#define NOTE_GS2 (104U)

/**
 * Note A2 with pitch frequency 110 Hz.
 */
#define NOTE_A2 (110U)

/**
 * Note A#2 with pitch frequency 117 Hz.
 */
#define NOTE_AS2 (117U)

/**
 * Note B2 with pitch frequency 123 Hz.
 */
#define NOTE_B2 (123U)

/**
 * Note C3 with pitch frequency 131 Hz.
 */
#define NOTE_C3 (131U)

/**
 * Note C#3 with pitch frequency 139 Hz.
 */
#define NOTE_CS3 (139U)

/**
 * Note D3 with pitch frequency 147 Hz.
 */
#define NOTE_D3 (147U)

/**
 * Note D#3 with pitch frequency 156 Hz.
 */
#define NOTE_DS3 (156U)

/**
 * Note E3 with pitch frequency 165 Hz.
 */
#define NOTE_E3 (165U)

/**
 * Note F3 with pitch frequency 175 Hz.
 */
#define NOTE_F3 (175U)

/**
 * Note F#3 with pitch frequency 185 Hz.
 */
#define NOTE_FS3 (185U)

/**
 * Note G3 with pitch frequency 196 Hz.
 */
#define NOTE_G3 (196U)

/**
 * Note G#3 with pitch frequency 208 Hz.
 */
#define NOTE_GS3 (208U)

/**
 * Note A3 with pitch frequency 220 Hz.
 */
#define NOTE_A3 (220U)

/**
 * Note A#3 with pitch frequency 233 Hz.
 */
#define NOTE_AS3 (233U)

/**
 * Note B3 with pitch frequency 247 Hz.
 */
#define NOTE_B3 (247U)

/**
 * Note C4 with pitch frequency 262 Hz.
 */
#define NOTE_C4 (262U)

/**
 * Note C#4 with pitch frequency 277 Hz.
 */
#define NOTE_CS4 (277U)

/**
 * Note D4 with pitch frequency 294 Hz.
 */
#define NOTE_D4 (294U)

/**
 * Note D#4 with pitch frequency 311 Hz.
 */
#define NOTE_DS4 (311U)

/**
 * Note E4 with pitch frequency 330 Hz.
 */
#define NOTE_E4 (330U)

/**
 * Note F4 with pitch frequency 349 Hz.
 */
#define NOTE_F4 (349U)

/**
 * Note F#4 with pitch frequency 370 Hz.
 */
#define NOTE_FS4 (370U)

/**
 * Note G4 with pitch frequency 392 Hz.
 */
#define NOTE_G4 (392U)

/**
 * Note G#4 with pitch frequency 415 Hz.
 */
#define NOTE_GS4 (415U)

/**
 * Note A4 with pitch frequency 440 Hz.
 */
#define NOTE_A4 (440U)

/**
 * Note A#4 with pitch frequency 466 Hz.
 */
#define NOTE_AS4 (466U)

/**
 * Note B4 with pitch frequency 494 Hz.
 */
#define NOTE_B4 (494U)

/**
 * Note C5 with pitch frequency 523 Hz.
 */
#define NOTE_C5 (523U)

/**
 * Note C#5 with pitch frequency 554 Hz.
 */
#define NOTE_CS5 (554U)

/**
 * Note D5 with pitch frequency 587 Hz.
 */
#define NOTE_D5 (587U)

/**
 * Note D#5 with pitch frequency 622 Hz.
 */
#define NOTE_DS5 (622U)

/**
 * Note E5 with pitch frequency 659 Hz.
 */
#define NOTE_E5 (659U)

/**
 * Note F5 with pitch frequency 698 Hz.
 */
#define NOTE_F5 (698U)

/**
 * Note F#5 with pitch frequency 740 Hz.
 */
#define NOTE_FS5 (740U)

/**
 * Note G5 with pitch frequency 784 Hz.
 */
#define NOTE_G5 (784U)

/**
 * Note G#5 with pitch frequency 831 Hz.
 */
#define NOTE_GS5 (831U)

/**
 * Note A5 with pitch frequency 880 Hz.
 */
#define NOTE_A5 (880U)

/**
 * Note A#5 with pitch frequency 932 Hz.
 */
#define NOTE_AS5 (932U)

/**
 * Note B5 with pitch frequency 988 Hz.
 */
#define NOTE_B5 (988U)

/**
 * Note C6 with pitch frequency 1047 Hz.
 */
#define NOTE_C6 (1047U)

/**
 * Note C#6 with pitch frequency 1109 Hz.
 */
#define NOTE_CS6 (1109U)

/**
 * Note D6 with pitch frequency 1175 Hz.
 */
#define NOTE_D6 (1175U)

/**
 * Note D#6 with pitch frequency 1245 Hz.
 */
#define NOTE_DS6 (1245U)

/**
 * Note E6 with pitch frequency 1319 Hz.
 */
#define NOTE_E6 (1319U)

/**
 * Note F6 with pitch frequency 1397 Hz.
 */
#define NOTE_F6 (1397U)

/**
 * Note F#6 with pitch frequency 1480 Hz.
 */
#define NOTE_FS6 (1480U)

/**
 * Note G6 with pitch frequency 1568 Hz.
 */
#define NOTE_G6 (1568U)

/**
 * Note G#6 with pitch frequency 1661 Hz.
 */
#define NOTE_GS6 (1661U)

/**
 * Note A6 with pitch frequency 1760 Hz.
 */
#define NOTE_A6 (1760U)

/**
 * Note A#6 with pitch frequency 1865 Hz.
 */
#define NOTE_AS6 (1865U)

/**
 * Note B6 with pitch frequency 1976 Hz.
 */
#define NOTE_B6 (1976U)

/**
 * Note C7 with pitch frequency 2093 Hz.
 */
#define NOTE_C7 (2093U)

/**
 * Note C#7 with pitch frequency 2217 Hz.
 */
#define NOTE_CS7 (2217U)

/**
 * Note D7 with pitch frequency 2349 Hz.
 */
#define NOTE_D7 (2349U)

/**
 * Note D#7 with pitch frequency 2489 Hz.
 */
#define NOTE_DS7 (2489U)

/**
 * Note E7 with pitch frequency 2637 Hz.
 */
#define NOTE_E7 (2637U)

/**
 * Note F7 with pitch frequency 2794 Hz.
 */
#define NOTE_F7 (2794U)

/**
 * Note F#7 with pitch frequency 2960 Hz.
 */
#define NOTE_FS7 (2960U)

/**
 * Note G7 with pitch frequency 3136 Hz.
 */
#define NOTE_G7 (3136U)

/**
 * Note G#7 with pitch frequency 3322 Hz.
 */
#define NOTE_GS7 (3322U)

/**
 * Note A7 with pitch frequency 3520 Hz.
 */
#define NOTE_A7 (3520U)

/**
 * Note A#7 with pitch frequency 3729 Hz.
 */
#define NOTE_AS7 (3729U)

/**
 * Note B7 with pitch frequency 3951 Hz.
 */
#define NOTE_B7 (3951U)

/**
 * Note C8 with pitch frequency 4186 Hz.
 */
#define NOTE_C8 (4186U)

/**
 * Note C#8 with pitch frequency 4435 Hz.
 */
#define NOTE_CS8 (4435U)

/**
 * Note D8 with pitch frequency 4699 Hz.
 */
#define NOTE_D8 (4699U)

/**
 * Note D#8 with pitch frequency 4978 Hz.
 */
#define NOTE_DS8 (4978U)

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Ring Tone Text Transfer Language (RTTTL) player.
 */
class RTTTL
{
public:

    /**
     * Constructs the RTTTL player.
     */
    RTTTL() :
        m_melody(nullptr),
        m_defaultDuration(DEFAULT_DURATION),
        m_defaultOctave(DEFAULT_OCTAVE),
        m_bpm(DEFAULT_BPM),
        m_volume(DEFAULT_VOLUME),
        m_noteDelay(0U),
        m_isPlaying(false),
        m_wholeNoteDuration(0U)
    {
    }

    /**
     * Destroys the RTTTL player.
     */
    ~RTTTL()
    {
        stop();
    }

    /**
     * Load melody in RTTTL format.
     * The default volume is 10.
     *
     * Example: "Entertainer:d=4,o=5,b=140:8d6,16d#6,16e6,16f6,16f#6,16g6,16g#6,16a6,16a#6,16b6,8c7"
     *
     * @param[in] melody The melody in RTTTL format.
     */
    void loadMelody(const char* melody);

    /**
     * Load melody in RTTTL format with specific volume.
     * Volume range is 0 to 10.
     *
     * Example: "Entertainer:d=4,o=5,b=140:8d6,16d#6,16e6,16f6,16f#6,16g6,16g#6,16a6,16a#6,16b6,8c7"
     *
     * @param[in] melody The melody in RTTTL format.
     * @param[in] volume The volume (0 to 10).
     */
    void loadMelody(const char* melody, uint8_t volume);

    /**
     * Play loaded song.
     * If no song is loaded, nothing happens.
     *
     * The play is non-blocking. You have to call the play() method cyclic.
     */
    void play();

    /**
     * Stop playing.
     */
    void stop();

    /**
     * Is the player currently playing?
     *
     * @return If playing, it will return true otherwise false.
     */
    bool isPlaying();

    /**
     * Is the song done?
     *
     * @return If done, it will return true otherwise false.
     */
    bool isDone();

private:

    static const uint16_t NOTES[];                   /**< Note frequencies for 8 octaves (C1 to C8) */
    static const uint32_t NOTES_PER_OCTAVE   = 12U;  /**< Number of notes per octave. */
    static const uint8_t  DEFAULT_DURATION   = 4U;   /**< Default duration if not specified in RTTTL. */
    static const uint8_t  DEFAULT_OCTAVE     = 6U;   /**< Default octave if not specified in RTTTL. */
    static const uint32_t DEFAULT_BPM        = 63U;  /**< Default BPM if not specified in RTTTL. */
    static const uint8_t  DEFAULT_VOLUME     = 10U;  /**< Default volume if not specified in RTTTL. */
    static const uint8_t  OCTAVE_LOWER_LIMIT = 4U;   /**< Lower octave limit. */
    static const uint8_t  OCTAVE_UPPER_LIMIT = 7U;   /**< Upper octave limit. */
    static const uint16_t MAX_DUTY_CYCLE     = 125U; /**< Maximum duty cycle in digits. A greater value may sound strange. */

    /** Whole note duration in milliseconds. */
    const char* m_melody;

    /** Default duration of a note (in beats). */
    uint8_t m_defaultDuration;

    /** Default octave. */
    uint8_t m_defaultOctave;

    /** Beats per minute. */
    uint32_t m_bpm;

    /** Volume (0 to 10). */
    uint8_t m_volume;

    /** Note delay in ms. */
    uint32_t m_noteDelay;

    /** Is the player currently playing? */
    bool m_isPlaying;

    /** Whole note duration in ms. */
    uint32_t m_wholeNoteDuration;

    /**
     * Get number from string.
     *
     * @param[in]   str         The string.
     * @param[out]  remaining   The remaining string after the number.
     *
     * @return The number.
     */
    uint32_t getNumber(const char* str, const char** remaining);

    /**
     * Proceed to the next note.
     */
    void nextNote();

    /**
     * Get note offset in the NOTES table.
     *
     * @param[in] noteChar The note character.
     *
     * @return Note offset in the NOTES table.
     */
    uint8_t getNoteOffset(char noteChar);
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* RTTTL_H */

/** @} */
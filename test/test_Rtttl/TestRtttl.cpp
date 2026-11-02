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
 * @file   TestRtttl.cpp
 * @brief  Test of the RTTTL player, especially with malformed melodies.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <RTTTL.h>
#include <Util.h>
#include "../common/TestBuzzerDrv.hpp"

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

static void testValidMelody(void);
static void testMelodyWithoutSeparator(void);
static void testMelodyTooLong(void);
static void testBeatsPerMinuteZero(void);
static void testTruncatedMelody(void);
static void testSharpNoteInUpperOctave(void);
static void testVolumeLimit(void);

static void playToEnd(RTTTL& player);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Maximum time in ms a melody may take, used to avoid a endless test run. */
static const uint32_t MAX_PLAY_TIME = 10000U;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/**
 * Main entry point
 *
 * @param[in] argc  Number of command line arguments
 * @param[in] argv  Command line arguments
 */
extern int main(int argc, char** argv)
{
    UTIL_NOT_USED(argc);
    UTIL_NOT_USED(argv);

    UNITY_BEGIN();

    RUN_TEST(testValidMelody);
    RUN_TEST(testMelodyWithoutSeparator);
    RUN_TEST(testMelodyTooLong);
    RUN_TEST(testBeatsPerMinuteZero);
    RUN_TEST(testTruncatedMelody);
    RUN_TEST(testSharpNoteInUpperOctave);
    RUN_TEST(testVolumeLimit);

    return UNITY_END();
}

/**
 * Setup a test. This function will be called before every test by unity.
 */
extern void setUp(void)
{
    /* Not used. */
}

/**
 * Clean up test. This function will be called after every test by unity.
 */
extern void tearDown(void)
{
    /* Not used. */
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Play the loaded melody until it is done.
 *
 * @param[in] player The RTTTL player.
 */
static void playToEnd(RTTTL& player)
{
    uint32_t startTime = millis();

    /* The player works with real time, therefore the loop is limited by time
     * and not by a number of cycles.
     */
    while ((false == player.isDone()) &&
           (MAX_PLAY_TIME > (millis() - startTime)))
    {
        player.play();
    }

    TEST_ASSERT_TRUE_MESSAGE(player.isDone(), "Melody did not finish.");
}

/**
 * A valid melody is accepted and played.
 */
static void testValidMelody(void)
{
    TestBuzzerDrv buzzerDrv;
    RTTTL         player(buzzerDrv);

    TEST_ASSERT_TRUE(player.loadMelody("Test:d=4,o=5,b=900:8c6,8d6,8e6"));
    TEST_ASSERT_TRUE(player.isPlaying());

    playToEnd(player);

    TEST_ASSERT_EQUAL_UINT32(3U, buzzerDrv.m_playCount);
}

/**
 * A melody without the mandatory separator is rejected. Without the rejection
 * the parser would search the separator beyond the end of the string.
 */
static void testMelodyWithoutSeparator(void)
{
    TestBuzzerDrv buzzerDrv;
    RTTTL         player(buzzerDrv);

    TEST_ASSERT_FALSE(player.loadMelody("no separator at all"));
    TEST_ASSERT_FALSE(player.isPlaying());
    TEST_ASSERT_TRUE(player.isDone());

    TEST_ASSERT_FALSE(player.loadMelody(""));
    TEST_ASSERT_FALSE(player.isPlaying());

    TEST_ASSERT_FALSE(player.loadMelody(nullptr));
    TEST_ASSERT_FALSE(player.isPlaying());

    TEST_ASSERT_EQUAL_UINT32(0U, buzzerDrv.m_playCount);
}

/**
 * A melody above the length limit is rejected.
 */
static void testMelodyTooLong(void)
{
    TestBuzzerDrv buzzerDrv;
    RTTTL         player(buzzerDrv);
    String        melody = "Test:d=4,o=5,b=900:";

    while (RTTTL::MAX_MELODY_LENGTH >= melody.length())
    {
        melody += "8c6,";
    }

    TEST_ASSERT_FALSE(player.loadMelody(melody.c_str()));
    TEST_ASSERT_FALSE(player.isPlaying());
    TEST_ASSERT_EQUAL_UINT32(0U, buzzerDrv.m_playCount);
}

/**
 * A beats per minute value of zero must not lead to a division by zero.
 */
static void testBeatsPerMinuteZero(void)
{
    TestBuzzerDrv buzzerDrv;
    RTTTL         player(buzzerDrv);

    TEST_ASSERT_TRUE(player.loadMelody("Test:d=4,o=5,b=0:8c6"));
    playToEnd(player);

    buzzerDrv.reset();

    /* A missing number behind "b=" has the same effect. */
    TEST_ASSERT_TRUE(player.loadMelody("Test:d=4,o=5,b=:8c6"));
    playToEnd(player);

    buzzerDrv.reset();

    /* A absurd number must not wrap around into zero either. */
    TEST_ASSERT_TRUE(player.loadMelody("Test:d=4,o=5,b=99999999999999999999:8c6"));
    playToEnd(player);
}

/**
 * A melody which ends in the middle of a token must not be parsed beyond its end.
 */
static void testTruncatedMelody(void)
{
    static const char* MELODIES[] = {
        "Test:",
        "Test:d",
        "Test:d=",
        "Test:d=4,",
        "Test:d=4,o",
        "Test:d=4,o=5,b",
        "Test:d=4,o=5,b=",
        "Test:d=4,o=5,b=900:",
        "Test:d=4,o=5,b=900:8",
        "Test:d=4,o=5,b=900:8c",
        "Test:d=4,o=5,b=900:8c#",
        "Test:d=4,o=5,b=900:8c6,",
        "Test:d=4,o=5,b=900:8c6,."
    };
    uint32_t index = 0U;

    for (index = 0U; index < UTIL_ARRAY_NUM(MELODIES); ++index)
    {
        TestBuzzerDrv buzzerDrv;
        RTTTL         player(buzzerDrv);

        /* The melody is syntactically acceptable, it must only not crash
         * and it must terminate.
         */
        TEST_ASSERT_TRUE_MESSAGE(player.loadMelody(MELODIES[index]), MELODIES[index]);
        playToEnd(player);
    }
}

/**
 * A sharp note of the last note in a octave must not leave the note table.
 */
static void testSharpNoteInUpperOctave(void)
{
    TestBuzzerDrv buzzerDrv;
    RTTTL         player(buzzerDrv);

    /* "b#7" would address the element behind the note table. */
    TEST_ASSERT_TRUE(player.loadMelody("Test:d=4,o=7,b=900:8b#7"));
    playToEnd(player);

    /* A tone was played and its frequency is one of the table, not garbage. */
    TEST_ASSERT_EQUAL_UINT32(1U, buzzerDrv.m_playCount);
    TEST_ASSERT_EQUAL_UINT32(NOTE_B7, buzzerDrv.m_lastFreq);
}

/**
 * The volume must be limited, otherwise the duty cycle calculation overflows.
 */
static void testVolumeLimit(void)
{
    TestBuzzerDrv buzzerDrv;
    RTTTL         player(buzzerDrv);

    TEST_ASSERT_TRUE(player.loadMelody("Test:d=4,o=5,b=900:8c6", UINT8_MAX));
    playToEnd(player);

    TEST_ASSERT_EQUAL_UINT32(1U, buzzerDrv.m_playCount);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(125U, buzzerDrv.m_maxDutyCycle);
}

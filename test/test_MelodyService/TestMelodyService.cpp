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
 * @file   TestMelodyService.cpp
 * @brief  Test of the melody service.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <MelodyService.h>
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

static void testNotStarted(void);
static void testPlayMelody(void);
static void testReplaceRunningMelody(void);
static void testRejectMalformedMelody(void);
static void testStopMelody(void);

static void processUntilDone(MelodyService& service);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Buzzer driver stub, it must exist over the service lifetime. */
static TestBuzzerDrv gBuzzerDrv;

/** Maximum time in ms a melody may take, used to avoid a endless test run. */
static const uint32_t MAX_PLAY_TIME = 10000U;

/** A melody with a short playback time. */
static const char* MELODY_SHORT     = "Short:d=4,o=5,b=900:8c6,8d6";

/** A different melody with a short playback time. */
static const char* MELODY_OTHER     = "Other:d=4,o=5,b=900:8e6,8f6";

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

    /* Must run first, it relies on the service not being started yet. */
    RUN_TEST(testNotStarted);

    RUN_TEST(testPlayMelody);
    RUN_TEST(testReplaceRunningMelody);
    RUN_TEST(testRejectMalformedMelody);
    RUN_TEST(testStopMelody);

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
 * Process the service until the melody is done.
 *
 * @param[in] service The melody service.
 */
static void processUntilDone(MelodyService& service)
{
    uint32_t startTime = millis();

    /* The player works with real time, therefore the loop is limited by time. */
    while ((true == service.isPlaying()) &&
           (MAX_PLAY_TIME > (millis() - startTime)))
    {
        service.process();
    }

    /* One more cycle, so the service can release the melody. */
    service.process();

    TEST_ASSERT_FALSE_MESSAGE(service.isPlaying(), "Melody did not finish.");
}

/**
 * Without a started service no melody is accepted.
 */
static void testNotStarted(void)
{
    MelodyService& service = MelodyService::getInstance();

    TEST_ASSERT_FALSE(service.playMelody(MELODY_SHORT, 5U));
    TEST_ASSERT_FALSE(service.isPlaying());
    TEST_ASSERT_TRUE(service.getCurrentMelody().isEmpty());
}

/**
 * A melody is played and reported until it is done.
 */
static void testPlayMelody(void)
{
    MelodyService& service = MelodyService::getInstance();

    TEST_ASSERT_TRUE(service.startWithBuzzer(gBuzzerDrv));

    gBuzzerDrv.reset();

    TEST_ASSERT_TRUE(service.playMelody(MELODY_SHORT, 5U));
    TEST_ASSERT_TRUE(service.isPlaying());
    TEST_ASSERT_EQUAL_STRING(MELODY_SHORT, service.getCurrentMelody().c_str());

    processUntilDone(service);

    /* The melody is released after the playback. */
    TEST_ASSERT_TRUE(service.getCurrentMelody().isEmpty());
    TEST_ASSERT_EQUAL_UINT32(2U, gBuzzerDrv.m_playCount);
}

/**
 * A new melody replaces the running one. There is no queue.
 *
 * This also covers the melody lifetime: the player references the string owned
 * by the service, so replacing it while playing must not leave a dangling
 * pointer behind.
 */
static void testReplaceRunningMelody(void)
{
    MelodyService& service = MelodyService::getInstance();

    TEST_ASSERT_TRUE(service.startWithBuzzer(gBuzzerDrv));

    gBuzzerDrv.reset();

    TEST_ASSERT_TRUE(service.playMelody(MELODY_SHORT, 5U));
    service.process();
    TEST_ASSERT_TRUE(service.isPlaying());

    /* Replace it in the middle of the playback. */
    TEST_ASSERT_TRUE(service.playMelody(MELODY_OTHER, 5U));
    TEST_ASSERT_TRUE(service.isPlaying());
    TEST_ASSERT_EQUAL_STRING(MELODY_OTHER, service.getCurrentMelody().c_str());

    processUntilDone(service);

    TEST_ASSERT_TRUE(service.getCurrentMelody().isEmpty());
}

/**
 * A malformed melody is rejected and does not become the current melody.
 */
static void testRejectMalformedMelody(void)
{
    MelodyService& service = MelodyService::getInstance();

    TEST_ASSERT_TRUE(service.startWithBuzzer(gBuzzerDrv));

    gBuzzerDrv.reset();

    TEST_ASSERT_FALSE(service.playMelody("no separator at all", 5U));
    TEST_ASSERT_FALSE(service.isPlaying());
    TEST_ASSERT_TRUE(service.getCurrentMelody().isEmpty());
    TEST_ASSERT_EQUAL_UINT32(0U, gBuzzerDrv.m_playCount);

    TEST_ASSERT_FALSE(service.playMelody("", 5U));
    TEST_ASSERT_FALSE(service.isPlaying());
    TEST_ASSERT_TRUE(service.getCurrentMelody().isEmpty());
}

/**
 * A running melody can be stopped.
 */
static void testStopMelody(void)
{
    MelodyService& service = MelodyService::getInstance();

    TEST_ASSERT_TRUE(service.startWithBuzzer(gBuzzerDrv));

    gBuzzerDrv.reset();

    TEST_ASSERT_TRUE(service.playMelody(MELODY_SHORT, 5U));
    TEST_ASSERT_TRUE(service.isPlaying());

    service.stopMelody();

    TEST_ASSERT_FALSE(service.isPlaying());
    TEST_ASSERT_TRUE(service.getCurrentMelody().isEmpty());

    /* A stopped melody must not continue on the next process cycle. */
    gBuzzerDrv.reset();
    service.process();
    TEST_ASSERT_EQUAL_UINT32(0U, gBuzzerDrv.m_playCount);
}

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
 * @brief  Test the native button driver.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <ButtonDrv.h>
#include <Io.hpp>
#include <Util.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/**
 * Button observer, which records every notification for later verification.
 */
class TestButtonObserver : public IButtonObserver
{
public:

    /** Max. number of recorded notifications. */
    static const uint8_t MAX_NOTIFICATIONS = 8U;

    /**
     * A single recorded notification.
     */
    struct Notification
    {
        ButtonId    buttonId; /**< Notified button id. */
        ButtonState state;    /**< Notified button state. */

        Notification() :
            buttonId(BUTTON_ID_CNT),
            state(BUTTON_STATE_NC)
        {
        }
    };

    /**
     * Constructs the test observer.
     */
    TestButtonObserver() :
        IButtonObserver(),
        m_notifications(),
        m_count(0U)
    {
    }

    /**
     * Destroys the test observer.
     */
    ~TestButtonObserver()
    {
    }

    /**
     * Record the notification.
     *
     * @param[in] buttonId  The id of the related button.
     * @param[in] state     New button state of the button.
     */
    void notify(ButtonId buttonId, ButtonState state) override
    {
        if (MAX_NOTIFICATIONS > m_count)
        {
            m_notifications[m_count].buttonId = buttonId;
            m_notifications[m_count].state    = state;
            ++m_count;
        }
    }

    /**
     * Get the number of recorded notifications.
     *
     * @return Number of recorded notifications.
     */
    uint8_t getCount() const
    {
        return m_count;
    }

    /**
     * Get a recorded notification.
     *
     * @param[in] index Notification index, must be lower than getCount().
     *
     * @return Notification
     */
    const Notification& get(uint8_t index) const
    {
        return m_notifications[index];
    }

    /**
     * Forget all recorded notifications.
     */
    void clear()
    {
        m_count = 0U;
    }

private:

    Notification m_notifications[MAX_NOTIFICATIONS]; /**< Recorded notifications. */
    uint8_t      m_count;                            /**< Number of recorded notifications. */
};

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static void testInitialStateIsReleased();
static void testRegisterObserverNotifiesCurrentStates();
static void testUpdateButtonNotifiesOnlyOnChange();

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Not connected digital input, used for the button driver initialization. */
static const DInPinT<IoPin::NC, INPUT_PULLUP> gNcPin;

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

    RUN_TEST(testInitialStateIsReleased);
    RUN_TEST(testRegisterObserverNotifiesCurrentStates);
    RUN_TEST(testUpdateButtonNotifiesOnlyOnChange);

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
 * The simulated buttons are not pressed at startup, therefore every button
 * shall report the released state right after initialization. The button
 * handler ignores all states of a button until it has seen a released state.
 */
static void testInitialStateIsReleased()
{
    ButtonDrv buttonDrv;
    uint8_t   buttonIdx = 0U;

    TEST_ASSERT_TRUE(buttonDrv.init(gNcPin, gNcPin, gNcPin));

    while (BUTTON_ID_CNT > buttonIdx)
    {
        TEST_ASSERT_EQUAL(BUTTON_STATE_RELEASED, buttonDrv.getState(static_cast<ButtonId>(buttonIdx)));
        ++buttonIdx;
    }
}

/**
 * Like the ESP32 button driver, registering an observer shall notify it once
 * about the current state of every button.
 */
static void testRegisterObserverNotifiesCurrentStates()
{
    ButtonDrv          buttonDrv;
    TestButtonObserver observer;
    uint8_t            buttonIdx = 0U;

    TEST_ASSERT_TRUE(buttonDrv.init(gNcPin, gNcPin, gNcPin));

    buttonDrv.registerObserver(observer);

    TEST_ASSERT_EQUAL_UINT8(BUTTON_ID_CNT, observer.getCount());

    while (BUTTON_ID_CNT > buttonIdx)
    {
        TEST_ASSERT_EQUAL(static_cast<ButtonId>(buttonIdx), observer.get(buttonIdx).buttonId);
        TEST_ASSERT_EQUAL(BUTTON_STATE_RELEASED, observer.get(buttonIdx).state);
        ++buttonIdx;
    }

    buttonDrv.unregisterObserver();
}

/**
 * A button state update shall notify the observer only if the state changed.
 */
static void testUpdateButtonNotifiesOnlyOnChange()
{
    ButtonDrv          buttonDrv;
    TestButtonObserver observer;

    TEST_ASSERT_TRUE(buttonDrv.init(gNcPin, gNcPin, gNcPin));

    buttonDrv.registerObserver(observer);
    observer.clear();

    /* Press */
    buttonDrv.updateButton(BUTTON_ID_OK, BUTTON_STATE_PRESSED);
    TEST_ASSERT_EQUAL_UINT8(1U, observer.getCount());
    TEST_ASSERT_EQUAL(BUTTON_ID_OK, observer.get(0U).buttonId);
    TEST_ASSERT_EQUAL(BUTTON_STATE_PRESSED, observer.get(0U).state);
    TEST_ASSERT_EQUAL(BUTTON_STATE_PRESSED, buttonDrv.getState(BUTTON_ID_OK));

    /* Same state again, no notification expected. */
    buttonDrv.updateButton(BUTTON_ID_OK, BUTTON_STATE_PRESSED);
    TEST_ASSERT_EQUAL_UINT8(1U, observer.getCount());

    /* Release */
    buttonDrv.updateButton(BUTTON_ID_OK, BUTTON_STATE_RELEASED);
    TEST_ASSERT_EQUAL_UINT8(2U, observer.getCount());
    TEST_ASSERT_EQUAL(BUTTON_ID_OK, observer.get(1U).buttonId);
    TEST_ASSERT_EQUAL(BUTTON_STATE_RELEASED, observer.get(1U).state);
    TEST_ASSERT_EQUAL(BUTTON_STATE_RELEASED, buttonDrv.getState(BUTTON_ID_OK));

    /* Other buttons are not affected. */
    TEST_ASSERT_EQUAL(BUTTON_STATE_RELEASED, buttonDrv.getState(BUTTON_ID_LEFT));
    TEST_ASSERT_EQUAL(BUTTON_STATE_RELEASED, buttonDrv.getState(BUTTON_ID_RIGHT));

    buttonDrv.unregisterObserver();
}

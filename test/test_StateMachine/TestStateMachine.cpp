/* MIT License
 *
 * Copyright (c) 2019 - 2023 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Test the abstract state machine.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <StateMachine.hpp>
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
 * Test state
 */
class TestState : public AbstractState
{
public:

    /**
     * Constructs the test state.
     */
    TestState() :
        m_callCntEntry(0U),
        m_callCntExit(0U),
        m_nextState(nullptr)
    {
    }

    /**
     * Destroys the test state.
     */
    ~TestState()
    {
    }

    /**
     * The entry is called once, a state is entered.
     *
     * @param[in] sm    Responsible state machine
     */
    void entry(StateMachine& sm)
    {
        ++m_callCntEntry;
        return;
    }

    /**
     * The process routine is called cyclic, as long as the state is active.
     *
     * @param[in] sm    Responsible state machine
     */

    void process(StateMachine& sm)
    {
        if (nullptr != m_nextState)
        {
            sm.setState(*m_nextState);
        }

        return;
    }

    /**
     * The exit is called once, a state will be left.
     *
     * @param[in] sm    Responsible state machine
     */
    void exit(StateMachine& sm)
    {
        ++m_callCntExit;
        return;
    }

    /**
     * Set next state.
     *
     * @param[in] nextState Next state to go in process method.
     */
    void setState(AbstractState& nextState)
    {
        m_nextState = &nextState;
        return;
    }

    /**
     * Get call counter for entry method.
     *
     * @return Call counter for entry method.
     */
    uint32_t getCallCntEntry() const
    {
        return m_callCntEntry;
    }

    /**
     * Get call counter for exit method.
     *
     * @return Call counter for exit method.
     */
    uint32_t getCallCntExit() const
    {
        return m_callCntExit;
    }

private:

    uint32_t        m_callCntEntry; /**< Call counter of entry method */
    uint32_t        m_callCntExit;  /**< Call counter of exit method */
    AbstractState*  m_nextState;    /**< Next state */
};

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static void testStateMachine();

/******************************************************************************
 * Local Variables
 *****************************************************************************/

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
extern int main(int argc, char **argv)
{
    UTIL_NOT_USED(argc);
    UTIL_NOT_USED(argv);

    UNITY_BEGIN();

    RUN_TEST(testStateMachine);

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
 * Test the abstract state machine.
 */
static void testStateMachine()
{
    TestState       stateA;
    TestState       stateB;
    StateMachine    sm;

    /* State machine has no state yet. */
    TEST_ASSERT_NULL(sm.getState());

    /* Add state A, but don't process it. */
    sm.setState(stateA);
    TEST_ASSERT_NULL(sm.getState());
    TEST_ASSERT_EQUAL_UINT32(0u, stateA.getCallCntEntry());
    TEST_ASSERT_EQUAL_UINT32(0u, stateA.getCallCntExit());

    /* Process it once.
     * Expectation:
     * The entry part is called once and the process part.
     */
    sm.process();
    TEST_ASSERT_EQUAL_UINT32(1u, stateA.getCallCntEntry());
    TEST_ASSERT_EQUAL_UINT32(0u, stateA.getCallCntExit());
    TEST_ASSERT_EQUAL_PTR(static_cast<AbstractState*>(&stateA), sm.getState());

    /* Process it a 2nd time.
     * Expectation:
     * Only the process part is called.
     */
    sm.process();
    TEST_ASSERT_EQUAL_UINT32(1u, stateA.getCallCntEntry());
    TEST_ASSERT_EQUAL_UINT32(0u, stateA.getCallCntExit());

    /* Transistion from A to B. */
    stateA.setState(stateB);
    sm.process();
    sm.process();
    TEST_ASSERT_EQUAL_UINT32(1u, stateA.getCallCntEntry());
    TEST_ASSERT_EQUAL_UINT32(1u, stateA.getCallCntExit());
    TEST_ASSERT_EQUAL_UINT32(1u, stateB.getCallCntEntry());
    TEST_ASSERT_EQUAL_UINT32(0u, stateB.getCallCntExit());

    /* Transistion from B to A. */
    stateB.setState(stateA);
    sm.process();
    sm.process();
    TEST_ASSERT_EQUAL_UINT32(2u, stateA.getCallCntEntry());
    TEST_ASSERT_EQUAL_UINT32(1u, stateA.getCallCntExit());
    TEST_ASSERT_EQUAL_UINT32(1u, stateB.getCallCntEntry());
    TEST_ASSERT_EQUAL_UINT32(1u, stateB.getCallCntExit());

    return;
}

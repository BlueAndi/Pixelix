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
 * @brief  Abstract state machine
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup statemachine
 *
 * @{
 */

#ifndef STATEMACHINE_HPP
#define STATEMACHINE_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stddef.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

class StateMachine;

/**
 * Abstract state interface.
 */
class AbstractState
{
public:

    /**
     * Constructs a state.
     */
    AbstractState()
    {
    }

    /**
     * Destroys a state.
     */
    virtual ~AbstractState()
    {
    }

    /**
     * The entry is called once, a state is entered.
     * 
     * @param[in] sm    Responsible state machine
     */
    virtual void entry(StateMachine& sm) = 0;

    /**
     * The process routine is called cyclic, as long as the state is active.
     * 
     * @param[in] sm    Responsible state machine
     */
    virtual void process(StateMachine& sm) = 0;

    /**
     * The exit is called once, a state will be left.
     * 
     * @param[in] sm    Responsible state machine
     */
    virtual void exit(StateMachine& sm) = 0;

private:

    AbstractState(const AbstractState& state);
    AbstractState& operator=(const AbstractState& state);
};

/**
 * Generic state machine.
 */
class StateMachine
{
public:

    /**
     * Constructs a state machine.
     */
    StateMachine() :
        m_currentState(nullptr),
        m_nextState(nullptr)
    {
    }

    /**
     * Constructs a state machine and set the starting state.
     * 
     * @param[in] state Starting state
     */
    explicit StateMachine(AbstractState& state) :
        m_currentState(nullptr),
        m_nextState(&state)
    {
    }

    /**
     * Get current state.
     * 
     * @return Current state
     */
    AbstractState* getState()
    {
        return m_currentState;
    }

    /**
     * Set next state.
     * 
     * @param[in] state Next state
     */
    void setState(AbstractState& state)
    {
        m_nextState = &state;
    }

    /**
     * Process the state machine.
     * It handles all state activities.
     */
    void process()
    {
        /* Request for change state available? */
        if (nullptr != m_nextState)
        {
            /* Leave current state */
            if (nullptr != m_currentState)
            {
                m_currentState->exit(*this);
            }

            /* Change state */
            m_currentState  = m_nextState;
            m_nextState     = nullptr;

            /* Enter new state */
            m_currentState->entry(*this);
        }
        /* Process current state */
        else if (nullptr != m_currentState)
        {
            m_currentState->process(*this);
        }
        else
        {
            /* Nothing to do */
            ;
        }
    }

private:

    AbstractState*  m_currentState; /**< Current active state */
    AbstractState*  m_nextState;    /**< Next state */

    StateMachine(const StateMachine& sm);
    StateMachine& operator=(const StateMachine& sm);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* STATEMACHINE_HPP */

/** @} */
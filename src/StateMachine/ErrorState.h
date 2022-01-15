/* MIT License
 *
 * Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>
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
 * @brief  System state: Error
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup sys_states
 * 
 * @{
 */

#ifndef __ERRORSTATE_H__
#define __ERRORSTATE_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <StateMachine.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * System state: Error
 */
class ErrorState : public AbstractState
{
public:

    /**
     * Get state instance.
     * 
     * @return State instance
     */
    static ErrorState& getInstance()
    {
        static ErrorState instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
    }

    /**
     * The entry is called once, a state is entered.
     * 
     * @param[in] sm    Responsible state machine
     */
    void entry(StateMachine& sm) final;

    /**
     * The process routine is called cyclic, as long as the state is active.
     * 
     * @param[in] sm    Responsible state machine
     */
    void process(StateMachine& sm) final;

    /**
     * The exit is called once, a state will be left.
     * 
     * @param[in] sm    Responsible state machine
     */
    void exit(StateMachine& sm) final;

    /**
     * Low level errors which can happen.
     * Low level means, it was before the display was initialized and the
     * display manager, as well as the system message handler, were not
     * active.
     */
    enum ErrorId
    {
        ERROR_ID_NO_ERROR = 0,      /**< No error */
        ERROR_ID_UNKNOWN,           /**< Unknown error */
        ERROR_ID_TWO_WIRE_ERROR,    /**< Two-wire (I2C) error */
        ERROR_ID_NO_USER_BUTTON,    /**< User button is not available */
        ERROR_ID_BAD_FS,            /**< Bad filesystem */
        ERROR_ID_DISP_MGR,          /**< Display manager error */
        ERROR_ID_SYS_MSG,           /**< System message handler error */
        ERROR_ID_UPDATE_MGR         /**< Update manager error */
    };

    /**
     * Set error cause, why this state will be entered.
     * 
     * @param[in] errorId   The error id of the root cause.
     */
    void setErrorId(ErrorId errorId)
    {
        m_errorId = errorId;
    }

    /**
     * Get current set error id.
     * 
     * @return Error id, which is set.
     */
    ErrorId getErrorId() const
    {
        return m_errorId;
    }

private:

    ErrorId m_errorId;  /**< The error cause, why this state is active. */

    /**
     * Constructs the state.
     */
    ErrorState() :
        m_errorId(ERROR_ID_NO_ERROR)
    {
    }

    /**
     * Destroys the state.
     */
    ~ErrorState()
    {
    }
    
    ErrorState(const ErrorState& state);
    ErrorState& operator=(const ErrorState& state);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __ERRORSTATE_H__ */

/** @} */
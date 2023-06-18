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
 * @brief  Buzzer driver
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "BuzzerDrv.h"
#include <Board.h>

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

void BuzzerDrv::stop()
{
    if (IoPin::NC != Board::buzzerOut.getPinNo())
    {
        if (true == m_isInit)
        {
            (void)ledcWriteTone(TONE_PWM_CHANNEL, 0U); /* Off */
            ledcDetachPin(Board::buzzerOut.getPinNo());

            m_isInit = false;
        }
    }
}

void BuzzerDrv::play(uint32_t freq)
{
    if (IoPin::NC != Board::buzzerOut.getPinNo())
    {
        if (false == m_isInit)
        {
            (void)ledcSetup(TONE_PWM_CHANNEL, INIT_FREQUENCY, DUTY_CYCLE_RESOLUTION_BITS);
            ledcAttachPin(Board::buzzerOut.getPinNo(), TONE_PWM_CHANNEL);

            m_isInit = true;
        }

        (void)ledcWriteTone(TONE_PWM_CHANNEL, freq); /* Note, it will set duty cycle 50%. */
        ledcWrite(TONE_PWM_CHANNEL, m_dutyCycle); /* Change duty cycle immediately. */
    }
}

void BuzzerDrv::play(uint32_t freq, uint8_t dc)
{
    if (IoPin::NC != Board::buzzerOut.getPinNo())
    {
        m_dutyCycle = (1023U * static_cast<uint32_t>(dc)) / 100U;

        if (false == m_isInit)
        {
            (void)ledcSetup(TONE_PWM_CHANNEL, INIT_FREQUENCY, DUTY_CYCLE_RESOLUTION_BITS);
            ledcAttachPin(Board::buzzerOut.getPinNo(), TONE_PWM_CHANNEL);

            m_isInit = true;
        }

        (void)ledcWriteTone(TONE_PWM_CHANNEL, freq); /* Note, it will set duty cycle 50%. */
        ledcWrite(TONE_PWM_CHANNEL, m_dutyCycle); /* Change duty cycle immediately. */
    }
}

void BuzzerDrv::changeDutyCycle(uint8_t dc)
{
    if (IoPin::NC != Board::buzzerOut.getPinNo())
    {
        m_dutyCycle = (1023U * static_cast<uint32_t>(dc)) / 100U;

        if (false == m_isInit)
        {
            (void)ledcSetup(TONE_PWM_CHANNEL, INIT_FREQUENCY, DUTY_CYCLE_RESOLUTION_BITS);
            ledcAttachPin(Board::buzzerOut.getPinNo(), TONE_PWM_CHANNEL);

            m_isInit = true;
        }

        ledcWrite(TONE_PWM_CHANNEL, m_dutyCycle);
    }
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


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
 * @brief  Ambient light sensor
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "SensorLdrGl5528.h"

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

/* Set threshold to detect that no LDR is connected.
 * Expected voltage is lower or equal than 3 mV.
 * This corresponds to the absolute dark resistance of the LDR with 1 MOhm.
 * 
 * Attention: This is only valid if a external pull-down resistor is connected.
 *            If the pin is floating, it will fail.
 */
const uint16_t  SensorLdrGl5528::NO_LDR_THRESHOLD   = (3UL * (Board::adcResolution - 1U)) / Board::adcRefVoltage;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

float LdrChannelIluminance::getValue()
{
    return m_driver->getIlluminance();
}

void SensorLdrGl5528::begin()
{
    const uint16_t ADC_UINT16 = Board::ldrIn.read();

    if (NO_LDR_THRESHOLD < ADC_UINT16)
    {
        m_isAvailable = true;
    }
    else
    {
        m_isAvailable = false;
    }
}

bool SensorLdrGl5528::isAvailable() const
{
    return m_isAvailable;
}

ISensorChannel* SensorLdrGl5528::getChannel(uint8_t index)
{
    ISensorChannel* channel = nullptr;

    if (true == m_isAvailable)
    {
        if (0U == index)
        {
            channel = &m_illuminanceChannel;
        }
    }

    return channel;
}

float SensorLdrGl5528::getIlluminance()
{
    const uint16_t  ADC_UINT16  = Board::ldrIn.read();
    float           illuminance = 0.0F;

    if (NO_LDR_THRESHOLD < ADC_UINT16)
    {
        /* LDR GL5528
         * from datasheet:
         * gamma gradient = 0.7
         * resistance at 10 Lux = 10 kOhm
         *
         * Calculation of R_LDR:
         * x = log10( I )
         * y = log10( R_LDR )
         *
         * Note, I is the illumiance and R_LDR is the ambient light depended resistance.
         *
         * y = m * x + b
         * log10( R_LDR ) = -(gamma gradient) * log10( I ) + b
         * log10( R_LDR ) = log10( I ^ -(gamma gradient) ) + b
         * log10( R_LDR ) = log10( I ^ -(gamma gradient) ) + log10( 10 ^ b )
         * log10( R_LDR ) = log10( ( I ^ -(gamma gradient) ) * ( 10 ^ b ) )
         * R_LDR = ( I ^ -(gamma gradient) ) * ( 10 ^ b )
         *
         * Calculation of the function axis section:
         * b = log10( R_LDR * I ^ (gamma gradient) )
         * b = log10 ( 10 kOhm * 10 Lux ^ 0.7 )
         * b = log10 ( 10^4 * 10^0.7 ) kOhm
         * b = 4 + 0.7
         * b = 4.7 kOhm
         *
         * Calculation of I:
         * R_LDR = ( I ^ -(gamma gradient) ) * ( 10 ^ b )
         * I ^ -(gamma gradient) = R_LDR / ( 10 ^ b )
         * 1 / ( I ^ (gamma gradient) ) = R_LDR / ( 10 ^ b )
         * I ^ (gamma gradient) = ( 10 ^ b ) / R_LDR
         * I = ( ( 10 ^ b ) / R_LDR ) ^ ( 1 / gamma gradient )
         * I = [ ( 10 ^ b ) ^ ( 1 / gamma gradient ) ] / [ R_LDR ^ ( 1 / gamma gradient ) ]
         * I = [ 10 ^ ( b / gamma gradient ) ] / R_LDR ^ ( 1 / gamma gradient )
         * I = [ 10 ^ ( b / gamma gradient ) ] * R_LDR ^ ( -1 / gamma gradient )
         * I = [ 10 ^ ( 4.7 / 0.7 ) ] * R_LDR ^ ( -1 / 0.7 )
         * I = 5179474.6792312 * R_LDR ^ -1.42857142857143 Lux
         */
        const float MULTIPLICATOR   = 5179474.6792312F;     /* 10 ^ ( b / gamma gradient ) */
        const float EXPONENT        = -1.42857142857143F;   /* -1 / gamma gradient */

        /* Calculation of R_LDR from the ADC value:
         * The schematic contains a voltage divider with R = 1 kOhm connected to GND.
         * The supply voltage Vcc is 3.3 V.
         * The ADC resolution is 4096.
         *
         * I = Vcc / ( R_LDR + R )
         *
         * V_R = R * I
         * V_R = R * ( Vcc / ( R_LDR + R ) )
         * V_R = R * Vcc / ( R_LDR + R )
         *
         * ADC = ( ADC resolution - 1 ) * V_R / Vcc
         * ADC = ( ADC resolution - 1 ) * R * Vcc / ( ( R_LDR + R ) * Vcc )
         * ADC = ( ADC resolution - 1 ) * R / ( R_LDR + R )
         * ADC * ( R_LDR + R ) = ( ADC resolution - 1 ) * R
         * ADC * R_LDR + ADC * R = ( ADC resolution - 1 ) * R
         * ADC * R_LDR = ( ADC resolution - 1 ) * R - ADC * R
         * R_LDR = [ ( ADC resolution - 1 ) * R - ADC * R ] / ADC
         * R_LDR = ( ADC_max * R - ADC * R ) / ADC
         *
         * Final calculation of I [Lux]:
         * I = 5179474.6792312 * R_LDR ^ -1.42857142857143
         * I = 5179474.6792312 * [ ( ADC_max * R - ADC * R ) / ADC ] ^ -1.42857142857143
         */
        const float ADC_MAX         = static_cast<float>(Board::adcResolution - 1U);
        const float R               = 1000.0F;  /* Resistor in the voltage divider, connected to GND. */
        const float ADC_FLOAT       = static_cast<float>(ADC_UINT16);

        illuminance = MULTIPLICATOR * powf( ( ADC_MAX * R - ADC_FLOAT * R ) / ADC_FLOAT, EXPONENT );
    }

    return illuminance;
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

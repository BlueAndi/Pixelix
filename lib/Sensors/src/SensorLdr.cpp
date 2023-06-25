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
 * @brief  Light depended resistor driver
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "SensorLdr.h"

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

/** LDR specific constants. */
typedef struct
{
    const char* name;           /**< User friendly name */
    const float multiplicator;  /**< Multiplicator used for illimunance calculation. Equ.: 10 ^ ( b / gamma gradient )*/
    const float exponent;       /**< Exponent used for illimunance calculation. Equ.: -1 / gamma gradient */

} Ldr;

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/**
 * The constants for all supported LDR types.
 * 
 * Get the required information from the datasheet:
 * - gamma gradient
 * - resistance at 10 Lux (use the average)
 *
 * Calculation of R_LDR:
 * x = log10( I )
 * y = log10( R_LDR )
 *
 * Note, I is the illuminance and R_LDR is the ambient light depended resistance.
 *
 * With logarithmic axis, its a straight line, therefore its equation can be used.
 * y = m * x + b
 * log10( R_LDR ) = -(gamma gradient) * log10( I ) + b
 * log10( R_LDR ) = log10( I ^ -(gamma gradient) ) + b
 * log10( R_LDR ) = log10( I ^ -(gamma gradient) ) + log10( 10 ^ b )
 * log10( R_LDR ) = log10( ( I ^ -(gamma gradient) ) * ( 10 ^ b ) )
 * R_LDR = ( I ^ -(gamma gradient) ) * ( 10 ^ b )
 *
 * Calculation of the function axis section:
 * b = y - m * x
 * b = log10( R_LDR ) - -(gamma gradient) * log10( I )
 * b = log10( R_LDR ) + log10( I ^ (gamma gradient) )
 * b = log10( R_LDR * ( I ^ (gamma gradient) ) )
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
 * 
 * I = MULTIPLICATOR * R_LDR ^ EXPONENT
 * 
 * => MULTIPLICATOR = 10 ^ ( b / gamma gradient )
 * => EXPONENT = -1 / gamma gradient
 *
 */
static const Ldr LDR_CONSTANTS[] =
{
    { "GL5516",     562500000.0F,       -2.0F       },
    { "GL5528",     91233029.9336F,     -1.6667F    },
    { "GL5537-1",   213746993.3346F,    -1.6667F    },
    { "GL5537-2",   37529382.2835F,     -1.4286F    },
    { "GL5539",     12411565.9487F,     -1.25F      },
    { "GL5549",     5639135.2390F,      -1.1111F    }
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

float LdrChannelIlluminance::getValue()
{
    return m_driver->getIlluminance() + m_offset;
}

void SensorLdr::begin()
{
    /* Depended on the current environment brightness, it may happen that the
     * LDR can not be detected by using a threshold for the analog input.
     * Therefore the availability is always to true.
     */
    m_isAvailable = true;
}

const char* SensorLdr::getName() const
{
    return LDR_CONSTANTS[m_ldrType].name;
}

bool SensorLdr::isAvailable() const
{
    return m_isAvailable;
}

ISensorChannel* SensorLdr::getChannel(uint8_t index)
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

float SensorLdr::getIlluminance()
{
    const uint16_t  ADC_UINT16      = Board::ldrIn.read();
    float           illuminance     = 0.0F;
    const float     MULTIPLICATOR   = LDR_CONSTANTS[m_ldrType].multiplicator;
    const float     EXPONENT        = LDR_CONSTANTS[m_ldrType].exponent;

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
    const float ADC_MAX             = static_cast<float>(Board::adcResolution - 1U);
    const float ADC_FLOAT           = static_cast<float>(ADC_UINT16);

    illuminance = MULTIPLICATOR * powf( ( ADC_MAX * m_resistance - ADC_FLOAT * m_resistance ) / ADC_FLOAT, EXPONENT );

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

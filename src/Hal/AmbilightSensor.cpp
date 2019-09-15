/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Ambilight sensor
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "AmbilightSensor.h"
#include "Board.h"

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

/* Set ambient light threshold in ADC digits for moonlit night.
 * Expected voltage is minium 46 mV for 1 lux.
 */
const uint16_t  AmbilightSensor::MOONLIT_NIGHT_ADC_THRESHOLD        = (46ul * (Board::adcResolution - 1u)) / Board::adcRefVoltage;

/* Set ambient light threshold in ADC digits for dark room.
 * Expected voltage is minimum 300 mV for 10 lux.
 */
const uint16_t  AmbilightSensor::DARK_ROOM_ADC_THRESHOLD            = (300ul * (Board::adcResolution - 1u)) / Board::adcRefVoltage;

/* Set ambient light threshold in ADC digits for dark overcast day.
 * Expected voltage is minimum 1320 mV for 100 lux.
 */
const uint16_t  AmbilightSensor::DARK_OVERCAST_DAY_ADC_THRESHOLD    = (1320ul * (Board::adcResolution - 1u)) / Board::adcRefVoltage;

/* Set ambient light threshold in ADC digits for overcast day.
 * Expected voltage is minimum 2538 mV for 1000 lux.
 */
const uint16_t  AmbilightSensor::OVERCAST_DAY_ADC_THRESHOLD         = (2538ul * (Board::adcResolution - 1u)) / Board::adcRefVoltage;

/* Set ambient light threshold in ADC digits for full daylight.
 * Expected voltage is minimum 3000 mV for 10000 lux.
 */
const uint16_t  AmbilightSensor::FULL_DAYLIGHT_ADC_THRESHOLD        = (3000ul * (Board::adcResolution - 1u)) / Board::adcRefVoltage;

/* Set hysterese in digits to 10 mV. */
const uint16_t  AmbilightSensor::HYSTERESE_DELTA                    = (10ul * (Board::adcResolution - 1u)) / Board::adcRefVoltage;

/* Initialize ambient light sensor */
AmbilightSensor AmbilightSensor::m_instance;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

AmbilightSensor::AmbientLightLevel AmbilightSensor::getAmbientLightLevel(void)
{
    uint16_t adcValue = Board::ldrIn.read();

    switch(m_lastLevel)
    {
    case AMBIENT_LIGHT_LEVEL_DARK:

        m_lastLevel = getAmbientLightLevelNoHysterese(adcValue);

        break;

    case AMBIENT_LIGHT_LEVEL_MOONLIT_NIGHT:

        if (MOONLIT_NIGHT_ADC_THRESHOLD > (adcValue + HYSTERESE_DELTA))
        {
            m_lastLevel = getAmbientLightLevelNoHysterese(adcValue);
        }
        else if (DARK_ROOM_ADC_THRESHOLD <= adcValue)
        {
            m_lastLevel = getAmbientLightLevelNoHysterese(adcValue);
        }
        else
        {
            /* Keep level */
            ;
        }
        
        break;

    case AMBIENT_LIGHT_LEVEL_DARK_ROOM:

        if (DARK_ROOM_ADC_THRESHOLD > (adcValue + HYSTERESE_DELTA))
        {
            m_lastLevel = getAmbientLightLevelNoHysterese(adcValue);
        }
        else if (DARK_OVERCAST_DAY_ADC_THRESHOLD <= adcValue)
        {
            m_lastLevel = getAmbientLightLevelNoHysterese(adcValue);
        }
        else
        {
            /* Keep level */
            ;
        }

        break;

    case AMBIENT_LIGHT_LEVEL_DARK_OVERCAST:

        if (DARK_OVERCAST_DAY_ADC_THRESHOLD > (adcValue + HYSTERESE_DELTA))
        {
            m_lastLevel = getAmbientLightLevelNoHysterese(adcValue);
        }
        else if (OVERCAST_DAY_ADC_THRESHOLD <= adcValue)
        {
            m_lastLevel = getAmbientLightLevelNoHysterese(adcValue);
        }
        else
        {
            /* Keep level */
            ;
        }

        break;

    case AMBIENT_LIGHT_LEVEL_OVERCAST_DAY:

        if (OVERCAST_DAY_ADC_THRESHOLD > (adcValue + HYSTERESE_DELTA))
        {
            m_lastLevel = getAmbientLightLevelNoHysterese(adcValue);
        }
        else if (FULL_DAYLIGHT_ADC_THRESHOLD <= adcValue)
        {
            m_lastLevel = getAmbientLightLevelNoHysterese(adcValue);
        }
        else
        {
            /* Keep level */
            ;
        }

        break;

    case AMBIENT_LIGHT_LEVEL_FULL_DAYLIGHT:

        if (FULL_DAYLIGHT_ADC_THRESHOLD > (adcValue + HYSTERESE_DELTA))
        {
            m_lastLevel = getAmbientLightLevelNoHysterese(adcValue);
        }
        else
        {
            /* Keep level */
            ;
        }

        break;

    default:
        break;
    }

    return m_lastLevel;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

AmbilightSensor::AmbientLightLevel AmbilightSensor::getAmbientLightLevelNoHysterese(uint16_t adcValue)
{
    AmbientLightLevel   level   = AMBIENT_LIGHT_LEVEL_DARK;

    if (FULL_DAYLIGHT_ADC_THRESHOLD <= adcValue)
    {
        level = AMBIENT_LIGHT_LEVEL_FULL_DAYLIGHT;
    }
    else if (DARK_OVERCAST_DAY_ADC_THRESHOLD <= adcValue)
    {
        level = AMBIENT_LIGHT_LEVEL_OVERCAST_DAY;
    }
    else if (DARK_OVERCAST_DAY_ADC_THRESHOLD <= adcValue)
    {
        level = AMBIENT_LIGHT_LEVEL_OVERCAST_DAY;
    }
    else if (DARK_ROOM_ADC_THRESHOLD <= adcValue)
    {
        level = AMBIENT_LIGHT_LEVEL_DARK_ROOM;
    }
    else if (MOONLIT_NIGHT_ADC_THRESHOLD <= adcValue)
    {
        level = AMBIENT_LIGHT_LEVEL_MOONLIT_NIGHT;
    }
    else
    {
        level = AMBIENT_LIGHT_LEVEL_DARK;
    }

    return level;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

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
 * 
 * @addtogroup hal
 *
 * @{
 */

#ifndef __AMBILIGHT_SENSOR_H__
#define __AMBILIGHT_SENSOR_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include <Board.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Ambilight sensor
 */
class AmbilightSensor
{
public:

    /**
     * Get ambilight sensor driver instance.
     * 
     * @return Ambilight sensor driver instance
     */
    static AmbilightSensor& getInstance(void)
    {
        return m_instance;
    }

    /**
     * Ambient light level
     */
    enum AmbientLightLevel
    {
        AMBIENT_LIGHT_LEVEL_DARK = 0,       /**< Dark */
        AMBIENT_LIGHT_LEVEL_MOONLIT_NIGHT,  /**< Moonlit night */
        AMBIENT_LIGHT_LEVEL_DARK_ROOM,      /**< Dark room */
        AMBIENT_LIGHT_LEVEL_DARK_OVERCAST,  /**< Dark overcast */
        AMBIENT_LIGHT_LEVEL_OVERCAST_DAY,   /**< Overcast day */
        AMBIENT_LIGHT_LEVEL_FULL_DAYLIGHT   /**< Full daylight */
    };

    /**
     * Get ambient light level.
     * 
     * @return Ambient light level
     */
    AmbientLightLevel getAmbientLightLevel(void);

    /**
     * Ambient light threshold in ADC digits for moonlit night.
     */
    static const uint16_t   MOONLIT_NIGHT_ADC_THRESHOLD;

    /**
     * Ambient light threshold in ADC digits for dark room.
     */
    static const uint16_t   DARK_ROOM_ADC_THRESHOLD;

    /**
     * Ambient light threshold in ADC digits for dark overcast day.
     */
    static const uint16_t   DARK_OVERCAST_DAY_ADC_THRESHOLD;

    /**
     * Ambient light threshold in ADC digits for overcast day.
     */
    static const uint16_t   OVERCAST_DAY_ADC_THRESHOLD;

    /**
     * Ambient light threshold in ADC digits for full daylight.
     */
    static const uint16_t   FULL_DAYLIGHT_ADC_THRESHOLD;

    /**
     * Hysterese in digits.
     */
    static const uint16_t   HYSTERESE_DELTA;

private:

    static AmbilightSensor  m_instance;     /**< Ambilight sensor instance */

    AmbientLightLevel   m_lastLevel;    /**< Last requested level */

    /* An instance shall not be copied. */
    AmbilightSensor(const AmbilightSensor& sensor);
    AmbilightSensor& operator=(const AmbilightSensor& sensor);

    /**
     * Constructs an ambilight instance.
     */
    AmbilightSensor() :
        m_lastLevel(AMBIENT_LIGHT_LEVEL_DARK)
    {
    }

    /**
     * Destroys the ambilight instance.
     */
    ~AmbilightSensor()
    {
    }

    /**
     * Get ambient light level, depended on ADC value and don't consider any hysterese.
     * 
     * @param[in] adcValue  Ambient light value in ADC digits
     * @return Ambient light level
     */
    AmbientLightLevel getAmbientLightLevelNoHysterese(uint16_t adcValue);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __AMBILIGHT_SENSOR_H__ */

/** @} */
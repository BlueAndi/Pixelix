/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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
 * 
 * @addtogroup hal
 *
 * @{
 */

#ifndef __AMBIENT_LIGHT_SENSOR_H__
#define __AMBIENT_LIGHT_SENSOR_H__

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
 * Ambient light sensor
 */
class AmbientLightSensor
{
public:

    /**
     * Get ambilight sensor driver instance.
     * 
     * @return Ambient light sensor driver instance
     */
    static AmbientLightSensor& getInstance(void)
    {
        return m_instance;
    }

    /**
     * Checks whether a sensor is available or not.
     * 
     * @return If a sensor is available, it will return true otherwise false.
     */
    bool isSensorAvailable(void);

    /**
     * Ambient light level
     * Source: https://docs.microsoft.com/de-de/windows-hardware/design/whitepapers/integrating-ambient-light-sensors-with-computers-running-windows-10-creators-update
     */
    enum AmbientLightLevel
    {
        AMBIENT_LIGHT_LEVEL_PITCH_BLACK = 0,    /**< Pitch black with 1 Lux */
        AMBIENT_LIGHT_LEVEL_NIGHT_SKY,          /**< Night sky with 10 Lux */
        AMBIENT_LIGHT_LEVEL_DARK_ROOM,          /**< Dark room with 50 Lux */
        AMBIENT_LIGHT_LEVEL_DARK_OVERCAST,      /**< Dark overcast with 500 Lux */
        AMBIENT_LIGHT_LEVEL_OVERCAST_DAY,       /**< Overcast day with 1000 Lux */
        AMBIENT_LIGHT_LEVEL_FULL_DAYLIGHT,      /**< Full daylight with 15000 Lux */
        AMBIENT_LIGHT_LEVEL_FULL_SUNLIGHT,      /**< Full sunlight with more than 15000 Lux */
        AMBIENT_LIGHT_LEVEL_MAX                 /**< Number of levels */
    };

    /**
     * Get ambient light level.
     * 
     * @return Ambient light level
     */
    AmbientLightLevel getAmbientLightLevel(void);

    /**
     * Get illuminance in Lux.
     * 
     * @return Illuminance level in Lux.
     */
    float getIlluminance(void);

    /**
     * Get the normalized light value in the range of 0.0f - 1.0f, from
     * 0 to 100000 Lux.
     * 
     * @return Normalized light value
     */
    float getNormalizedLight(void);

    /**
     * Threshold to detect a not connected LDR.
     */
    static const uint16_t NO_LDR_THRESHOLD;

private:

    static AmbientLightSensor  m_instance;     /**< Ambient light sensor instance */

    /* An instance shall not be copied. */
    AmbientLightSensor(const AmbientLightSensor& sensor);
    AmbientLightSensor& operator=(const AmbientLightSensor& sensor);

    /**
     * Constructs an ambilight instance.
     */
    AmbientLightSensor()
    {
    }

    /**
     * Destroys the ambilight instance.
     */
    ~AmbientLightSensor()
    {
    }

    /**
     * Calculate the normalized light value in the range of 0.0f - 1.0f, from
     * 0 to 100000 Lux.
     * 
     * @param[in] illuminance   Illuminance in Lux
     * @return Normalized light value
     */
    float calcNormalizedLight(float illuminance);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __AMBIENT_LIGHT_SENSOR_H__ */

/** @} */
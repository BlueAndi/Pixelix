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
 * @brief  Ambient light functions
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef AMBIENT_LIGHT_H
#define AMBIENT_LIGHT_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Ambient light functions, based on illuminance, measured by a external sensor.
 */
namespace AmbientLight
{

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
 * Calculuate normalized light value in range of 0.0F - 1.0F, which
 * corresponds from 0 to 100000 lux.
 *
 * @param[in] illuminance   Illuminance in lux
 * 
 * @return Normalized light value
 */
extern float normalizeIlluminance(float illuminance);

/**
 * Get ambient light level.
 *
 * @param[in] illuminance   Illuminance in lux
 * 
 * @return Ambient light level
 */
extern AmbientLightLevel getAmbientLightLevel(float illuminance);

}

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* AMBIENT_LIGHT_H */

/** @} */
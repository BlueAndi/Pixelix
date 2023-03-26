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
 * @brief  Ambient light sensor
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "AmbientLight.h"
#include <math.h>

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

/**
 * Ambient light level definitions.
 */
static const float  gAmbientLightLevels[AmbientLight::AMBIENT_LIGHT_LEVEL_MAX] =
{
    /* Pitch black with 1 Lux */
    1.0F,
    /* Night sky with 10 Lux */
    10.0F,
    /* Dark room with 50 Lux */
    50.0F,
    /* Dark overcast with 500 Lux */
    500.0F,
    /* Overcast day with 1000 Lux */
    1000.0F,
    /* Full daylight with 15000 Lux */
    15000.0F,
    /* Full sunlight with more than 15000 Lux */
    15000.0F
};

/**
 * Lower limit for light luminance normalization in lux.
 */
static const float  LIMIT_LOW   = 1.0F;

/**
 * Upper limit for light luminance normalization in lux.
 */
static const float  LIMIT_HIGH  = 100000.0F;

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

extern float AmbientLight::normalizeIlluminance(float illuminance)
{
    const float LIGHT_NORM_MIN  = 0.0F;
    const float LIGHT_NORM_MAX  = 1.0F;
    float       lightNormalized = 0.0F; /* Range: 0.0F - 1.0F */

    if (LIMIT_LOW > illuminance)
    {
        lightNormalized = LIGHT_NORM_MIN;
    }
    else if (LIMIT_HIGH < illuminance)
    {
        lightNormalized = LIGHT_NORM_MAX;
    }
    else
    {
        /* Map lux values to human perception, according to
         * https://docs.microsoft.com/en-us/windows/win32/sensorsapi/understanding-and-interpreting-lux-values
         */
        lightNormalized = log10f(illuminance) / 5.0F; // NOLINT (readability-magic-numbers)
    }

    return lightNormalized;
}

extern AmbientLight::AmbientLightLevel AmbientLight::getAmbientLightLevel(float illuminance)
{
    uint8_t             levelIndex  = 0U;
    AmbientLightLevel   level       = AMBIENT_LIGHT_LEVEL_MAX;

    while((AMBIENT_LIGHT_LEVEL_MAX >= levelIndex) && (AMBIENT_LIGHT_LEVEL_MAX == level))
    {
        if (gAmbientLightLevels[levelIndex] >= illuminance)
        {
            level = static_cast<AmbientLightLevel>(levelIndex);
        }

        ++levelIndex;
    }

    if (AMBIENT_LIGHT_LEVEL_MAX == level)
    {
        level = AMBIENT_LIGHT_LEVEL_FULL_SUNLIGHT;
    }

    return level;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

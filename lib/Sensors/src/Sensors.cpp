/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Sensors
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Sensors.h"

#include <Util.h>
#include <SensorLdr.h>
#include <SensorSht3X.h>
#include <SensorDhtX.h>
#include <SensorBattery.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/**
 * Macro is used to check whether a sensor is enabled or disabled.
 */
#define IS_ENABLED(_configSensorSwitch) ((_configSensorSwitch) != 0)

/**
 * Calculates the next sensor id from the last one.
 */
#define SENSOR_ID_NEXT(_id)             ((_id) + 1)

/**
 * Route the last sensor id through.
 */
#define SENSOR_ID_NOT_APPLICABLE(_id)   (_id)

#if IS_ENABLED(CONFIG_SENSOR_LDR_ENABLE)
/**
 * LDR sensor id.
 * Its 0 because its the first sensor in the gSensors list.
 */
#define SENSOR_ID_LDR       0
#else   /* IS_ENABLED(CONFIG_SENSOR_LDR_ENABLE) */
/**
 * LDR sensor id.
 * Its -1 because its the first sensor in the gSensors list, but not used.
 */
#define SENSOR_ID_LDR       -1
#endif /* IS_ENABLED(CONFIG_SENSOR_LDR_ENABLE) */

#if IS_ENABLED(CONFIG_SENSOR_SHT3X_ENABLE)
/**
 * SHT3x sensor id.
 */
#define SENSOR_ID_SHT3X     SENSOR_ID_NEXT(SENSOR_ID_LDR)
#else   /* IS_ENABLED(CONFIG_SENSOR_SHT3X_ENABLE) */
/**
 * SHT3x sensor id (not applicable).
 */
#define SENSOR_ID_SHT3X     SENSOR_ID_NOT_APPLICABLE(SENSOR_ID_LDR)
#endif  /* IS_ENABLED(CONFIG_SENSOR_SHT3X_ENABLE) */

#if IS_ENABLED(CONFIG_SENSOR_DHT11_ENABLE)
/**
 * DHT11 sensor id.
 */
#define SENSOR_ID_DHT11     SENSOR_ID_NEXT(SENSOR_ID_SHT3X)
#else   /* IS_ENABLED(CONFIG_SENSOR_DHT11_ENABLE) */
/**
 * DHT11 sensor id (not applicable).
 */
#define SENSOR_ID_DHT11     SENSOR_ID_NOT_APPLICABLE(SENSOR_ID_SHT3X)
#endif  /* IS_ENABLED(CONFIG_SENSOR_DHT11_ENABLE) */

#if IS_ENABLED(CONFIG_SENSOR_BATTERY_ENABLE)
/**
 * Battery sensor id.
 */
#define SENSOR_ID_BATTERY   SENSOR_ID_NEXT(SENSOR_ID_DHT11)
#else   /* IS_ENABLED(CONFIG_SENSOR_BATTERY_ENABLE) */
/**
 * Battery sensor id (not applicable).
 */
#define SENSOR_ID_BATTERY   SENSOR_ID_NOT_APPLICABLE(SENSOR_ID_DHT11)
#endif  /* IS_ENABLED(CONFIG_SENSOR_BATTERY_ENABLE) */

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

#if IS_ENABLED(CONFIG_SENSOR_LDR_ENABLE)

/** The LDR is used for automatic display brightness control. */
static SensorLdr        gLdr(CONFIG_SENSOR_LDR, CONFIG_SENSOR_LDR_SERIES_RESISTANCE);

#endif /* IS_ENABLED(CONFIG_SENSOR_LDR_ENABLE) */

#if IS_ENABLED(CONFIG_SENSOR_SHT3X_ENABLE)

/** The SHT3x sensor in autodetect mode (for two-wire sensors only). */
static SensorSht3X      gSht3x(SHTSensor::AUTO_DETECT);

#endif /* IS_ENABLED(CONFIG_SENSOR_SHT3X_ENABLE) */

#if IS_ENABLED(CONFIG_SENSOR_DHT11_ENABLE)

/** The DHT11 sensor. */
static SensorDhtX       gDht11(SensorDhtX::MODEL_DHT11);

#endif /* IS_ENABLED(CONFIG_SENSOR_DHT11_ENABLE) */

#if IS_ENABLED(CONFIG_SENSOR_BATTERY_ENABLE)

/** Battery sensor. */
static SensorBattery    gBattery;

#endif /* IS_ENABLED(CONFIG_SENSOR_BATTERY_ENABLE) */

/** A list with all registered sensors. */
static ISensor*         gSensors[] =
{
#if IS_ENABLED(CONFIG_SENSOR_LDR_ENABLE)
    &gLdr,
#endif /* IS_ENABLED(CONFIG_SENSOR_LDR_ENABLE) */

#if IS_ENABLED(CONFIG_SENSOR_SHT3X_ENABLE)
    &gSht3x,
#endif /* IS_ENABLED(CONFIG_SENSOR_SHT3X_ENABLE) */

#if IS_ENABLED(CONFIG_SENSOR_DHT11_ENABLE)
    &gDht11,
#endif /* IS_ENABLED(CONFIG_SENSOR_DHT11_ENABLE) */

#if IS_ENABLED(CONFIG_SENSOR_BATTERY_ENABLE)
    &gBattery,
#endif /* IS_ENABLED(CONFIG_SENSOR_BATTERY_ENABLE) */
    nullptr
};

/** The concrete sensor data provider implementation. */
static SensorDataProviderImpl   gSensorDataProviderImpl(gSensors, UTIL_ARRAY_NUM(gSensors) - 1U);

/**
 * Default offset table, used to initialize the sensor channel offset once in the very
 * first startup in the SensorDataProvider.
 */
static const SensorChannelDefaultValue gSensorDefaultValues[] =
{
    /*  Sensor id           Channel id  Value as JSON string */
#if IS_ENABLED(CONFIG_SENSOR_SHT3X_ENABLE)
    {   SENSOR_ID_SHT3X,    0U,         "{ offset: -9 }"  }    /* SHT3x temperature offset */
#endif /* IS_ENABLED(CONFIG_SENSOR_SHT3X_ENABLE) */
};

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

extern SensorDataProviderImpl* Sensors::getSensorDataProviderImpl()
{
    return &gSensorDataProviderImpl;
}

extern const SensorChannelDefaultValue* Sensors::getSensorChannelDefaultValues(uint8_t& values)
{
    values = UTIL_ARRAY_NUM(gSensorDefaultValues);

    return gSensorDefaultValues;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

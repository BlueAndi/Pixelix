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
 * @brief  Board Abstraction
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup hal
 *
 * @{
 */

#ifndef __BOARD_H__
#define __BOARD_H__

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Io.hpp"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/** Electronic board abstraction */
namespace Board
{

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Variables
 *****************************************************************************/

/** Pin number of all used pins. */
namespace Pin
{
    /** Pin number of onboard LED */
    static const uint8_t    onBoardLedPinNo         = LED_BUILTIN;

    /** Pin number of user button */
    static const uint8_t    userButtonPinNo         = 4U;

#ifdef BOARD_ADAFRUIT_FEATHER_ESP32_V2
    /** Pin number of dht sensor in */
    static const uint8_t    dhtInPinNo              = 25U;
#else
    /** Pin number of dht sensor in */
    static const uint8_t    dhtInPinNo              = 5U;
#endif

    /** Pin number of I2C SDA */
    static const uint8_t    i2cSdaPinNo             = 21U;

    /** Pin number of I2C SCL */
    static const uint8_t    i2cSclPinNo             = 22U;

#ifdef BOARD_ADAFRUIT_FEATHER_ESP32_V2
    /** Pin number of test pin */
    static const uint8_t    testPinNo               = 32U;
#else
    /** Pin number of test pin */
    static const uint8_t    testPinNo               = 23U;
#endif

#ifdef BOARD_ADAFRUIT_FEATHER_ESP32_V2
    /** Pin number of I2S word select (chooses between left and right channel) */
    static const uint8_t    i2sWordSelect           = 39U;
#else
    /** Pin number of I2S word select (chooses between left and right channel) */
    static const uint8_t    i2sWordSelect           = 25U;
#endif

#ifdef BOARD_ADAFRUIT_FEATHER_ESP32_V2
    /** Pin number of I2S serial clock (bit clock line BCLK) */
    static const uint8_t    i2sSerialClock          = 36U;
#else
    /** Pin number of I2S serial clock (bit clock line BCLK) */
    static const uint8_t    i2sSerialClock          = 26U;
#endif

    /** Pin number of LED matrix data out */
    static const uint8_t    ledMatrixDataOutPinNo   = 27U;

    /** Pin number of I2S serial data (payload is transmitted in 2 complements). */
    static const uint8_t    i2sSerialDataIn         = 33U;

    /** Pin number of LDR in */
    static const uint8_t    ldrInPinNo              = 34U;
};

/** Digital output pin: Onboard LED */
static const DOutPin<Pin::onBoardLedPinNo>              onBoardLedOut;

/** Digital input pin: User button (input with pull-up) */
static const DInPin<Pin::userButtonPinNo, INPUT_PULLUP> userButtonIn;

/** Digital output pin: Test pin (only for debug purposes) */
static const DOutPin<Pin::testPinNo>                    testPinOut;

/** Digital output pin: LED matrix data out */
static const DOutPin<Pin::ledMatrixDataOutPinNo>        ledMatrixDataOut;

/** Analog input pin: LDR in */
static const AnalogPin<Pin::ldrInPinNo>                 ldrIn;

/** Digital input pin: DHT Sensor (input with pull-up) */
static const DInPin<Pin::dhtInPinNo, INPUT_PULLUP>      dhtIn;

/** ADC resolution in digits */
static const uint16_t   adcResolution   = 4096U;

/** ADC reference voltage in mV */
static const uint16_t   adcRefVoltage   = 3300U;

/** LED matrix specific values */
namespace LedMatrix
{

/** LED matrix width in pixels */
static const uint8_t    width               = 32U;

/** LED matrix height in pixels */
static const uint8_t    height              = 8U;

/** LED matrix supply voltage in volt */
static const uint8_t    supplyVoltage       = 5U;

/** LED matrix max. supply current in mA */
static const uint32_t   supplyCurrentMax    = 3500U;

/** Max. current in mA per LED */
static const uint32_t   maxCurrentPerLed    = 60U;

};

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Initialize all i/o pins.
 */
extern void init();

/**
 * Execute a hard reset!
 */
extern void reset();

};

#endif  /* __BOARD_H__ */

/** @} */
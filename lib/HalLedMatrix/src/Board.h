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
 * @brief  Board Abstraction
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup hal
 *
 * @{
 */

#ifndef BOARD_H
#define BOARD_H

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
    constexpr uint8_t   onBoardLedPinNo         = LED_BUILTIN;

    /** Pin number of user button */
    constexpr uint8_t   userButtonPinNo         = 4U;

    /** Pin number of dht sensor in */
    constexpr uint8_t   dhtInPinNo              = 5U;

#ifdef ARDUINO_LOLIN_S2_MINI
    /** Pin number of I2C SDA */
    constexpr uint8_t   i2cSdaPinNo             = 33U;

    /** Pin number of I2C SCL */
    constexpr uint8_t   i2cSclPinNo             = 35U;
#else
    /** Pin number of I2C SDA */
    constexpr uint8_t   i2cSdaPinNo             = 21U;

    /** Pin number of I2C SCL */
    constexpr uint8_t   i2cSclPinNo             = 22U;
#endif

#ifdef ARDUINO_ADAFRUIT_FEATHER_ESP32_V2
    /** Pin number of test pin */
    constexpr uint8_t   testPinNo               = 32U;
#else
    /** Pin number of test pin */
    constexpr uint8_t   testPinNo               = 23U;
#endif

#ifndef ARDUINO_LOLIN_S2_MINI
    /** Pin number of I2S word select (chooses between left and right channel) */
    constexpr uint8_t   i2sWordSelect           = 25U;

    /** Pin number of I2S serial clock (bit clock line BCLK) */
    constexpr uint8_t   i2sSerialClock          = 26U;
#endif

#ifdef ARDUINO_LOLIN_S2_MINI
    /** Pin number of LED matrix data out */
    constexpr uint8_t   ledMatrixDataOutPinNo   = 21U;
#else
    /** Pin number of LED matrix data out */
    constexpr uint8_t   ledMatrixDataOutPinNo   = 27U;
#endif

#ifndef ARDUINO_LOLIN_S2_MINI
    /** Pin number of I2S serial data (payload is transmitted in 2 complements). */
    constexpr uint8_t   i2sSerialDataIn         = 33U;
#endif

#ifdef ARDUINO_LOLIN_S2_MINI
    /** Pin number of LDR in */
    constexpr uint8_t   ldrInPinNo              = 1U;
#else
    /** Pin number of LDR in */
    constexpr uint8_t   ldrInPinNo              = 34U;
#endif
};

/* Digital output pin: Onboard LED */
extern const DOutPin<Pin::onBoardLedPinNo>              onBoardLedOut;

/* Digital input pin: User button (input with pull-up) */
extern const DInPin<Pin::userButtonPinNo, INPUT_PULLUP> userButtonIn;

/* Digital output pin: Test pin (only for debug purposes) */
extern const DOutPin<Pin::testPinNo>                    testPinOut;

/* Digital output pin: LED matrix data out */
extern const DOutPin<Pin::ledMatrixDataOutPinNo>        ledMatrixDataOut;

/* Analog input pin: LDR in */
extern const AnalogPin<Pin::ldrInPinNo>                 ldrIn;

/* Digital input pin: DHT Sensor (input with pull-up) */
extern const DInPin<Pin::dhtInPinNo, INPUT_PULLUP>      dhtIn;

/** ADC resolution in digits */
constexpr uint16_t  adcResolution           = 4096U;

/** ADC reference voltage in mV */
constexpr uint16_t  adcRefVoltage           = 3300U;

/** LED matrix specific values */
namespace LedMatrix
{

    /** LED matrix width in pixels */
    constexpr uint8_t   width               = 32U;

    /** LED matrix height in pixels */
    constexpr uint8_t   height              = 8U;

    /** LED matrix supply voltage in volt */
    constexpr uint8_t   supplyVoltage       = 5U;

    /** LED matrix max. supply current in mA */
    constexpr uint32_t  supplyCurrentMax    = 3500U;

    /** Max. current in mA per LED */
    constexpr uint32_t  maxCurrentPerLed    = 60U;

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

/**
 * Switch onboard LED on.
 */
extern void ledOn();

/**
 * Switch onboard LED off.
 */
extern void ledOff();

/**
 * Is the onboard LED on?
 * 
 * @return If onboard LED is on, it will return true otherwise false.
 */
extern bool isLedOn();

};

#endif  /* BOARD_H */

/** @} */
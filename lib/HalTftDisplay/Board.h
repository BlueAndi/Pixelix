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
#if defined(BOARD_LILYGO_T_DISPLAY_S3)
    /** Pin number of onboard LED */
    constexpr uint8_t    onBoardLedPinNo         = IoPin::NC;  // Not available

    /** Pin number of I2C SDA */
    constexpr uint8_t    i2cSdaPinNo             = 18U;

    /** Pin number of I2C SDL */
    constexpr uint8_t    i2cSclPinNo             = 17U;

    /** Pin number of I2S word select (chooses between left and right channel) */
    constexpr uint8_t    i2sWordSelect           = 12U;

    /** Pin number of I2S serial clock (bit clock line BCLK) */
    constexpr uint8_t    i2sSerialClock          = 11U;

    /** Pin number of dht sensor in */
    constexpr uint8_t    dhtInPinNo              = 21U;

    /** Pin number of LDR in */
    constexpr uint8_t    ldrInPinNo              = 10U;

    /** Pin number of I2S serial data (payload is transmitted in 2 complements). */
    constexpr uint8_t    i2sSerialDataIn         = 18U;

    /** Pin number of user button */
    constexpr uint8_t    userButtonPinNo         = 14U;

#else //defined(BOARD_LILYGO_T_DISPLAY_S3)

    /** Pin number of onboard LED */
    constexpr uint8_t   onBoardLedPinNo         = 2U;

    /** Pin number of I2C SDA */
    constexpr uint8_t   i2cSdaPinNo             = 21U;

    /** Pin number of I2C SDL */
    constexpr uint8_t   i2cSclPinNo             = 22U;

    /** Pin number of I2S word select (chooses between left and right channel) */
    constexpr uint8_t   i2sWordSelect           = 25U;

    /** Pin number of I2S serial clock (bit clock line BCLK) */
    constexpr uint8_t   i2sSerialClock          = 26U;

    /** Pin number of dht sensor in */
    constexpr uint8_t   dhtInPinNo              = 27U;

    /** Pin number of LDR in */
    constexpr uint8_t   ldrInPinNo              = 32U;

    /** Pin number of I2S serial data (payload is transmitted in 2 complements). */
    constexpr uint8_t   i2sSerialDataIn         = 33U;

    /** Pin number of user button */
    constexpr uint8_t   userButtonPinNo         = 35U;
#endif //!defined(BOARD_LILYGO_T_DISPLAY_S3)
};

/* Digital output pin: Onboard LED */
extern const DOutPin<Pin::onBoardLedPinNo>              onBoardLedOut;

/* Digital input pin: User button (input with pull-up) */
extern const DInPin<Pin::userButtonPinNo, INPUT_PULLUP> userButtonIn;

/* Analog input pin: LDR in */
extern const AnalogPin<Pin::ldrInPinNo>                 ldrIn;

/* Digital input pin: DHT Sensor (input with pull-up) */
extern const DInPin<Pin::dhtInPinNo, INPUT_PULLUP>      dhtIn;

/** ADC resolution in digits */
constexpr uint16_t  adcResolution   = 4096U;

/** ADC reference voltage in mV */
constexpr uint16_t  adcRefVoltage   = 3300U;

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
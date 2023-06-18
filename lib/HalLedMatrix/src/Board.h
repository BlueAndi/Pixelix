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
    constexpr uint8_t   onBoardLedPinNo         = CONFIG_PIN_ONBOARD_LED;

    /** Pin number of button "ok" (former user button) */
    constexpr uint8_t   buttonOkPinNo           = CONFIG_PIN_BUTTON_OK;

    /** Pin number of button "left" */
    constexpr uint8_t   buttonLeftPinNo         = CONFIG_PIN_BUTTON_LEFT;

    /** Pin number of button "right" */
    constexpr uint8_t   buttonRightPinNo        = CONFIG_PIN_BUTTON_RIGHT;

    /** Pin number of dht sensor in */
    constexpr uint8_t   dhtInPinNo              = CONFIG_PIN_DHT_IN;

    /** Pin number of I2C SDA */
    constexpr uint8_t   i2cSdaPinNo             = CONFIG_PIN_I2C_SDA;

    /** Pin number of I2C SCL */
    constexpr uint8_t   i2cSclPinNo             = CONFIG_PIN_I2C_SCL;

    /** Pin number of test pin */
    constexpr uint8_t   testPinNo               = CONFIG_PIN_TEST;

    /** Pin number of I2S word select (chooses between left and right channel) */
    constexpr uint8_t   i2sWordSelect           = CONFIG_PIN_I2S_WS;

    /** Pin number of I2S serial clock (bit clock line BCLK) */
    constexpr uint8_t   i2sSerialClock          = CONFIG_PIN_I2S_SC;

    /** Pin number of I2S serial data (payload is transmitted in 2 complements). */
    constexpr uint8_t   i2sSerialDataIn         = CONFIG_PIN_I2S_DI;

    /** Pin number of LDR in */
    constexpr uint8_t   ldrInPinNo              = CONFIG_PIN_LDR_IN;

    /** Pin number of battery voltage in */
    constexpr uint8_t   batteryInPinNo          = CONFIG_PIN_BATTERY_IN;

    /** Pin number of LED matrix data out */
    constexpr uint8_t   ledMatrixDataOutPinNo   = CONFIG_PIN_LED_MATRIX_OUT;

    /** Pin number of buzzer out */
    constexpr uint8_t   buzzerOutPinNo          = CONFIG_PIN_BUZZER_OUT;
};

/* Digital output pin: Onboard LED */
extern const DOutPin<Pin::onBoardLedPinNo>                  onBoardLedOut;

/* Digital input pin: Button "ok" (input with pull-up) */
extern const DInPin<Pin::buttonOkPinNo, INPUT_PULLUP>       buttonOkIn;

/* Digital input pin: Button "left" (input with pull-up) */
extern const DInPin<Pin::buttonLeftPinNo, INPUT_PULLUP>     buttonLeftIn;

/* Digital input pin: Button "right" (input with pull-up) */
extern const DInPin<Pin::buttonRightPinNo, INPUT_PULLUP>    buttonRightIn;

/* Digital output pin: Test pin (only for debug purposes) */
extern const DOutPin<Pin::testPinNo>                        testPinOut;

/* Analog input pin: LDR in */
extern const AnalogPin<Pin::ldrInPinNo>                     ldrIn;

/* Digital input pin: DHT Sensor (input with pull-up) */
extern const DInPin<Pin::dhtInPinNo, INPUT_PULLUP>          dhtIn;

/* Analog input pin: battery voltage in */
extern const AnalogPin<Pin::batteryInPinNo>                 batteryVoltageIn;

/* Digital output pin: LED matrix data out */
extern const DOutPin<Pin::ledMatrixDataOutPinNo>            ledMatrixDataOut;

/* Digital output pin: Buzzer */
extern const DOutPin<Pin::buzzerOutPinNo>                   buzzerOut;

/** ADC resolution in digits */
constexpr uint16_t  adcResolution               = 4096U;

/** ADC reference voltage in mV */
constexpr uint16_t  adcRefVoltage               = 3300U;

/** LED matrix specific values */
namespace LedMatrix
{

    /** LED matrix width in pixels */
    constexpr uint8_t   width               = CONFIG_LED_MATRIX_WIDTH;

    /** LED matrix height in pixels */
    constexpr uint8_t   height              = CONFIG_LED_MATRIX_HEIGHT;

    /** LED matrix supply voltage in volt */
    constexpr uint8_t   supplyVoltage       = 5U;

    /** LED matrix max. supply current in mA */
    constexpr uint32_t  supplyCurrentMax    = CONFIG_SUPPLY_CURRENT;

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
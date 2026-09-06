/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   Arduino.h
 * @brief  Arduino stuff for test
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef ARDUINO_H
#define ARDUINO_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <math.h>
#include <cmath>
#include <array>

#include "WString.h"
#include "Print.h"
#include "HardwareSerial.h"
#include "IPAddress.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Digital pin level low */
#define LOW (0x0)

/** Digital pin level high */
#define HIGH (0x1)

/** Pin mode: digital input */
#define INPUT (0x01)

/** Pin mode: digital output */
#define OUTPUT (0x03)

/** Pin mode: digital input with pull-up */
#define INPUT_PULLUP (0x05)

/** Pin mode: digital input with pull-down */
#define INPUT_PULLDOWN (0x09)

/** Pin mode: analog input */
#define ANALOG (0xC0)

/* There is no separate program memory on the host, therefore every PROGMEM
 * helper of the Arduino just addresses the standard memory.
 */

/** Pointer to a string in the program memory. */
#define PGM_P const char*

/** Pointer to a string in the program memory. */
#define PGM_VOID_P const void*

/** Mark a variable to be placed in the program memory. */
#ifndef PROGMEM
#define PROGMEM
#endif /* PROGMEM */

/** Place a string literal in the program memory. */
#define PSTR(str) (str)

/** Place a string literal in the program memory, used by the Arduino. */
#define F(str) (str)

/** Type of a string in the program memory. */
#define FPSTR(str) (reinterpret_cast<const char*>(str))

/** Read a byte from the program memory. */
#define pgm_read_byte(addr) (*reinterpret_cast<const uint8_t*>(addr))

/** Read a word from the program memory. */
#define pgm_read_word(addr) (*reinterpret_cast<const uint16_t*>(addr))

/** Read a dword from the program memory. */
#define pgm_read_dword(addr) (*reinterpret_cast<const uint32_t*>(addr))

/** Read a pointer from the program memory. */
#define pgm_read_ptr(addr) (*reinterpret_cast<void* const*>(addr))

/** Copy from the program memory. */
#define memcpy_P memcpy

/** Get the length of a string in the program memory. */
#define strlen_P strlen

/** Copy a string from the program memory. */
#define strcpy_P strcpy

/** Copy a string from the program memory. */
#define strncpy_P strncpy

/** Concatenate a string from the program memory. */
#define strcat_P strcat

/** Compare with a string in the program memory. */
#define strcmp_P strcmp

/** Compare with a string in the program memory. */
#define strncmp_P strncmp

/** Search for a string in the program memory. */
#define strstr_P strstr

/** Print formatted into a buffer, format string in the program memory. */
#define snprintf_P snprintf

/** Print formatted into a buffer, format string in the program memory. */
#define sprintf_P sprintf

/** Print formatted into a buffer, format string in the program memory. */
#define vsnprintf_P vsnprintf

/** Pin mode: open drain output */
#define OPEN_DRAIN (0x13)

/** Pin mode: digital output, open drain */
#define OUTPUT_OPEN_DRAIN (0x13)

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

#ifndef __RPCNDR_H__

/** Arduino boolean */
typedef bool boolean;

#else /* __RPCNDR_H__ */

/* The Windows SDK provides boolean as unsigned char already, see rpcndr.h.
 * It is pulled in by winsock, which the native AsyncTCP is based on.
 */

#endif /* __RPCNDR_H__ */

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Get timestamp in ms since the program is running.
 *
 * @return Timestamp in ms
 */
extern unsigned long millis();

/**
 * Get timestamp for log output in ms.
 *
 * @return Timestamp in ms
 */
extern uint32_t esp_log_timestamp(void);

/**
 * Suspend the program for the given time.
 *
 * @param[in] ms    Time in ms
 */
extern void delay(unsigned long ms);

/**
 * Get timestamp in us since the program is running.
 *
 * @return Timestamp in us
 */
extern unsigned long micros();

/**
 * Configure the mode of a digital pin.
 * There is no GPIO on the host, therefore the mode is just remembered.
 *
 * @param[in] pinNo Pin number
 * @param[in] mode  Pin mode
 */
extern void pinMode(uint8_t pinNo, uint8_t mode);

/**
 * Set the level of a digital output pin.
 * There is no GPIO on the host, therefore the level is just remembered.
 *
 * @param[in] pinNo Pin number
 * @param[in] level Pin level, LOW or HIGH
 */
extern void digitalWrite(uint8_t pinNo, uint8_t level);

/**
 * Get the level of a digital pin.
 *
 * @param[in] pinNo Pin number
 *
 * @return Pin level, LOW or HIGH
 */
extern int digitalRead(uint8_t pinNo);

/**
 * Get the value of a analog input pin.
 * There is no ADC on the host, therefore it is always 0.
 *
 * @param[in] pinNo Pin number
 *
 * @return Analog value
 */
extern uint16_t analogRead(uint8_t pinNo);

/**
 * Get a random number in the range [0; max[.
 *
 * @param[in] max   Upper limit, exclusive.
 *
 * @return Random number
 */
extern long random(long max);

/**
 * Get a random number in the range [min; max[.
 *
 * @param[in] min   Lower limit, inclusive.
 * @param[in] max   Upper limit, exclusive.
 *
 * @return Random number
 */
extern long random(long min, long max);

/**
 * Initialize the random number generator.
 *
 * @param[in] seed  Seed value
 */
extern void randomSeed(unsigned long seed);

#endif /* ARDUINO_H */

/** @} */
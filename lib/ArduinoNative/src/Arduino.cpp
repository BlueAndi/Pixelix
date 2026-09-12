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
 * @file   Arduino.cpp
 * @brief  Arduino stuff for test
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Arduino.h"

#include <chrono>
#include <thread>

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

/** Max. number of supported pins. */
static const uint8_t MAX_PINS       = 64U;

/** Mode of every pin. There is no GPIO on the host, its just remembered. */
static uint8_t gPinModes[MAX_PINS]  = { 0U };

/** Level of every pin. There is no GPIO on the host, its just remembered. */
static uint8_t gPinLevels[MAX_PINS] = { 0U };

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

extern unsigned long millis()
{
    clock_t now = clock();

    return (now * 1000UL) / CLOCKS_PER_SEC;
}

extern unsigned long micros()
{
    clock_t now = clock();

    return (now * 1000000UL) / CLOCKS_PER_SEC;
}

extern void delay(unsigned long ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

extern void yield()
{
    std::this_thread::yield();
}

extern void pinMode(uint8_t pinNo, uint8_t mode)
{
    if (MAX_PINS > pinNo)
    {
        gPinModes[pinNo] = mode;
    }
}

extern void digitalWrite(uint8_t pinNo, uint8_t level)
{
    if (MAX_PINS > pinNo)
    {
        gPinLevels[pinNo] = level;
    }
}

extern int digitalRead(uint8_t pinNo)
{
    int level = LOW;

    if (MAX_PINS > pinNo)
    {
        level = gPinLevels[pinNo];
    }

    return level;
}

extern uint16_t analogRead(uint8_t pinNo)
{
    /* There is no ADC on the host. */
    (void)pinNo;

    return 0U;
}

extern long random(long max)
{
    return random(0, max);
}

extern long random(long min, long max)
{
    long result = min;

    if (min < max)
    {
        result = min + (rand() % (max - min));
    }

    return result;
}

extern void randomSeed(unsigned long seed)
{
    srand(static_cast<unsigned int>(seed));
}

extern uint32_t esp_log_timestamp(void)
{
    return millis();
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/
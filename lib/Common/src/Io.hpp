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
 * @brief  I/O abstraction
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup hal
 *
 * @{
 */

#ifndef IO_H
#define IO_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Standard i/o pin.
 *
 */
class IoPin
{
public:

    /**
     *  Unconnected/unavailable PIN
     */
    const static uint8_t NC = 0xFF;

    /**
     * Constructs an i/o pin instance.
     *
     * @param[in] pinNo     Arduino pin number
     * @param[in] pinMode   Arduino pin mode
     */
    IoPin(uint8_t pinNo, uint8_t pinMode) :
        m_pinNo(pinNo),
        m_pinMode(pinMode)
    {
    }

    /**
     * Destroys the i/o pin instance.
     */
    ~IoPin()
    {
    }

    /**
     * Initialize pin, which sets the mode.
     */
    void init() const
    {
        if (NC != m_pinNo)
        {
            pinMode(m_pinNo, m_pinMode);
        }
    }

    /**
     * Get pin number.
     *
     * @return Arduino pin number
     */
    uint8_t getPinNo() const
    {
        return m_pinNo;
    }

    /**
     * Get pin mode.
     *
     * @return Arduino pin mode
     */
    uint8_t getPinMode() const
    {
        return m_pinMode;
    }

private:

    const uint8_t   m_pinNo;    /**< Arduino pin number */
    const uint8_t   m_pinMode;  /**< Arduino pin mode */

    /* An instance shall not be copied. */
    IoPin(const IoPin& ioPin);
    IoPin& operator=(const IoPin& ioPin);
};

/**
 * Digital output pin.
 */
template < uint8_t pinNo >
class DOutPin : public IoPin
{
public:

    /**
     * Constructs an digital output pin instance.
     */
    DOutPin() :
        IoPin(pinNo, OUTPUT)
    {
    }

    /**
     * Destroys the digital output pin instance.
     */
    ~DOutPin()
    {
    }

    /**
     * Read from digital output pin
     *
     * @return Digital output pin value or LOW for unconnected pins.
     */
    int read() const
    {
        return (NC != pinNo) ? digitalRead(pinNo) : LOW;
    }

    /**
     * Write to digital output pin, do nothing for unconnected pins.
     *
     * @param[in] value Digital output pin value (LOW, HIGH).
     */
    void write(uint8_t value) const
    {
        if (NC != pinNo)
        {
            digitalWrite(pinNo, value);
        }
    }

private:

    /* An instance shall not be copied. */
    DOutPin(const DOutPin& dOutPin);
    DOutPin& operator=(const DOutPin& dOutPin);
};

/**
 * Digital input pin with not supported pin mode.
 */
template < uint8_t pinNo, uint8_t pinMode >
class DInPin : public IoPin
{
public:

private:

    /**
     * Constructs an digital output pin instance.
     */
    DInPin() :
        IoPin(pinNo, pinMode)
    {
    }

    /**
     * Destroys the digital output pin instance.
     */
    ~DInPin()
    {
    }

    /* An instance shall not be copied. */
    DInPin(const DInPin& dInPin);
    DInPin& operator=(const DInPin& dInPin);
};

/**
 * Digital input pin without pull-up or pull-down.
 */
template < uint8_t pinNo >
class DInPin<pinNo, INPUT> : public IoPin
{
public:

    /**
     * Constructs an digital output pin instance.
     */
    DInPin() :
        IoPin(pinNo, INPUT)
    {
    }

    /**
     * Destroys the digital output pin instance.
     */
    ~DInPin()
    {
    }

    /**
     * Read from digital input pin.
     *
     * @return Digital input pin value or LOW for unconnected ones.
     */
    int read() const
    {
        return (NC != pinNo) ? digitalRead(pinNo) : LOW;
    }

private:

    /* An instance shall not be copied. */
    DInPin(const DInPin& dInPin);
    DInPin& operator=(const DInPin& dInPin);
};

/**
 * Digital input pin with pull-up.
 */
template < uint8_t pinNo >
class DInPin<pinNo, INPUT_PULLUP> : public IoPin
{
public:

    /**
     * Constructs an digital output pin instance.
     */
    DInPin() :
        IoPin(pinNo, INPUT_PULLUP)
    {
    }

    /**
     * Destroys the digital output pin instance.
     */
    ~DInPin()
    {
    }

    /**
     * Read from digital input pin,
     *
     * @return Digital input pin value or LOW for unconnected pins.
     */
    int read() const
    {
        return (NC != pinNo) ? digitalRead(pinNo) : LOW;
    }

private:

    /* An instance shall not be copied. */
    DInPin(const DInPin& dInPin);
    DInPin& operator=(const DInPin& dInPin);
};

/**
 * Digital input pin with pull-down.
 */
template < uint8_t pinNo >
class DInPin<pinNo, INPUT_PULLDOWN> : public IoPin
{
public:

    /**
     * Constructs an digital output pin instance.
     */
    DInPin() :
        IoPin(pinNo, INPUT_PULLDOWN)
    {
    }

    /**
     * Destroys the digital output pin instance.
     */
    ~DInPin()
    {
    }

    /**
     * Read from digital input pin
     *
     * @return Digital input pin value or LOW for unconnected pins.
     */
    int read() const
    {
        return (NC != pinNo) ? digitalRead(pinNo) : LOW;
    }

private:

    /* An instance shall not be copied. */
    DInPin(const DInPin& dInPin);
    DInPin& operator=(const DInPin& dInPin);
};

/**
 * Analog pin.
 */
template < uint8_t pinNo >
class AnalogPin : public IoPin
{
public:

    /**
     * Constructs an analog input pin instance.
     */
    AnalogPin() :
        IoPin(pinNo, ANALOG)
    {
    }

    /**
     * Destroys the analog input pin instance.
     */
    ~AnalogPin()
    {
    }

    /**
     * Read from analog input pin.
     *
     * @return Value in ADC digits or LOW for unconnected pins.
     */
    uint16_t read() const
    {
        return (NC != pinNo) ? analogRead(pinNo) : LOW;
    }

private:

    /* An instance shall not be copied. */
    AnalogPin(const AnalogPin& analogPin);
    AnalogPin& operator=(const AnalogPin& analogPin);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* IO_H */

/** @} */
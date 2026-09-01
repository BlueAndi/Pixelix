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
 * @file   Io.hpp
 * @brief  I/O abstraction
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup HAL
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
    virtual ~IoPin()
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

    const uint8_t m_pinNo;   /**< Arduino pin number */
    const uint8_t m_pinMode; /**< Arduino pin mode */

    /* An instance shall not be copied. */
    IoPin(const IoPin& ioPin);
    IoPin& operator=(const IoPin& ioPin);
};

/**
 * Digital output pin.
 */
class DOutPin : public IoPin
{
public:

    /**
     * Destroys the digital output pin interface.
     */
    virtual ~DOutPin()
    {
    }

    /**
     * Read from digital output pin.
     *
     * @return Digital output pin value.
     */
    virtual int read() const                = 0;

    /**
     * Write to digital output pin.
     *
     * @param[in] value Digital output pin value (LOW, HIGH).
     */
    virtual void write(uint8_t value) const = 0;

protected:

    /**
     * Constructs the digital output pin interface.
     */
    DOutPin(uint8_t pinNo) :
        IoPin(pinNo, OUTPUT)
    {
    }
};

/**
 * Digital output pin with a compile-time pin number.
 */
template < uint8_t pinNo >
class DOutPinT : public DOutPin
{
public:

    DOutPinT() :
        DOutPin(pinNo)
    {
    }

    /**
     * Destroys the digital output pin instance.
     */
    ~DOutPinT()
    {
    }

    /**
     * Read from digital output pin
     *
     * @return Digital output pin value or LOW for unconnected pins.
     */
    int read() const override
    {
        return (NC != pinNo) ? digitalRead(pinNo) : LOW;
    }

    /**
     * Write to digital output pin, do nothing for unconnected pins.
     *
     * @param[in] value Digital output pin value (LOW, HIGH).
     */
    void write(uint8_t value) const override
    {
        if (NC != pinNo)
        {
            digitalWrite(pinNo, value);
        }
    }

private:

    /* An instance shall not be copied. */
    DOutPinT(const DOutPinT& dOutPin);
    DOutPinT& operator=(const DOutPinT& dOutPin);
};

/**
 * Digital input pin with not supported pin mode.
 */
class DInPin : public IoPin
{
public:

    virtual ~DInPin()
    {
    }

    /**
     * Read from digital input pin.
     *
     * @return Digital input pin value.
     */
    virtual int read() const = 0;

protected:

    /**
     * Constructs the digital input pin interface.
     */
    DInPin(uint8_t pinNo, uint8_t pinMode) :
        IoPin(pinNo, pinMode)
    {
    }
};

template < uint8_t pinNo, uint8_t pinMode >
class DInPinT;

/**
 * Digital input pin without pull-up or pull-down.
 */
template < uint8_t pinNo >
class DInPinT<pinNo, INPUT> : public DInPin
{
public:

    /**
     * Constructs an digital output pin instance.
     */
    DInPinT() :
        DInPin(pinNo, INPUT)
    {
    }

    /**
     * Destroys the digital output pin instance.
     */
    ~DInPinT()
    {
    }

    /**
     * Read from digital input pin.
     *
     * @return Digital input pin value or LOW for unconnected ones.
     */
    int read() const override
    {
        return (NC != pinNo) ? digitalRead(pinNo) : LOW;
    }

private:

    /* An instance shall not be copied. */
    DInPinT(const DInPinT& dInPin);
    DInPinT& operator=(const DInPinT& dInPin);
};

/**
 * Digital input pin with pull-up.
 */
template < uint8_t pinNo >
class DInPinT<pinNo, INPUT_PULLUP> : public DInPin
{
public:

    /**
     * Constructs an digital output pin instance.
     */
    DInPinT() :
        DInPin(pinNo, INPUT_PULLUP)
    {
    }

    /**
     * Destroys the digital output pin instance.
     */
    ~DInPinT()
    {
    }

    /**
     * Read from digital input pin,
     *
     * @return Digital input pin value or LOW for unconnected pins.
     */
    int read() const override
    {
        return (NC != pinNo) ? digitalRead(pinNo) : LOW;
    }

private:

    /* An instance shall not be copied. */
    DInPinT(const DInPinT& dInPin);
    DInPinT& operator=(const DInPinT& dInPin);
};

/**
 * Digital input pin with pull-down.
 */
template < uint8_t pinNo >
class DInPinT<pinNo, INPUT_PULLDOWN> : public DInPin
{
public:

    /**
     * Constructs an digital output pin instance.
     */
    DInPinT() :
        DInPin(pinNo, INPUT_PULLDOWN)
    {
    }

    /**
     * Destroys the digital output pin instance.
     */
    ~DInPinT()
    {
    }

    /**
     * Read from digital input pin
     *
     * @return Digital input pin value or LOW for unconnected pins.
     */
    int read() const override
    {
        return (NC != pinNo) ? digitalRead(pinNo) : LOW;
    }

private:

    /* An instance shall not be copied. */
    DInPinT(const DInPinT& dInPin);
    DInPinT& operator=(const DInPinT& dInPin);
};

/**
 * Analog pin.
 */
class AnalogPin : public IoPin
{
public:

    virtual ~AnalogPin()
    {
    }

    /**
     * Read from analog input pin.
     *
     * @return Value in ADC digits.
     */
    virtual uint16_t read() const = 0;

protected:

    /**
     * Constructs the analog pin interface.
     */
    AnalogPin(uint8_t pinNo) :
        IoPin(pinNo, ANALOG)
    {
    }
};

/**
 * Analog pin with a compile-time pin number.
 */
template < uint8_t pinNo >
class AnalogPinT : public AnalogPin
{
public:

    AnalogPinT() :
        AnalogPin(pinNo)
    {
    }

    /**
     * Destroys the analog input pin instance.
     */
    ~AnalogPinT()
    {
    }

    /**
     * Read from analog input pin.
     *
     * @return Value in ADC digits or LOW for unconnected pins.
     */
    uint16_t read() const override
    {
        return (NC != pinNo) ? analogRead(pinNo) : LOW;
    }

private:

    /* An instance shall not be copied. */
    AnalogPinT(const AnalogPinT& analogPin);
    AnalogPinT& operator=(const AnalogPinT& analogPin);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* IO_H */

/** @} */
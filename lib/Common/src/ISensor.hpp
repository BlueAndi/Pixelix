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
 * @brief  Generic sensor interface
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup hal
 *
 * @{
 */

#ifndef ISENSOR_HPP
#define ISENSOR_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <ISensorChannel.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Abstract sensor interface.
 */
class ISensor
{
public:

    /**
     * Configures the sensor, so it is able to provide sensor data.
     */
    virtual void begin() = 0;

    /**
     * Process the sensor driver. Mainly used to read the sensor value and
     * provide its data cached to the sensor channels.
     */
    virtual void process() = 0;

    /**
     * Get sensor name.
     * 
     * @return Sensor name
     */
    virtual const char* getName() const = 0;

    /**
     * Is sensor available?
     * If a sensor is physically not available or the initialization failed (see begin()),
     * this can be checked with this method.
     *
     * @return If sensor is available, it will return true otherwise false.
     */
    virtual bool isAvailable() const = 0;

    /**
     * Get number of data channels.
     * 
     * @return Number of data channels.
     */
    virtual uint8_t getNumChannels() const = 0;

    /**
     * Get data channel by index.
     * If sensor is not available or channel index is out of bounds, it will 
     * return nullptr.
     * 
     * @return Data channel
     */
    virtual ISensorChannel* getChannel(uint8_t index) = 0;

protected:

    /**
     * Constructs the sensor interface.
     */
    ISensor()
    {
    }

private:

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* ISENSOR_HPP */

/** @} */
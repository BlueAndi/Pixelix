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
 * @file   MqttTypes.h
 * @brief  MQTT types
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup MQTT_SERVICE
 *
 * @{
 */

#ifndef MQTT_TYPES_H
#define MQTT_TYPES_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <functional>
#include <WString.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** MQTT types */
namespace MqttTypes
{
    /**
     * Topic callback prototype.
     */
    typedef std::function<void(const String& topic, const uint8_t* payload, size_t size)> TopicCallback;

    /**
     * MQTT connection states.
     */
    typedef enum
    {
        STATE_IDLE = 0,     /**< Connection is idle */
        STATE_DISCONNECTED, /**< No connection to a MQTT broker */
        STATE_CONNECTED     /**< Connected with a MQTT broker */
    } State;

} /* MQTT types */

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* MQTT_TYPES_H */

/** @} */
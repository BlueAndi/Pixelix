/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @file   MqttTopic.cpp
 * @brief  Makapix MQTT topics
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MqttTopic.h"

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

void MqttTopic::getTopicPrefix(const String& playerKey, String& topic)
{
    topic = "makapix/player/" + playerKey;
}

void MqttTopic::getCommandTopic(const String& playerKey, String& topic)
{
    getTopicPrefix(playerKey, topic);
    topic += "/command";
}

void MqttTopic::getStatusTopic(const String& playerKey, String& topic)
{
    getTopicPrefix(playerKey, topic);
    topic += "/status";
}

void MqttTopic::getRequestTopic(const String& playerKey, String& topic, int32_t requestId)
{
    getTopicPrefix(playerKey, topic);
    topic += "/request/" + String(requestId);
}

void MqttTopic::getResponseTopic(const String& playerKey, String& topic, int32_t requestId)
{
    getTopicPrefix(playerKey, topic);
    topic += "/response/" + String(requestId);
}

void MqttTopic::getViewUpdateTopic(const String& playerKey, String& topic)
{
    getTopicPrefix(playerKey, topic);
    topic += "/view";
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

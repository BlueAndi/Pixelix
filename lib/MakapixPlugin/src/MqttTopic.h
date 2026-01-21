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
 * @file   MqttTopic.h
 * @brief  Makapix MQTT topics
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef MQTT_TOPIC_H
#define MQTT_TOPIC_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <WString.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Makapix MQTT topics.
 */
namespace MqttTopic
{

    /**
     * Get topic prefix.
     *
     * @param[in]  playerKey    The player key.
     * @param[out] topic        The topic prefix.
     */
    void getTopicPrefix(const String& playerKey, String& topic);

    /**
     * Get command topic.
     *
     * @param[in]  playerKey    The player key.
     * @param[out] topic        The command topic.
     */
    void getCommandTopic(const String& playerKey, String& topic);

    /**
     * Get status topic.
     *
     * @param[in]  playerKey    The player key.
     * @param[out] topic        The status topic.
     */
    void getStatusTopic(const String& playerKey, String& topic);

    /**
     * Get request topic.
     *
     * @param[in]  playerKey    The player key.
     * @param[out] topic        The request topic.
     * @param[in]  requestId    The request id.
     */
    void getRequestTopic(const String& playerKey, String& topic, int32_t requestId);

    /**
     * Get response topic.
     *
     * @param[in]  playerKey    The player key.
     * @param[out] topic        The response topic.
     * @param[in]  requestId    The request id.
     */
    void getResponseTopic(const String& playerKey, String& topic, int32_t requestId);

} /* namespace MqttTopic */

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* MQTT_TOPIC_H */

/** @} */

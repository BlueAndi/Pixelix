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
 * @brief  Home Assistant MQTT extension
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef HOME_ASSISTANT_MQTT_H
#define HOME_ASSISTANT_MQTT_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <KeyValueString.h>
#include <ArduinoJson.h>
#include <vector>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Home Assistant MQTT extension, which supports the MQTT discovery.
 * https://www.home-assistant.io/integrations/mqtt/
 */
class HomeAssistantMqtt
{
public:

    /**
     * Construct the Home Assistant extension instance.
     */
    HomeAssistantMqtt() :
        m_haDiscoveryPrefixSetting(KEY_HA_DISCOVERY_PREFIX, NAME_HA_DISCOVERY_PREFIX, DEFAULT_HA_DISCOVERY_PREFIX, MIN_VALUE_HA_DISCOVERY_PREFIX, MAX_VALUE_HA_DISCOVERY_PREFIX),
        m_haDiscoveryPrefix()
    {
    }

    /**
     * Destroy the Home Assistant extension instance.
     */
    ~HomeAssistantMqtt()
    {
        clearMqttDiscoveryInfoList();
    }

    /**
     * Start the Home Assistant extension.
     */
    void start();

    /**
     * Stop the Home Assistant extension.
     */
    void stop();

    /**
     * Register Home Assistant MQTT discovery.
     * It will not publish, just prepare the MQTT discovery information
     * and hold it internally.
     * 
     * @param[in]   nodeId      ID of the node providing the topic.
     * @param[in]   objectId    ID of the object to divide between topics.
     * @param[in]   stateTopic  The MQTT status topic.
     * @param[in]   cmdTopic    The MQTT command topic.
     * @param[in]   extra       Extra parameters used by this extension.
     */
    void registerMqttDiscovery(const String& nodeId, const String& objectId, const String& stateTopic, const String& cmdTopic, JsonObjectConst& extra);

    /**
     * Unregister Home Assistant MQTT discovery.
     * 
     * @param[in]   nodeId      ID of the node providing the topic.
     * @param[in]   objectId    ID of the object to divide between topics.
     * @param[in]   stateTopic  The MQTT status topic.
     * @param[in]   cmdTopic    The MQTT command topic.
     */
    void unregisterMqttDiscovery(const String& nodeId, const String& objectId, const String& stateTopic, const String& cmdTopic);

    /**
     * Publish the MQTT auto discovery information.
     */
    void publishAutoDiscoveryInfo();

private:

    /** Home Assistant discovery prefix key */
    static const char*  KEY_HA_DISCOVERY_PREFIX;

    /**Home Assistant discovery prefix name */
    static const char*  NAME_HA_DISCOVERY_PREFIX;

    /** Home Assistant discovery prefix default value */
    static const char*  DEFAULT_HA_DISCOVERY_PREFIX;

    /** Home Assistant discovery prefix min. length */
    static const size_t MIN_VALUE_HA_DISCOVERY_PREFIX   = 0U;

    /** Home Assistant discovery prefix max. length */
    static const size_t MAX_VALUE_HA_DISCOVERY_PREFIX   = 64U;

    /** Information necessary for Home Assistant MQTT discovery. */
    struct MqttDiscoveryInfo
    {
        String  component;          /**< Home Assistant component */
        String  nodeId;             /**< Home Assistant node id */
        String  objectId;           /**< Home Assistant object id */
        String  stateTopic;         /**< Status topic */
        String  valueTemplate;      /**< Value template to extract the text state value */
        String  commandTopic;       /**< Command topic */
        String  commandTemplate;    /**< Command template to generate payload to send to command topic */

        /** Construct Home Assistant MQTT discovery information. */
        MqttDiscoveryInfo() :
            component(),
            nodeId(),
            objectId(),
            stateTopic(),
            valueTemplate(),
            commandTopic(),
            commandTemplate()
        {
        }
    };

    /** List of Home Assistant MQTT discovery information. */
    typedef std::vector<MqttDiscoveryInfo*> ListOfMqttDiscoveryInfo;

    KeyValueString          m_haDiscoveryPrefixSetting; /**< Setting for the Home Assistant MQTT discovery prefix. */
    String                  m_haDiscoveryPrefix;        /**< Home Assistant MQTT discovery prefix. */
    ListOfMqttDiscoveryInfo m_mqttDiscoveryInfoList;    /**< List of Home Assistant MQTT discovery informations. */

    HomeAssistantMqtt(const HomeAssistantMqtt& ext);
    HomeAssistantMqtt& operator=(const HomeAssistantMqtt& ext);

    /**
     * Clear MQTT discovery info list.
     */
    void clearMqttDiscoveryInfoList();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* HOME_ASSISTANT_MQTT_H */

/** @} */
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
#include <KeyValueBool.h>
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
        m_haDiscoveryEnabledSetting(KEY_HA_DISCOVERY_ENABLE, NAME_HA_DISCOVERY_ENABLE, DEFAULT_HA_DISCOVERY_ENABLE),
        m_haDiscoveryPrefix(),
        m_haDiscoveryEnabled(false),
        m_isConnected(false)
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
     * Process the Home Assistant extension.
     * 
     * @param[in] isConnected   Is a MQTT broker connection established?
     */
    void process(bool isConnected);

    /**
     * Register Home Assistant MQTT discovery.
     * It will not publish, just prepare the MQTT discovery information
     * and hold it internally.
     * 
     * @param[in]   deviceId            Device id.
     * @param[in]   entityId            Entity id.
     * @param[in]   stateTopic          The MQTT status topic.
     * @param[in]   cmdTopic            The MQTT command topic.
     * @param[in]   availabilityTopic   The MQTT availability topic.
     * @param[in]   extra               Extra parameters used by this extension.
     */
    void registerMqttDiscovery(const String& deviceId, const String& entityId, const String& stateTopic, const String& cmdTopic, const String& availabilityTopic, JsonObjectConst& extra);

    /**
     * Unregister Home Assistant MQTT discovery.
     * 
     * @param[in]   deviceId    Device id.
     * @param[in]   entityId    Entity id.
     * @param[in]   stateTopic  The MQTT status topic.
     * @param[in]   cmdTopic    The MQTT command topic.
     */
    void unregisterMqttDiscovery(const String& deviceId, const String& entityId, const String& stateTopic, const String& cmdTopic);

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

    /** Home Assistant discovery enable flag key */
    static const char*  KEY_HA_DISCOVERY_ENABLE;

    /** Home Assistant discovery enable flag name */
    static const char*  NAME_HA_DISCOVERY_ENABLE;

    /** Home Assistant discovery enable flag default value */
    static const bool   DEFAULT_HA_DISCOVERY_ENABLE;

    /** Information necessary for Home Assistant MQTT discovery. */
    struct MqttDiscoveryInfo
    {
        String              component;          /**< Home Assistant component */
        String              nodeId;             /**< Home Assistant node id */
        String              objectId;           /**< Home Assistant object id */
        DynamicJsonDocument discoveryDetails;   /**< Additional discovery information. */
        bool                isReqToPublish;     /**< Is requested to publish this discovery info? */

        /** Construct Home Assistant MQTT discovery information. */
        MqttDiscoveryInfo() :
            component(),
            nodeId(),
            objectId(),
            discoveryDetails(368U),
            isReqToPublish(true)
        {
        }
    };

    /** List of Home Assistant MQTT discovery information. */
    typedef std::vector<MqttDiscoveryInfo*> ListOfMqttDiscoveryInfo;

    KeyValueString          m_haDiscoveryPrefixSetting;     /**< Setting for the Home Assistant MQTT discovery prefix. */
    KeyValueBool            m_haDiscoveryEnabledSetting;    /**< Setting for the Home Assistant MQTT discovery enable flag. */
    String                  m_haDiscoveryPrefix;            /**< Home Assistant MQTT discovery prefix. */
    bool                    m_haDiscoveryEnabled;           /**< Is the Home Assistant MQTT discovery enabled or not. */
    ListOfMqttDiscoveryInfo m_mqttDiscoveryInfoList;        /**< List of Home Assistant MQTT discovery informations. */
    bool                    m_isConnected;                  /**< Is MQTT broker connection established? */

    HomeAssistantMqtt(const HomeAssistantMqtt& ext);
    HomeAssistantMqtt& operator=(const HomeAssistantMqtt& ext);

    /**
     * Get the object id from the entity id.
     * 
     * @param[in] entityId  The entity id.
     * 
     * @return Object id
     */
    String getObjectId(const String& entityId);

    /**
     * Clear MQTT discovery info list.
     */
    void clearMqttDiscoveryInfoList();

    /**
     * Get the discovery configuration topic.
     * 
     * @param[out]  haConfigTopic   Discovery configuration topic
     * @param[in]   component       Home Assistant component
     * @param[in]   nodeId          Home Assistant node id
     * @param[in]   objectId        Home Assistant object id
     */
    void getConfigTopic(String& haConfigTopic, const String& component, const String& nodeId, const String& objectId);

    /**
     * Publish the MQTT auto discovery information.
     */
    void publishAutoDiscoveryInfo(MqttDiscoveryInfo& mqttDiscoveryInfo);

    /**
     * Request to publish all automatic discovery info's.
     */
    void requestToPublishAllAutoDiscoveryInfos();

    /**
     * Publish MQTT auto discovery informations, which are requested.
     * 
     * Note: Need to be called continously and will only publish one info per
     *       call cycle.
     */
    void publishAutoDiscoveryInfosOnDemand();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* HOME_ASSISTANT_MQTT_H */

/** @} */
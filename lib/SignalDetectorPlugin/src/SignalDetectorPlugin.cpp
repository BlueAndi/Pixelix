/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Audio signal detector plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "SignalDetectorPlugin.h"
#include "AudioService.h"
#include "HttpStatus.h"

#include <Logging.h>

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

/* Initialize plugin topic. */
const char*     SignalDetectorPlugin::TOPIC_CONFIG      = "/signalDetector";

/* Initialize the default text which will be shown if signal is detected. */
const char*     SignalDetectorPlugin::DEFAULT_TEXT      = "{hc}Signal!";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool SignalDetectorPlugin::isEnabled() const
{
    bool isEnabled = false;

    /* The plugin shall only be scheduled if signal was detected and this info
     * shall be shown to the user.
     */
    if ((true == m_isEnabled) &&
        (true == m_isDetected))
    {
        isEnabled = true;
    }

    return isEnabled;
}

void SignalDetectorPlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC_CONFIG);
}

bool SignalDetectorPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_CONFIG))
    {
        getConfiguration(value);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool SignalDetectorPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_CONFIG))
    {
        const size_t        JSON_DOC_SIZE           = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg                 = jsonDoc.to<JsonObject>();
        JsonArrayConst      jsonTones               = value["tones"];
        JsonVariantConst    jsonText                = value["text"];
        JsonVariantConst    jsonPushUrl             = value["pushUrl"];

        /* The received configuration may not contain all single key/value pair.
         * Therefore read first the complete internal configuration and
         * overwrite them with the received ones.
         */
        getConfiguration(jsonCfg);

        /* Note:
         * Check only for the key/value pair availability.
         * The type check will follow in the setConfiguration().
         */

        if (false == jsonText.isNull())
        {
            jsonCfg["text"] = jsonText;
            isSuccessful = true;
        }

        if (false == jsonPushUrl.isNull())
        {
            jsonCfg["pushUrl"] = jsonPushUrl;
            isSuccessful = true;
        }

        if (false == jsonTones.isNull())
        {
            uint8_t toneIdx = 0U;

            for(JsonVariantConst tone : jsonTones)
            {
                if (AudioService::MAX_TONE_DETECTORS <= toneIdx)
                {
                    break;
                }
                else
                {
                    JsonVariantConst jsonTargetFreq     = tone["frequency"];
                    JsonVariantConst jsonMinDuration    = tone["minDuration"];
                    JsonVariantConst jsonThreshold      = tone["threshold"];

                    if (false == jsonTargetFreq.isNull())
                    {
                        jsonCfg["tones"][toneIdx]["frequency"] = jsonTargetFreq.as<float>();
                        isSuccessful = true;
                    }

                    if (false == jsonMinDuration.isNull())
                    {
                        jsonCfg["tones"][toneIdx]["minDuration"] = jsonMinDuration.as<uint32_t>();
                        isSuccessful = true;
                    }

                    if (false == jsonThreshold.isNull())
                    {
                        jsonCfg["tones"][toneIdx]["threshold"] = jsonThreshold.as<float>();
                        isSuccessful = true;
                    }
                }

                ++toneIdx;
            }
        }

        if (true == isSuccessful)
        {
            JsonObjectConst jsonCfgConst = jsonCfg;

            isSuccessful = setConfiguration(jsonCfgConst);

            if (true == isSuccessful)
            {
                requestStoreToPersistentMemory();
            }
        }
    }

    return isSuccessful;
}

bool SignalDetectorPlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    bool                        hasTopicChanged = m_hasTopicChanged;

    /* Only a single topic, therefore its not necessary to check. */
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void SignalDetectorPlugin::setSlot(const ISlotPlugin* slotInterf)
{
    m_slotInterf = slotInterf;
}

void SignalDetectorPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_view.init(width, height);
    m_view.setFormatText(DEFAULT_TEXT);

    /* Clear */
    m_isDetected = false;

    PluginWithConfig::start(width, height);

    initHttpClient();
}

void SignalDetectorPlugin::stop()
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    PluginWithConfig::stop();
}

void SignalDetectorPlugin::active(YAGfx& gfx)
{
    /* Nothing to do. */
}

void SignalDetectorPlugin::inactive()
{
    /* Active phase ends with slot duration timeout or user triggered slot change.
     * Attention: If plugin is the only one, inactive() won't be called. For this
     *            case a own timer is used.
     */
    m_isDetected = false;
    m_timer.stop();
}

void SignalDetectorPlugin::process(bool isConnected)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    /* Call isSignalDetected() every time although it was already detected in the
     * previous call. This clears the detection flag in the audio service.
     */
    bool                        isDetected = isSignalDetected();

    if (true == isDetected)
    {
        LOG_INFO("Signal detected.");
    }

    /* Ensure that once the signal is detected, it is shown to the user. */
    if (false == m_isDetected)
    {
        m_isDetected = isDetected;

        /* Observe active phase. */
        if (nullptr != m_slotInterf)
        {
            /* Start with 10% greater slot duration. */
            m_timer.start(m_slotInterf->getDuration() * 110U / 100U);
        }

        /* Send notification */
        (void)startHttpRequest();
    }
    else
    {
        /* Exception case if plugin is the only one and inactive() won't
         * be called.
         */
        if ((true == m_timer.isTimerRunning()) &&
            (true == m_timer.isTimeout()))
        {
            m_timer.stop();
            m_isDetected = false;
        }
    }

    PluginWithConfig::process(isConnected);
}

void SignalDetectorPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_view.update(gfx);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void SignalDetectorPlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    uint8_t                     idx             = 0U;
    JsonArray                   jsonTones       = jsonCfg.createNestedArray("tones");

    while(AudioService::MAX_TONE_DETECTORS > idx)
    {
        AudioToneDetector*  audioToneDetector   = AudioService::getInstance().getAudioToneDetector(idx);

        if (nullptr != audioToneDetector)
        {
            JsonObject jsonTone = jsonTones.createNestedObject();

            jsonTone["frequency"]   = audioToneDetector->getTargetFreq();
            jsonTone["minDuration"] = audioToneDetector->getMinDuration();
            jsonTone["threshold"]   = audioToneDetector->getThreshold();
        }

        ++idx;
    }

    jsonCfg["text"]     = m_view.getFormatText();
    jsonCfg["pushUrl"]  = m_pushUrl;
}

bool SignalDetectorPlugin::setConfiguration(const JsonObjectConst& jsonCfg)
{
    bool                status      = false;
    JsonArrayConst      jsonTones   = jsonCfg["tones"];
    JsonVariantConst    jsonText    = jsonCfg["text"];
    JsonVariantConst    jsonPushUrl = jsonCfg["pushUrl"];

    if (true == jsonTones.isNull())
    {
        LOG_WARNING("Tone(s) not found or invalid type.");
    }
    else if (false == jsonText.is<String>())
    {
        LOG_WARNING("Text not found or invalid type.");
    }
    else if (false == jsonPushUrl.is<String>())
    {
        LOG_WARNING("Push URL not found or invalid type.");
    }
    else
    {
        MutexGuard<MutexRecursive>  guard(m_mutex);
        uint8_t                     idx = 0U;

        status = true;

        for(JsonVariantConst tone : jsonTones)
        {
            AudioToneDetector* audioToneDetector = AudioService::getInstance().getAudioToneDetector(idx);

            if (nullptr == audioToneDetector)
            {
                LOG_WARNING("Too many tone detector configurations.");
                break;
            }
            else
            {
                JsonVariantConst jsonTargetFreq     = tone["frequency"];
                JsonVariantConst jsonMinDuration    = tone["minDuration"];
                JsonVariantConst jsonThreshold      = tone["threshold"];

                if (false == jsonTargetFreq.is<float>())
                {
                    LOG_WARNING("Target frequency not found or invalid type.");
                    status = false;
                }
                else if (false == jsonMinDuration.is<uint32_t>())
                {
                    LOG_WARNING("Min. duration not found or invalid type.");
                    status = false;
                }
                else if (false == jsonThreshold.is<float>())
                {
                    LOG_WARNING("Threshold not found or invalid type.");
                    status = false;
                }
                else
                {
                    audioToneDetector->setTargetFreq(jsonTargetFreq.as<float>());
                    audioToneDetector->setMinDuration(jsonMinDuration.as<uint32_t>());
                    audioToneDetector->setThreshold(jsonThreshold.as<float>());

                    ++idx;
                }

                if (false == status)
                {
                    break;
                }
            }
        }

        m_view.setFormatText(jsonText.as<String>());
        m_pushUrl = jsonPushUrl.as<String>();

        m_hasTopicChanged = true;
    }

    return status;
}

bool SignalDetectorPlugin::startHttpRequest()
{
    bool status = false;

    if (false == m_pushUrl.isEmpty())
    {
        String      url         = m_pushUrl;
        const char* GET_CMD     = "get ";
        const char* POST_CMD    = "post ";
        bool        isGet       = true;

        /* URL prefix might indicate the kind of request. */
        url.toLowerCase();
        if (true == url.startsWith(GET_CMD))
        {
            url = url.substring(strlen(GET_CMD));
            isGet = true;
        }
        else if (true == url.startsWith(POST_CMD))
        {
            url = url.substring(strlen(POST_CMD));
            isGet = false;
        }
        else
        {
            ;
        }

        if (true == m_client.begin(url))
        {
            if (false == isGet)
            {
                if (false == m_client.POST())
                {
                    LOG_WARNING("POST %s failed.", url.c_str());
                }
                else
                {
                    status = true;
                }
            }
            else
            {
                if (false == m_client.GET())
                {
                    LOG_WARNING("GET %s failed.", url.c_str());
                }
                else
                {
                    status = true;
                }
            }
        }
    }

    return status;
}

void SignalDetectorPlugin::initHttpClient()
{
    /* Note: All registered callbacks are running in a different task context! */
    m_client.regOnResponse([](const HttpResponse& rsp) {
        uint16_t statusCode = rsp.getStatusCode();

        if (HttpStatus::STATUS_CODE_OK == statusCode)
        {
            LOG_INFO("Signal detection reported.");
        }

    });

    m_client.regOnError([]() {
        LOG_WARNING("Connection error happened.");
   });
}

bool SignalDetectorPlugin::isSignalDetected()
{
    uint8_t idx                         = 0U;
    bool    isDetected                  = false;
    uint8_t countDetectedTones          = 0U;
    uint8_t countEnabledToneDetectors   = 0U;

    /* Every enabled tone detector must be considered.
     * A target frequency of 0 Hz means, the tone detector is disabled.
     */
    while(AudioService::MAX_TONE_DETECTORS > idx)
    {
        AudioToneDetector* audioToneDetector = AudioService::getInstance().getAudioToneDetector(idx);

        if (nullptr != audioToneDetector)
        {
            float freq = audioToneDetector->getTargetFreq();

            if ((AudioToneDetector::EPSILON < freq) || (-AudioToneDetector::EPSILON > freq))
            {
                ++countEnabledToneDetectors;

                if (true == audioToneDetector->isTargetFreqDetected())
                {
                    LOG_INFO("Freq %u detected with magnitude %0.0f.", idx, audioToneDetector->getLastMagnitude());

                    ++countDetectedTones;
                }
            }
        }

        ++idx;
    }

    if ((0U < countEnabledToneDetectors) &&
        (countDetectedTones == countEnabledToneDetectors))
    {
        isDetected = true;
    }

    return isDetected;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

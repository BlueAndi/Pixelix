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
#include <FileSystem.h>
#include <JsonFile.h>
#include <Util.h>

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

/* Initialize plugin topics. */
const char*     SignalDetectorPlugin::TOPIC_CFG         = "/cfg";
const char*     SignalDetectorPlugin::TOPIC_TEXT        = "/text";
const char*     SignalDetectorPlugin::TOPIC_PUSH_URL    = "/pushUrl";
const char*     SignalDetectorPlugin::DEFAULT_TEXT      = "\\calignSignal!";

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
    uint8_t idx = 0U;

    while(AudioService::MAX_TONE_DETECTORS > idx)
    {
        (void)topics.add(String(TOPIC_CFG) + "/" + idx);
        ++idx;
    }

    (void)topics.add(TOPIC_TEXT);
    (void)topics.add(TOPIC_PUSH_URL);
}

bool SignalDetectorPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (0U != topic.startsWith(String(TOPIC_CFG) + "/"))
    {
        uint32_t    indexBeginToneId    = topic.lastIndexOf("/") + 1U;
        String      toneIdStr           = topic.substring(indexBeginToneId);
        uint8_t     toneId              = AudioService::MAX_TONE_DETECTORS;
        bool        status              = Util::strToUInt8(toneIdStr, toneId);

        if ((true == status) &&
            (AudioService::MAX_TONE_DETECTORS > toneId))
        {
            value["frequency"]      = getTargetFreq(toneId);
            value["minDuration"]    = getMinDuration(toneId);
            value["threshold"]      = getThreshold(toneId);

            isSuccessful = true;
        }
    }
    else if (0U != topic.equals(TOPIC_TEXT))
    {
        value["text"] = getText();
        isSuccessful = true;
    }
    else if (0U != topic.equals(TOPIC_PUSH_URL))
    {
        value["pushUrl"] = getPushUrl();
        isSuccessful = true;
    }
    else
    {
        ;
    }

    return isSuccessful;
}

bool SignalDetectorPlugin::setTopic(const String& topic, const JsonObject& value)
{
    bool isSuccessful = false;

    if (0U != topic.startsWith(String(TOPIC_CFG) + "/"))
    {
        uint32_t    indexBeginToneId    = topic.lastIndexOf("/") + 1U;
        String      toneIdStr           = topic.substring(indexBeginToneId);
        uint8_t     toneId              = AudioService::MAX_TONE_DETECTORS;
        bool        status              = Util::strToUInt8(toneIdStr, toneId);

        if ((true == status) &&
            (AudioService::MAX_TONE_DETECTORS > toneId))
        {
            JsonVariantConst jsonTargetFreq  = value["frequency"];
            JsonVariantConst jsonMinDuration = value["minDuration"];
            JsonVariantConst jsonThreshold   = value["threshold"];

            if (false == jsonTargetFreq.isNull())
            {
                float freq = jsonTargetFreq.as<float>();

                setTargetFreq(toneId, freq);
            }

            if (false == jsonMinDuration.isNull())
            {
                uint32_t minDuration = jsonMinDuration.as<uint32_t>();

                setMinDuration(toneId, minDuration);
            }

            if (false == jsonThreshold.isNull())
            {
                float threshold = jsonThreshold.as<float>();

                setThreshold(toneId, threshold);
            }

            isSuccessful = true;   
        }
    }
    else if (0U != topic.equals(TOPIC_TEXT))
    {
        String              text;
        JsonVariantConst    jsonShow    = value["show"];

        if (false == jsonShow.isNull())
        {
            text = jsonShow.as<String>();
            isSuccessful = true;
        }

        if (true == isSuccessful)
        {
            setText(text);
        }
    }
    else if (0U != topic.equals(TOPIC_PUSH_URL))
    {
        String              url;
        JsonVariantConst    jsonSet = value["set"];

        if (false == jsonSet.isNull())
        {
            url = jsonSet.as<String>();
            isSuccessful = true;
        }

        if (true == isSuccessful)
        {
            setPushUrl(url);
        }
    }
    else
    {
        ;
    }

    return isSuccessful;
}

void SignalDetectorPlugin::setSlot(const ISlotPlugin* slotInterf)
{
    m_slotInterf = slotInterf;
}

void SignalDetectorPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    PLUGIN_NOT_USED(width);

    /* Choose font. */
    m_textWidget.setFont(Fonts::getFontByType(m_fontType));

    /* The text widget is left aligned on x-axis and aligned to the center
     * of y-axis.
     */
    if (height > m_textWidget.getFont().getHeight())
    {
        uint16_t diffY = height - m_textWidget.getFont().getHeight();
        uint16_t offsY = diffY / 2U;

        m_textWidget.move(0, offsY);
    }

    /* Clear */
    m_isDetected = false;

    /* Try to load configuration. If there is no configuration available, a default configuration
     * will be created.
     */
    if (false == loadConfiguration())
    {
        if (false == saveConfiguration())
        {
            LOG_WARNING("Failed to create initial configuration file %s.", getFullPathToConfiguration().c_str());
        }
    }

    initHttpClient();
}

void SignalDetectorPlugin::stop()
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    String                      configurationFilename   = getFullPathToConfiguration();

    if (false != FILESYSTEM.remove(configurationFilename))
    {
        LOG_INFO("File %s removed", configurationFilename.c_str());
    }
}

void SignalDetectorPlugin::active(YAGfx& gfx)
{
    /* Request display update. In principle only necessary once, except
     * that the text changes during active phase.
     */
    m_isUpdateReq = true;
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

    PLUGIN_NOT_USED(isConnected);

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
}

void SignalDetectorPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    /* Update display only if requested. */
    if (true == m_isUpdateReq)
    {
        gfx.fillScreen(ColorDef::BLACK);
        m_textWidget.update(gfx);

        m_isUpdateReq = false;
    }
}

float SignalDetectorPlugin::getTargetFreq(uint8_t idx) const
{
    float                       freq            = 0.0F;
    AudioToneDetector*          audioToneDetector   = AudioService::getInstance().getAudioToneDetector(idx);
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if (nullptr != audioToneDetector)
    {
        freq = audioToneDetector->getTargetFreq();
    }

    return freq;
}

void SignalDetectorPlugin::setTargetFreq(uint8_t idx, float freq)
{
    AudioToneDetector*          audioToneDetector   = AudioService::getInstance().getAudioToneDetector(idx);
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if (nullptr != audioToneDetector)
    {
        if (freq != audioToneDetector->getTargetFreq())
        {
            audioToneDetector->setTargetFreq(freq);

            (void)saveConfiguration();
        }
    }
}

uint32_t SignalDetectorPlugin::getMinDuration(uint8_t idx) const
{
    uint32_t                    minDuration         = 0U;
    AudioToneDetector*          audioToneDetector   = AudioService::getInstance().getAudioToneDetector(idx);
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if (nullptr != audioToneDetector)
    {
        minDuration = audioToneDetector->getMinDuration();
    }

    return minDuration;
}

void SignalDetectorPlugin::setMinDuration(uint8_t idx, uint32_t duration)
{
    AudioToneDetector*          audioToneDetector   = AudioService::getInstance().getAudioToneDetector(idx);
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if (nullptr != audioToneDetector)
    {
        if (duration != audioToneDetector->getMinDuration())
        {
            audioToneDetector->setMinDuration(duration);

            (void)saveConfiguration();
        }
    }
}

float SignalDetectorPlugin::getThreshold(uint8_t idx) const
{
    float                       threshold           = 0.0F;
    AudioToneDetector*          audioToneDetector   = AudioService::getInstance().getAudioToneDetector(idx);
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if (nullptr != audioToneDetector)
    {
        threshold = audioToneDetector->getThreshold();
    }

    return threshold;
}

void SignalDetectorPlugin::setThreshold(uint8_t idx, float threshold)
{
    AudioToneDetector*          audioToneDetector   = AudioService::getInstance().getAudioToneDetector(idx);
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if (nullptr != audioToneDetector)
    {
        if (threshold != audioToneDetector->getThreshold())
        {
            audioToneDetector->setThreshold(threshold);

            (void)saveConfiguration();
        }
    }
}

String SignalDetectorPlugin::getText() const
{
    String                      formattedText;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    formattedText = m_textWidget.getFormatStr();

    return formattedText;
}

void SignalDetectorPlugin::setText(const String& formatText)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (formatText != m_textWidget.getFormatStr())
    {
        m_textWidget.setFormatStr(formatText);
        (void)saveConfiguration();
    }
}

String SignalDetectorPlugin::getPushUrl() const
{
    String                      url;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    url = m_pushUrl;

    return url;
}

void SignalDetectorPlugin::setPushUrl(const String& url)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (url != m_pushUrl)
    {
        m_pushUrl = url;
        (void)saveConfiguration();
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

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
        if (0U != url.startsWith(GET_CMD))
        {
            url = url.substring(strlen(GET_CMD));
            isGet = true;
        }
        else if (0U != url.startsWith(POST_CMD))
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
    m_client.regOnResponse([](const HttpResponse& rsp){
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

bool SignalDetectorPlugin::saveConfiguration() const
{
    bool                status                  = true;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    String              configurationFilename   = getFullPathToConfiguration();
    uint8_t             idx                     = 0U;
    JsonArray           jsonTones               = jsonDoc.createNestedArray("tones");

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

    jsonDoc["text"]     = m_textWidget.getFormatStr();
    jsonDoc["pushUrl"]  = m_pushUrl;
    
    if (false == jsonFile.save(configurationFilename, jsonDoc))
    {
        LOG_WARNING("Failed to save file %s.", configurationFilename.c_str());
        status = false;
    }
    else
    {
        LOG_INFO("File %s saved.", configurationFilename.c_str());
    }

    return status;
}

bool SignalDetectorPlugin::loadConfiguration()
{
    bool                status                  = true;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    String              configurationFilename   = getFullPathToConfiguration();

    if (false == jsonFile.load(configurationFilename, jsonDoc))
    {
        LOG_WARNING("Failed to load file %s.", configurationFilename.c_str());
        status = false;
    }
    else
    {
        JsonArrayConst      jsonTones   = jsonDoc["tones"];
        JsonVariantConst    jsonText    = jsonDoc["text"];
        JsonVariantConst    jsonPushUrl = jsonDoc["pushUrl"];

        if (true == jsonTones.isNull())
        {
            LOG_WARNING("Tone(s) not found or invalid type.");
            status = false;
        }
        else if (false == jsonText.is<String>())
        {
            LOG_WARNING("Text not found or invalid type.");
            status = false;
        }
        else if (false == jsonPushUrl.is<String>())
        {
            LOG_WARNING("Push URL not found or invalid type.");
            status = false;
        }
        else
        {
            JsonArrayConst::iterator    it;
            uint8_t                     idx = 0U;

            for(it = jsonTones.begin(); it != jsonTones.end(); ++it)
            {
                AudioToneDetector*  audioToneDetector   = AudioService::getInstance().getAudioToneDetector(idx);

                if (nullptr == audioToneDetector)
                {
                    LOG_WARNING("Too many tone detector configurations.");
                    break;
                }
                else
                {
                    JsonVariantConst jsonTargetFreq     = (*it)["frequency"];
                    JsonVariantConst jsonMinDuration    = (*it)["minDuration"];
                    JsonVariantConst jsonThreshold      = (*it)["threshold"];

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

            m_textWidget.setFormatStr(jsonText.as<String>());
            m_pushUrl = jsonPushUrl.as<String>();
        }
    }

    return status;
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

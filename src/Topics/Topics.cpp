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
 * @file   Topics.cpp
 * @brief  Topics
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Topics.h"
#include <SettingsService.h>
#include <TopicHandlerService.h>
#include <Util.h>
#include <PsramJsonDocument.hpp>
#include <Json.hpp>
#include "DisplayMgr.h"
#include "RestartMgr.h"
#include "ButtonActions.h"
#include "SysMsg.h"

#include <Logging.h>

#include <string.h>

#if (0 != CONFIG_MELODY_SERVICE_ENABLE)

#include <MelodyService.h>

#endif /* (0 != CONFIG_MELODY_SERVICE_ENABLE) */

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/** Topic data element. */
typedef struct
{
    const char*                         entity;          /**< The entity which provides the topic. */
    const char*                         topic;           /**< The feature topic. */
    ITopicHandler::GetTopicFunc         getTopicFunc;    /**< Function to read the feature topic content. */
    TopicHandlerService::HasChangedFunc hasChangedFunc;  /**< Function to checked whether the feature topic content changed. */
    ITopicHandler::SetTopicFunc         setTopicFunc;    /**< Function to write the feature topic content. */
    const char*                         extraHAFileName; /**< File name of a file with extra Home Assistant information. */

} TopicElem;

/**
 * Virtual button which can be triggered.
 */
class VirtualButton : public ButtonActions
{
public:

    /**
     * Construct virtual button instance.
     */
    VirtualButton() :
        ButtonActions()
    {
    }

    /**
     * Destroy virtual button instance.
     */
    virtual ~VirtualButton()
    {
    }

    /**
     * Execute action by button action id.
     *
     * @param[in] id    Button action id
     */
    void executeAction(ButtonActionId id)
    {
        ButtonActions::executeAction(id, true);
    }

private:

    /* Make copy constructor and assignment operator unavailable */
    VirtualButton(const VirtualButton& button);
    VirtualButton& operator=(const VirtualButton& button);
};

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static bool execButtonAction(const String& topic, const JsonObjectConst& value);
static bool parseUInt8(JsonVariantConst jsonValue, uint8_t& value);
static bool parseUInt32(JsonVariantConst jsonValue, uint32_t& value);
static bool getDisplayState(const String& topic, JsonObject& value);
static bool hasDisplayStateChanged(const String& topic);
static bool setDisplayState(const String& topic, const JsonObjectConst& value);
static bool getAutoBrightnessState(const String& topic, JsonObject& value);
static bool hasAutoBrightnessStateChanged(const String& topic);
static bool setAutoBrightnessState(const String& topic, const JsonObjectConst& value);
static bool restart(const String& topic, const JsonObjectConst& value);
static bool notify(const String& topic, const JsonObjectConst& value);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Default duration of a notification in ms, if none or 0 is given. */
static const uint32_t NOTIFY_DURATION_DEFAULT        = 10000U;

/** Maximum duration of a notification in ms. A greater value will be limited. */
static const uint32_t NOTIFY_DURATION_MAX            = 60000U;

/**
 * Maximum length of a notification text in characters.
 *
 * The text is copied into the message queue, which holds several entries. Without
 * a limit a few requests could occupy a noticeable amount of heap on a device
 * without PSRAM. A too long text is rejected and not truncated, so the sender
 * notices it instead of seeing a cut off message.
 */
static const size_t NOTIFY_TEXT_MAX_LENGTH           = 256U;

/**
 * Maximum length of a notification icon file name in characters. It is queued
 * together with the text, see NOTIFY_TEXT_MAX_LENGTH. A too long file name is
 * ignored, because it can not be a valid path anyway, but the notification
 * itself is shown.
 */
static const size_t NOTIFY_ICON_FILE_NAME_MAX_LENGTH = 128U;

/**
 * Minimum brightness difference which is reported as a change.
 *
 * The automatic brightness control adapts the brightness continously to the
 * ambient light. Without a threshold every single step would be published,
 * which would spam the MQTT broker and the Home Assistant history.
 */
static const uint8_t BRIGHTNESS_CHANGE_THRESHOLD     = 8U;

/**
 * Device id, which is unique for each device.
 */
static String gDeviceId;

/* clang-format off */

/**
 * List of topics, sorted by topics in ascending order.
 *
 * ENTITY-ID     : display/plugin/PLUGIN-ALIAS | empty
 *
 * REST API      : BASE-URL/[ENTITY-ID/]TOPIC
 *
 * MQTT topic(s) : DEVICE-ID[/ENTITY-ID]/TOPIC[/ENTITY-INDEX]/set (writeable)
 *                 DEVICE-ID[/ENTITY-ID]/TOPIC[/ENTITY-INDEX]/state (readable)
 *
 * HomeAssistant : NODE-ID = DEVICE-ID with "/" and "." replaced by "_"
 *                 OBJECT-ID = [ENTITY-ID/]TOPIC[/ENTITIY-INDEX] with "/" and "." replaced by "_"
 *                 UNIQUE-ID = NODE-ID/OBJECT-ID
 *                 DISCOVERY-TOPIC = DISCOVERY-PREFIX/COMPONENT/NODE-ID/OBJECT-ID/config
 */
static TopicElem gTopicList[] = {
    /* ENTITY-ID    TOPIC               GET                     HAS-CHANGED                     SET                     EXTRA-HA-FILE */
    { "display",    "autoBrightness",   getAutoBrightnessState, hasAutoBrightnessStateChanged,  setAutoBrightnessState, "/extra/autoBrightness.json" },
    { "",           "button",           nullptr,                nullptr,                        execButtonAction,       "/extra/button.json"         },
    { "",           "notify",           nullptr,                nullptr,                        notify,                 "/extra/notify.json"         },
    { "display",    "power",            getDisplayState,        hasDisplayStateChanged,         setDisplayState,        "/extra/display.json"        },
    { "",           "restart",          nullptr,                nullptr,                        restart,                "/extra/restart.json"        }
};

/* clang-format on */

/**
 * Last display on state.
 */
static bool gLastDisplayOnState      = false;

/**
 * Last reported display brightness.
 */
static uint8_t gLastBrightness       = 0U;

/**
 * Last automatic brightness adjustment state.
 */
static bool gLastAutoBrightnessState = false;

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

void Topics::begin()
{
    SettingsService&  settings      = SettingsService::getInstance();
    const size_t      JSON_DOC_SIZE = Json::getJsonSize(256U);
    PsramJsonDocument jsonDocExtra(JSON_DOC_SIZE);
    size_t            idx;

    gDeviceId = settings.getHostname().getValueFromRam();

#if (0 != CONFIG_MELODY_SERVICE_ENABLE)

    /* Connect the notification to the melody service. The system message plugin
     * itself has no audio knowledge.
     */
    SysMsg::getInstance().registerMelodyHandler(
        [](const String& melody, uint8_t volume) {
            (void)MelodyService::getInstance().playMelody(melody, volume);
        });

#endif /* (0 != CONFIG_MELODY_SERVICE_ENABLE) */

    /* Register topics */
    for (idx = 0U; idx < UTIL_ARRAY_NUM(gTopicList); ++idx)
    {
        TopicElem*      topicElem = &gTopicList[idx];
        JsonObjectConst jsonExtra;

        jsonDocExtra["ha"] = topicElem->extraHAFileName;
        jsonExtra          = jsonDocExtra.as<JsonObjectConst>();

        TopicHandlerService::getInstance().registerTopic(
            gDeviceId,
            topicElem->entity,
            topicElem->topic,
            jsonExtra,
            topicElem->getTopicFunc,
            topicElem->hasChangedFunc,
            topicElem->setTopicFunc,
            nullptr);
    }
}

void Topics::end()
{
    size_t idx;

    /* Unregister topics */
    for (idx = 0U; idx < UTIL_ARRAY_NUM(gTopicList); ++idx)
    {
        TopicElem* topicElem = &gTopicList[idx];

        TopicHandlerService::getInstance().unregisterTopic(
            gDeviceId,
            topicElem->entity,
            topicElem->topic);
    }
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Execute a button action.
 *
 * @param[in]   topic   Topic
 * @param[in]   value   Value
 *
 * @return If successful, it will return true otherwise false.
 */
static bool execButtonAction(const String& topic, const JsonObjectConst& value)
{
    bool             isSuccessful = true;
    int32_t          i32ActionId  = BUTTON_ACTION_ID_MAX;
    ButtonActionId   actionId     = BUTTON_ACTION_ID_ACTIVATE_NEXT_SLOT; /* Default */
    JsonVariantConst jsonActionId = value["actionId"];

    UTIL_NOT_USED(topic);

    /* Action id validation? */
    if (false == jsonActionId.isNull())
    {
        if (true == jsonActionId.is<String>())
        {
            i32ActionId = jsonActionId.as<String>().toInt();
        }
        else if (true == jsonActionId.is<int>())
        {
            i32ActionId = jsonActionId.as<int>();
        }
        else
        {
            isSuccessful = false;
        }

        if (BUTTON_ACTION_ID_MAX > i32ActionId)
        {
            actionId = static_cast<ButtonActionId>(i32ActionId);
        }
    }

    if (true == isSuccessful)
    {
        VirtualButton button;

        button.executeAction(actionId);
    }

    return isSuccessful;
}

/**
 * Convert a JSON value to a 8-bit unsigned integer.
 *
 * Via MQTT and via a JSON body of the REST API the value arrives as JSON
 * number. Via the HTTP parameters of the REST API it arrives as string.
 * Therefore both are accepted, everything else is rejected. The destination
 * is kept untouched if the value can not be converted.
 *
 * @param[in]   jsonValue   The JSON value to convert.
 * @param[out]  value       The converted value.
 *
 * @return If successful, it will return true otherwise false.
 */
static bool parseUInt8(JsonVariantConst jsonValue, uint8_t& value)
{
    bool isSuccessful = false;

    /* A negative number is rejected by the unsigned type check already. */
    if (true == jsonValue.is<uint32_t>())
    {
        uint32_t rawValue = jsonValue.as<uint32_t>();

        if (UINT8_MAX >= rawValue)
        {
            value        = static_cast<uint8_t>(rawValue);
            isSuccessful = true;
        }
    }
    else if (true == jsonValue.is<const char*>())
    {
        /* The conversion validates the range as well. */
        isSuccessful = Util::strToUInt8(jsonValue.as<const char*>(), value);
    }
    else
    {
        ;
    }

    return isSuccessful;
}

/**
 * Convert a JSON value to a 32-bit unsigned integer.
 *
 * See parseUInt8() for the accepted representations.
 *
 * @param[in]   jsonValue   The JSON value to convert.
 * @param[out]  value       The converted value.
 *
 * @return If successful, it will return true otherwise false.
 */
static bool parseUInt32(JsonVariantConst jsonValue, uint32_t& value)
{
    bool isSuccessful = false;

    if (true == jsonValue.is<uint32_t>())
    {
        value        = jsonValue.as<uint32_t>();
        isSuccessful = true;
    }
    else if (true == jsonValue.is<const char*>())
    {
        isSuccessful = Util::strToUInt32(jsonValue.as<const char*>(), value);
    }
    else
    {
        ;
    }

    return isSuccessful;
}

/**
 * Get display state.
 *
 * @param[in]   topic   Topic
 * @param[out]  value   Value
 *
 * @return If successful, it will return true otherwise false.
 */
static bool getDisplayState(const String& topic, JsonObject& value)
{
    DisplayMgr& displayMgr  = DisplayMgr::getInstance();
    bool        isDisplayOn = displayMgr.isDisplayOn();

    UTIL_NOT_USED(topic);

    if (false == isDisplayOn)
    {
        value["state"] = "off";
    }
    else
    {
        value["state"] = "on";
    }

    value["brightness"] = displayMgr.getBrightness();

    return true;
}

/**
 * Has the display state changed?
 *
 * @param[in]   topic   Topic
 *
 * @return If the display state has changed, it will return true otherwise false.
 */
static bool hasDisplayStateChanged(const String& topic)
{
    DisplayMgr& displayMgr  = DisplayMgr::getInstance();
    bool        isDisplayOn = displayMgr.isDisplayOn();
    uint8_t     brightness  = displayMgr.getBrightness();
    bool        hasChanged  = false;

    UTIL_NOT_USED(topic);

    if (isDisplayOn != gLastDisplayOnState)
    {
        gLastDisplayOnState = isDisplayOn;
        hasChanged          = true;
    }

    if (brightness != gLastBrightness)
    {
        uint8_t difference = (brightness > gLastBrightness) ? (brightness - gLastBrightness) : (gLastBrightness - brightness);

        /* Only a noticeable difference is reported, see BRIGHTNESS_CHANGE_THRESHOLD.
         * Both limits are always reported, otherwise the brightness would never
         * be shown as completely dark or completely bright.
         */
        if ((BRIGHTNESS_CHANGE_THRESHOLD <= difference) ||
            (0U == brightness) ||
            (UINT8_MAX == brightness))
        {
            gLastBrightness = brightness;
            hasChanged      = true;
        }
    }

    return hasChanged;
}

/**
 * Set display state.
 *
 * @param[in]   topic   Topic
 * @param[in]   value   Value
 *
 * @return If successful, it will return true otherwise false.
 */
static bool setDisplayState(const String& topic, const JsonObjectConst& value)
{
    bool             isSuccessful   = false;
    JsonVariantConst jsonState      = value["state"];
    JsonVariantConst jsonBrightness = value["brightness"];
    bool             hasState       = (false == jsonState.isNull());
    bool             hasBrightness  = (false == jsonBrightness.isNull());
    bool             isValid        = true;
    bool             displayOn      = false;
    uint8_t          brightness     = 0U;

    UTIL_NOT_USED(topic);

    /* The display state is optional, but if given it shall be valid. */
    if (true == hasState)
    {
        String state = jsonState.as<const char*>();

        if (true == state.equalsIgnoreCase("off"))
        {
            displayOn = false;
        }
        else if (true == state.equalsIgnoreCase("on"))
        {
            displayOn = true;
        }
        else
        {
            isValid = false;
        }
    }

    /* The brightness is optional, but if given it shall be in range. */
    if ((true == hasBrightness) &&
        (false == parseUInt8(jsonBrightness, brightness)))
    {
        isValid = false;
    }

    /* At least one of both shall be given, otherwise there is nothing to do. */
    if ((false == hasState) &&
        (false == hasBrightness))
    {
        /* Nothing to set. */
        ;
    }
    else if (false == isValid)
    {
        /* Reject the whole request, to never apply it only partly. */
        ;
    }
    else
    {
        DisplayMgr& displayMgr = DisplayMgr::getInstance();

        /* The brightness is set first, so that the display doesn't shortly
         * appear with the previous brightness after it is switched on.
         */
        if (true == hasBrightness)
        {
            displayMgr.setBrightness(brightness);
        }

        if (true == hasState)
        {
            if (false == displayOn)
            {
                displayMgr.displayOff();
            }
            else
            {
                displayMgr.displayOn();
            }
        }

        isSuccessful = true;
    }

    return isSuccessful;
}

/**
 * Get automatic brightness adjustment state.
 *
 * @param[in]   topic   Topic
 * @param[out]  value   Value
 *
 * @return If successful, it will return true otherwise false.
 */
static bool getAutoBrightnessState(const String& topic, JsonObject& value)
{
    DisplayMgr& displayMgr = DisplayMgr::getInstance();

    UTIL_NOT_USED(topic);

    if (false == displayMgr.getAutoBrightnessAdjustment())
    {
        value["state"] = "off";
    }
    else
    {
        value["state"] = "on";
    }

    return true;
}

/**
 * Has the automatic brightness adjustment state changed?
 *
 * @param[in]   topic   Topic
 *
 * @return If the automatic brightness adjustment state has changed, it will return true otherwise false.
 */
static bool hasAutoBrightnessStateChanged(const String& topic)
{
    DisplayMgr& displayMgr = DisplayMgr::getInstance();
    bool        isEnabled  = displayMgr.getAutoBrightnessAdjustment();
    bool        hasChanged = false;

    UTIL_NOT_USED(topic);

    if (isEnabled != gLastAutoBrightnessState)
    {
        gLastAutoBrightnessState = isEnabled;
        hasChanged               = true;
    }

    return hasChanged;
}

/**
 * Set automatic brightness adjustment state.
 *
 * @param[in]   topic   Topic
 * @param[in]   value   Value
 *
 * @return If successful, it will return true otherwise false.
 */
static bool setAutoBrightnessState(const String& topic, const JsonObjectConst& value)
{
    bool             isSuccessful = false;
    JsonVariantConst jsonState    = value["state"];

    UTIL_NOT_USED(topic);

    if (false == jsonState.isNull())
    {
        DisplayMgr& displayMgr = DisplayMgr::getInstance();
        String      state      = jsonState.as<const char*>();

        /* The adjustment may be rejected, e.g. if the board has no ambient
         * light sensor at all.
         */
        if (true == state.equalsIgnoreCase("off"))
        {
            isSuccessful = displayMgr.setAutoBrightnessAdjustment(false);
        }
        else if (true == state.equalsIgnoreCase("on"))
        {
            isSuccessful = displayMgr.setAutoBrightnessAdjustment(true);
        }
        else
        {
            ;
        }
    }

    return isSuccessful;
}

/**
 * Show a notification on the display.
 *
 * Mandatory is only the text. The duration is limited, so a notification can
 * not block the display. A problem with the optional icon or melody never
 * suppresses the notification itself.
 *
 * @param[in] topic The topic.
 * @param[in] value The notification in JSON format.
 *
 * @return If successful, it will return true otherwise false.
 */
static bool notify(const String& topic, const JsonObjectConst& value)
{
    bool             isSuccessful     = false;
    JsonVariantConst jsonText         = value["text"];
    JsonVariantConst jsonDuration     = value["duration"];
    JsonVariantConst jsonIconFileName = value["iconFileName"];
    const char*      text             = jsonText.as<const char*>();

    UTIL_NOT_USED(topic);

    /* Guard path: without a text there is nothing to show. */
    if (nullptr == text)
    {
        LOG_WARNING("Notification without text.");
    }
    /* Guard path: the text is queued, therefore its length is limited. */
    else if (NOTIFY_TEXT_MAX_LENGTH < strlen(text))
    {
        LOG_WARNING("Notification text too long.");
    }
    else
    {
        uint32_t duration = NOTIFY_DURATION_DEFAULT;
        String   iconFileName;
        String   melody;
        uint8_t  volume = 0U;

        if (true == parseUInt32(jsonDuration, duration))
        {
            /* A duration of 0 would show the notification infinite, because the
             * system message slot has no slot duration.
             */
            if (0U == duration)
            {
                duration = NOTIFY_DURATION_DEFAULT;
            }
            else if (NOTIFY_DURATION_MAX < duration)
            {
                duration = NOTIFY_DURATION_MAX;
            }
            else
            {
                /* Duration is in the valid range. */
                ;
            }
        }

        {
            const char* iconFileNamePtr = jsonIconFileName.as<const char*>();

            if (nullptr == iconFileNamePtr)
            {
                /* No icon. */
                ;
            }
            else if (NOTIFY_ICON_FILE_NAME_MAX_LENGTH < strlen(iconFileNamePtr))
            {
                LOG_WARNING("Notification icon file name too long.");
            }
            else
            {
                iconFileName = iconFileNamePtr;
            }
        }

#if (0 != CONFIG_MELODY_SERVICE_ENABLE)

        {
            JsonVariantConst jsonMelody = value["melody"];
            JsonVariantConst jsonVolume = value["volume"];

            const char*      melodyPtr  = jsonMelody.as<const char*>();

            if (nullptr != melodyPtr)
            {
                /* The melody is queued as well, therefore its length is limited
                 * too. It is rejected and not truncated, because a cut off RTTTL
                 * string is either invalid or sounds wrong. A rejected melody
                 * does not suppress the notification itself.
                 */
                if (RTTTL::MAX_MELODY_LENGTH < strlen(melodyPtr))
                {
                    LOG_WARNING("Notification melody too long.");
                }
                else
                {
                    melody = melodyPtr;
                    volume = RTTTL::MAX_VOLUME;

                    (void)parseUInt8(jsonVolume, volume);
                }
            }
        }

#endif /* (0 != CONFIG_MELODY_SERVICE_ENABLE) */

        /* The melody travels with the notification and is started when it
         * appears. Playing it here would sound to the previous message, or to a
         * notification which is never shown because the queue is full.
         */
        isSuccessful = SysMsg::getInstance().showNotification(text, duration, iconFileName, melody, volume);
    }

    return isSuccessful;
}

/**
 * Restart the device.
 *
 * @param[in]   topic   Topic
 * @param[in]   value   Value
 *
 * @return If successful, it will return true otherwise false.
 */
static bool restart(const String& topic, const JsonObjectConst& value)
{
    const uint32_t RESTART_DELAY = 100U; /* ms */

    UTIL_NOT_USED(topic);
    UTIL_NOT_USED(value);

    /* To ensure that a positive response will be sent before the device restarts,
     * a short delay is necessary.
     */
    (void)RestartMgr::getInstance().reqRestart(RESTART_DELAY, false);

    return true;
}

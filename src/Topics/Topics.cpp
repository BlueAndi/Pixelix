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
#include "DisplayMgr.h"
#include "RestartMgr.h"
#include "ButtonActions.h"

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
static bool getDisplayState(const String& topic, JsonObject& value);
static bool hasDisplayStateChanged(const String& topic);
static bool setDisplayState(const String& topic, const JsonObjectConst& value);
static bool restart(const String& topic, const JsonObjectConst& value);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/**
 * Device id, which is unique for each device.
 */
static String gDeviceId;

/* clang-format off */

/**
 * List of topics, sorted by topics in ascending order.
 *
 * ENTITY-ID     : display/uid/PLUGIN-UID | display/alias/PLUGIN-ALIAS | empty
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
    /* ENTITY-ID    TOPIC       GET                 HAS-CHANGED             SET                 EXTRA-HA-FILE */
    { "",           "button",   nullptr,            nullptr,                execButtonAction,   "/extra/button.json"  },
    { "display",    "power",    getDisplayState,    hasDisplayStateChanged, setDisplayState,    "/extra/display.json" },
    { "",           "restart",  nullptr,            nullptr,                restart,            "/extra/restart.json" }
};

/* clang-format on */

/**
 * Last display on state.
 */
static bool gLastDisplayOnState = false;

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
    SettingsService&    settings      = SettingsService::getInstance();
    const size_t        JSON_DOC_SIZE = 256U;
    DynamicJsonDocument jsonDocExtra(JSON_DOC_SIZE);
    size_t              idx;

    if (false == settings.open(true))
    {
        gDeviceId = settings.getHostname().getDefault();
    }
    else
    {
        gDeviceId = settings.getHostname().getValue();

        settings.close();
    }

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
    bool        hasChanged  = false;

    UTIL_NOT_USED(topic);

    if (isDisplayOn != gLastDisplayOnState)
    {
        gLastDisplayOnState = isDisplayOn;
        hasChanged          = true;
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
    bool             isSuccessful = false;
    JsonVariantConst jsonState    = value["state"];

    UTIL_NOT_USED(topic);

    if (false == jsonState.isNull())
    {
        bool   displayOn = false;
        String state     = jsonState.as<const char*>();

        if (true == state.equalsIgnoreCase("off"))
        {
            displayOn    = false;
            isSuccessful = true;
        }
        else if (true == state.equalsIgnoreCase("on"))
        {
            displayOn    = true;
            isSuccessful = true;
        }
        else
        {
            ;
        }

        if (true == isSuccessful)
        {
            DisplayMgr& displayMgr = DisplayMgr::getInstance();

            if (false == displayOn)
            {
                displayMgr.displayOff();
            }
            else
            {
                displayMgr.displayOn();
            }
        }
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

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
#include "UpdateMgr.h"

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
    const char*                         entity;         /**< Entity */
    const char*                         topic;          /**< Topic */
    ITopicHandler::GetTopicFunc         getTopicFunc;   /**< Get topic function */
    TopicHandlerService::HasChangedFunc hasChangedFunc; /**< Has changed function */
    ITopicHandler::SetTopicFunc         setTopicFunc;   /**< Set topic function */
    const char*                         extraFileName;  /**< File name of a file with extra information. */

} TopicElem;

/******************************************************************************
 * Prototypes
 *****************************************************************************/

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

/**
 * List of topics.
 */
static TopicElem gTopicList[] = {
    { "display", "/power", getDisplayState, hasDisplayStateChanged, setDisplayState, "/extra/display.json" },
    { "device", "/restart", nullptr, nullptr, restart, "/extra/restart.json" }
};

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
    SettingsService& settings = SettingsService::getInstance();
    size_t           idx;

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
        TopicElem* topicElem = &gTopicList[idx];

        TopicHandlerService::getInstance().registerTopic(
            gDeviceId,
            topicElem->entity,
            topicElem->topic,
            topicElem->extraFileName,
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
        String state     = jsonState.as<String>();

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
    UpdateMgr::getInstance().reqRestart(RESTART_DELAY);

    return true;
}

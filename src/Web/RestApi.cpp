/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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
 * @brief  REST pages
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "RestApi.h"
#include "HttpStatus.h"
#include "Settings.h"
#include "DisplayMgr.h"
#include "Version.h"
#include "PluginMgr.h"
#include "WiFiUtil.h"
#include "FileSystem.h"

#include <Util.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Esp.h>
#include <Logging.h>
#include <SensorDataProvider.h>

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

static void handleButton(AsyncWebServerRequest* request);
static void handleFadeEffect(AsyncWebServerRequest* request);
static void handleSlots(AsyncWebServerRequest* request);
static void handlePluginInstall(AsyncWebServerRequest* request);
static void handlePluginUninstall(AsyncWebServerRequest* request);
static void handlePlugins(AsyncWebServerRequest* request);
static void handleSensors(AsyncWebServerRequest* request);
static void handleSettings(AsyncWebServerRequest* request);
static void handleSetting(AsyncWebServerRequest* request);
static bool storeSetting(KeyValue* parameter, const String& value, String& error);
static void handleStatus(AsyncWebServerRequest* request);
static void handleFilesystem(AsyncWebServerRequest* request);
static void handleFileGet(AsyncWebServerRequest* request);
static String getContentType(const String& filename);
static void handleFilePost(AsyncWebServerRequest* request);
static void uploadHandler(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);
static void handleFileDelete(AsyncWebServerRequest* request);
static bool isValidHostname(const String& hostname);

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

void RestApi::init(AsyncWebServer& srv)
{
    (void)srv.on("/rest/api/v1/button", handleButton);
    (void)srv.on("/rest/api/v1/display/fadeEffect", handleFadeEffect);
    (void)srv.on("/rest/api/v1/display/slots", handleSlots);
    (void)srv.on("/rest/api/v1/plugin/install", handlePluginInstall);
    (void)srv.on("/rest/api/v1/plugin/uninstall", handlePluginUninstall);
    (void)srv.on("/rest/api/v1/plugins", handlePlugins);
    (void)srv.on("/rest/api/v1/sensors", handleSensors);
    (void)srv.on("/rest/api/v1/settings", handleSettings);
    (void)srv.on("/rest/api/v1/setting", handleSetting);
    (void)srv.on("/rest/api/v1/status", handleStatus);
    (void)srv.on("/rest/api/v1/fs/file", HTTP_GET, handleFileGet);
    (void)srv.on("/rest/api/v1/fs/file", HTTP_POST, handleFilePost, uploadHandler);
    (void)srv.on("/rest/api/v1/fs/file", HTTP_DELETE, handleFileDelete);
    (void)srv.on("/rest/api/v1/fs", handleFilesystem);

    return;
}

/**
 * Handle invalid rest path request.
 *
 * @param[in] request   HTTP request
 */
void RestApi::error(AsyncWebServerRequest* request)
{
    String              content;
    const size_t        JSON_DOC_SIZE   = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;
    JsonObject          errorObj        = jsonDoc.createNestedObject("error");

    if (nullptr == request)
    {
        return;
    }

    /* Prepare response */
    jsonDoc["status"]   = "error";
    errorObj["msg"]     = "Invalid path requested.";
    httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;

    if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document has less memory available.");
    }
    else
    {
        LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
    }

    (void)serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Trigger virtual user button.
 * POST \c "/api/v1/button"
 *
 * @param[in] request   HTTP request
 */
static void handleButton(AsyncWebServerRequest* request)
{
    String              content;
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;
    const size_t        JSON_DOC_SIZE   = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_POST != request->method())
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = "error";
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        DisplayMgr::getInstance().activateNextSlot();
 
        /* Prepare response */
        (void)jsonDoc.createNestedObject("data");
        jsonDoc["status"]   = "ok";
        httpStatusCode      = HttpStatus::STATUS_CODE_OK;
    }

    if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document has less memory available.");
    }
    else
    {
        LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
    }

    (void)serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

/**
 * Activate next fade effect.
 * POST \c "/api/v1/display/fadeEffect"
 *
 * @param[in] request   HTTP request
 */
static void handleFadeEffect(AsyncWebServerRequest* request)
{
    String              content;
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;
    const size_t        JSON_DOC_SIZE   = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET == request->method())
    {
        JsonObject data = jsonDoc.createNestedObject("data");
        
        data["fadeEffect"] = DisplayMgr::getInstance().getFadeEffect();

        /* Prepare response */
        jsonDoc["status"]   = "ok";
        httpStatusCode      = HttpStatus::STATUS_CODE_OK;
    }
    else if (HTTP_POST == request->method())
    {
        JsonObject              data                = jsonDoc.createNestedObject("data");
        DisplayMgr::FadeEffect  currentFadeEffect   = DisplayMgr::getInstance().getFadeEffect();
        uint8_t                 fadeEffectId        = static_cast<uint8_t>(currentFadeEffect);
        DisplayMgr::FadeEffect  nextFadeEffect      = static_cast<DisplayMgr::FadeEffect>(fadeEffectId + 1);

        DisplayMgr::getInstance().activateNextFadeEffect(nextFadeEffect);

        data["fadeEffect"] = DisplayMgr::getInstance().getFadeEffect();

        /* Prepare response */
        jsonDoc["status"]   = "ok";
        httpStatusCode      = HttpStatus::STATUS_CODE_OK;
    }
    else
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = "error";
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }

    if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document has less memory available.");
    }
    else
    {
        LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
    }

    (void)serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

/**
 * Get number of slots and which plugin is installed.
 * GET \c "/api/v1/display/slots"
 *
 * @param[in] request   HTTP request
 */
static void handleSlots(AsyncWebServerRequest* request)
{
    String              content;
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;
    const size_t        JSON_DOC_SIZE   = 1024U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET != request->method())
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = "error";
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        JsonObject  dataObj     = jsonDoc.createNestedObject("data");
        JsonArray   slotArray   = dataObj.createNestedArray("slots");
        uint8_t     slotId      = 0U;
        DisplayMgr& displayMgr  = DisplayMgr::getInstance();

        /* Add max. number of slots */
        dataObj["maxSlots"] = displayMgr.getMaxSlots();

        /* Add which plugin's are installed. */
        for(slotId = 0U; slotId < displayMgr.getMaxSlots(); ++slotId)
        {
            IPluginMaintenance* plugin      = displayMgr.getPluginInSlot(slotId);
            const char*         name        = (nullptr != plugin) ? plugin->getName() : "";
            uint16_t            uid         = (nullptr != plugin) ? plugin->getUID() : 0U;
            bool                isLocked    = displayMgr.isSlotLocked(slotId);
            uint32_t            duration    = displayMgr.getSlotDuration(slotId);
            JsonObject          slot        = slotArray.createNestedObject();

            slot["name"]        = name;
            slot["uid"]         = uid;
            slot["isLocked"]    = isLocked;
            slot["duration"]    = duration;
        }

        /* Prepare response */
        jsonDoc["status"]   = "ok";
        httpStatusCode      = HttpStatus::STATUS_CODE_OK;
    }

    if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document has less memory available.");
    }
    else
    {
        LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
    }

    (void)serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

/**
 * Install plugin
 * POST \c "/api/v1/plugin/install?name=<plugin-name>"
 *
 * @param[in] request   HTTP request
 */
static void handlePluginInstall(AsyncWebServerRequest* request)
{
    String              content;
    const size_t        JSON_DOC_SIZE   = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_POST != request->method())
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = "error";
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        /* Plugin name missing? */
        if (false == request->hasArg("name"))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = "error";
            errorObj["msg"]     = "Plugin name is missing.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else
        {
            String              pluginName  = request->arg("name");
            IPluginMaintenance* plugin      = PluginMgr::getInstance().install(pluginName);

            /* Plugin not found? */
            if (nullptr == plugin)
            {
                JsonObject errorObj = jsonDoc.createNestedObject("error");

                /* Prepare response */
                jsonDoc["status"]   = "error";
                errorObj["msg"]     = "Plugin unknown.";
                httpStatusCode      = HttpStatus::STATUS_CODE_METHOD_NOT_ALLOWED;
            }
            /* Plugin successful installed. */
            else
            {
                JsonObject dataObj = jsonDoc.createNestedObject("data");

                plugin->enable();

                /* Save current installed plugins to persistent memory. */
                PluginMgr::getInstance().save();

                /* Prepare response */
                dataObj["slotId"]   = DisplayMgr::getInstance().getSlotIdByPluginUID(plugin->getUID());
                dataObj["uid"]      = plugin->getUID();
                jsonDoc["status"]   = "ok";
                httpStatusCode      = HttpStatus::STATUS_CODE_OK;
            }
        }
    }

    if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document has less memory available.");
    }
    else
    {
        LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
    }

    (void)serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

/**
 * Uninstall plugin
 * POST \c "/api/v1/plugin/uninstall?name=<plugin-name>&slotId=<slot-id>"
 *
 * @param[in] request   HTTP request
 */
static void handlePluginUninstall(AsyncWebServerRequest* request)
{
    String              content;
    const size_t        JSON_DOC_SIZE   = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_POST != request->method())
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = "error";
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        /* Plugin name missing? */
        if (false == request->hasArg("name"))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = "error";
            errorObj["msg"]     = "Plugin name is missing.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        /* Slot id missing? */
        else if (false == request->hasArg("slotId"))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = "error";
            errorObj["msg"]     = "Slot id is missing.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else
        {
            uint8_t slotId          = DisplayMgr::SLOT_ID_INVALID;
            bool    slotIdStatus    = Util::strToUInt8(request->arg("slotId"), slotId);

            if (false == slotIdStatus)
            {
                JsonObject errorObj = jsonDoc.createNestedObject("error");

                /* Prepare response */
                jsonDoc["status"]   = "error";
                errorObj["msg"]     = "Invalid slot id.";
                httpStatusCode      = HttpStatus::STATUS_CODE_METHOD_NOT_ALLOWED;
            }
            else
            {
                String              pluginName  = request->arg("name");
                IPluginMaintenance* plugin      = DisplayMgr::getInstance().getPluginInSlot(slotId);

                if (nullptr == plugin)
                {
                    JsonObject errorObj = jsonDoc.createNestedObject("error");

                    /* Prepare response */
                    jsonDoc["status"]   = "error";
                    errorObj["msg"]     = "No plugin in slot.";
                    httpStatusCode      = HttpStatus::STATUS_CODE_METHOD_NOT_ALLOWED;
                }
                else if (0 != pluginName.compareTo(plugin->getName()))
                {
                    JsonObject errorObj = jsonDoc.createNestedObject("error");

                    /* Prepare response */
                    jsonDoc["status"]   = "error";
                    errorObj["msg"]     = "Wrong plugin in slot.";
                    httpStatusCode      = HttpStatus::STATUS_CODE_METHOD_NOT_ALLOWED;
                }
                else if (true == DisplayMgr::getInstance().isSlotLocked(slotId))
                {
                    JsonObject errorObj = jsonDoc.createNestedObject("error");

                    /* Prepare response */
                    jsonDoc["status"]   = "error";
                    errorObj["msg"]     = "Slot is locked.";
                    httpStatusCode      = HttpStatus::STATUS_CODE_METHOD_NOT_ALLOWED;
                }
                else if (false == PluginMgr::getInstance().uninstall(plugin))
                {
                    JsonObject errorObj = jsonDoc.createNestedObject("error");

                    /* Prepare response */
                    jsonDoc["status"]   = "error";
                    errorObj["msg"]     = "Failed to uninstall.";
                    httpStatusCode      = HttpStatus::STATUS_CODE_METHOD_NOT_ALLOWED;
                }
                else
                {
                    /* Save current installed plugins to persistent memory. */
                    PluginMgr::getInstance().save();

                    /* Prepare response */
                    (void)jsonDoc.createNestedObject("data");
                    jsonDoc["status"]   = "ok";
                    httpStatusCode      = HttpStatus::STATUS_CODE_OK;
                }
            }
        }
    }

    if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document has less memory available.");
    }
    else
    {
        LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
    }

    (void)serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

/**
 * List all available plugins.
 * GET \c "/api/v1/plugins"
 *
 * @param[in] request   HTTP request
 */
static void handlePlugins(AsyncWebServerRequest* request)
{
    String              content;
    const size_t        JSON_DOC_SIZE   = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET != request->method())
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = "error";
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        JsonObject  dataObj     = jsonDoc.createNestedObject("data");
        JsonArray   pluginArray = dataObj.createNestedArray("plugins");
        const char* pluginName  = PluginMgr::getInstance().findFirst();

        while(nullptr != pluginName)
        {
            pluginArray.add(pluginName);
            pluginName = PluginMgr::getInstance().findNext();
        }

        /* Prepare response */
        jsonDoc["status"]   = "ok";
        httpStatusCode      = HttpStatus::STATUS_CODE_OK;
    }

    if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document has less memory available.");
    }
    else
    {
        LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
    }

    (void)serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

/**
 * List all sensors.
 * GET \c "/api/v1/sensors"
 *
 * @param[in] request   HTTP request
 */
static void handleSensors(AsyncWebServerRequest* request)
{
    String              content;
    const size_t        JSON_DOC_SIZE   = 1024U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET != request->method())
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = "error";
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        JsonObject          dataObj         = jsonDoc.createNestedObject("data");
        JsonArray           sensorsArray    = dataObj.createNestedArray("sensors");
        SensorDataProvider& sensorDataProv  = SensorDataProvider::getInstance();
        uint8_t             numSensors      = sensorDataProv.getNumSensors();
        uint8_t             sensorIdx       = 0U;

        for(sensorIdx = 0; sensorIdx < numSensors; ++sensorIdx)
        {
            ISensor*    sensor  = sensorDataProv.getSensor(sensorIdx);

            if (nullptr != sensor)
            {
                uint8_t     numChannels     = sensor->getNumChannels();
                uint8_t     channelIdx      = 0U;
                JsonObject  sensorObj       = sensorsArray.createNestedObject();

                sensorObj["index"]          = sensorIdx;
                sensorObj["name"]           = sensor->getName();
                sensorObj["isAvailable"]    = sensor->isAvailable();

                /* Block is only used, to have the channels in the correct JSON order. */
                {
                    JsonArray   channelsArray = sensorObj.createNestedArray("channels");

                    for(channelIdx = 0U; channelIdx < numChannels; ++channelIdx)
                    {
                        ISensorChannel* channel     = sensor->getChannel(channelIdx);
                        JsonObject      channelObj  = channelsArray.createNestedObject();

                        if (nullptr != channel)
                        {
                            channelObj["index"]  = channelIdx;
                            channelObj["name"]   = ISensorChannel::channelTypeToName(channel->getType());
                        }
                    }

                }
            }
        }

        /* Prepare response */
        jsonDoc["status"]   = "ok";
        httpStatusCode      = HttpStatus::STATUS_CODE_OK;
    }

    if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document has less memory available.");
    }
    else
    {
        LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
    }

    (void)serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

/**
 * List settings by keys.
 * GET \c "/api/v1/settings"
 *
 * @param[in] request   HTTP request
 */
static void handleSettings(AsyncWebServerRequest* request)
{
    String              content;
    const size_t        JSON_DOC_SIZE   = 1024U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET != request->method())
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = "error";
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        JsonObject  dataObj         = jsonDoc.createNestedObject("data");
        JsonArray   settingsArray   = dataObj.createNestedArray("settings");
        uint8_t     settingIdx      = 0U;
        KeyValue**  settings        = Settings::getInstance().getList();

        for(settingIdx = 0; settingIdx < Settings::KEY_VALUE_PAIR_NUM; ++settingIdx)
        {
            KeyValue*   setting = settings[settingIdx];

            if (nullptr != setting)
            {
                (void)settingsArray.add(setting->getKey());
            }
        }

        /* Prepare response */
        jsonDoc["status"]   = "ok";
        httpStatusCode      = HttpStatus::STATUS_CODE_OK;
    }

    if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document has less memory available.");
    }
    else
    {
        LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
    }

    (void)serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

/**
 * Get/Set single setting.
 * GET \c "/api/v1/setting"
 * POST \c "/api/v1/setting"
 *
 * @param[in] request   HTTP request
 */
static void handleSetting(AsyncWebServerRequest* request)
{
    String              content;
    const size_t        JSON_DOC_SIZE   = 1024U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET == request->method())
    {
        if (false == request->hasArg("key"))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = "error";
            errorObj["msg"]     = "Key is missing.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else if (false == Settings::getInstance().open(true))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            jsonDoc["status"]   = "error";
            errorObj["msg"]     = "Internal error.";
            httpStatusCode      = HttpStatus::STATUS_CODE_BAD_REQUEST;
        }
        else
        {
            JsonObject      dataObj = jsonDoc.createNestedObject("data");
            const String&   key     = request->arg("key");
            KeyValue*       setting = Settings::getInstance().getSettingByKey(key.c_str());

            dataObj["key"]  = setting->getKey();
            dataObj["name"] = setting->getName();

            switch(setting->getValueType())
            {
            case KeyValue::TYPE_STRING:
                {
                    KeyValueString* kvStr = static_cast<KeyValueString*>(setting);

                    dataObj["value"]        = kvStr->getValue();
                    dataObj["minlength"]    = kvStr->getMinLength();
                    dataObj["maxlength"]    = kvStr->getMaxLength();
                    dataObj["isSecret"]     = kvStr->isSecret();
                }
                break;
            
            case KeyValue::TYPE_BOOL:
                {
                    KeyValueBool* kvBool = static_cast<KeyValueBool*>(setting);

                    dataObj["value"]    = kvBool->getValue();
                }
                break;

            case KeyValue::TYPE_UINT8:
                {
                    KeyValueUInt8* kvUInt8 = static_cast<KeyValueUInt8*>(setting);

                    dataObj["value"]    = kvUInt8->getValue();
                    dataObj["min"]      = kvUInt8->getMin();
                    dataObj["max"]      = kvUInt8->getMax();
                }
                break;
            
            case KeyValue::TYPE_INT32:
                {
                    KeyValueInt32* kvInt32 = static_cast<KeyValueInt32*>(setting);

                    dataObj["value"]    = kvInt32->getValue();
                    dataObj["min"]      = kvInt32->getMin();
                    dataObj["max"]      = kvInt32->getMax();
                }
                break;
            
            case KeyValue::TYPE_JSON:
                {
                    KeyValueJson*           kvJson      = static_cast<KeyValueJson*>(setting);
                    JsonObject              valueObj    = dataObj.createNestedObject("value");
                    DynamicJsonDocument     jsonBuffer(JSON_DOC_SIZE);
                    DeserializationError    error       = deserializeJson(jsonBuffer, kvJson->getValue());

                    if (DeserializationError::Ok != error.code())
                    {
                        LOG_WARNING("JSON deserialization failed: %s", error.c_str());
                    }
                    else
                    {
                        /* Copy deserialized JSON object. */
                        valueObj.set(jsonBuffer.as<JsonObject>());
                    }

                    dataObj["minlength"]    = kvJson->getMinLength();
                    dataObj["maxlength"]    = kvJson->getMaxLength();
                }
                break;

            case KeyValue::TYPE_UINT32:
                {
                    KeyValueUInt32* kvUInt32 = static_cast<KeyValueUInt32*>(setting);

                    dataObj["value"]    = kvUInt32->getValue();
                    dataObj["min"]      = kvUInt32->getMin();
                    dataObj["max"]      = kvUInt32->getMax();
                }
                break;

            default:
                break;
            }

            Settings::getInstance().close();

            /* Prepare response */
            jsonDoc["status"]   = "ok";
            httpStatusCode      = HttpStatus::STATUS_CODE_OK;
        }
    }
    else if (HTTP_POST == request->method())
    {
        if (false == request->hasArg("key"))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = "error";
            errorObj["msg"]     = "Key is missing.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else if (false == request->hasArg("value"))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = "error";
            errorObj["msg"]     = "Value is missing.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else
        {
            const String&   key     = request->arg("key");
            KeyValue*       setting = Settings::getInstance().getSettingByKey(key.c_str());

            if (nullptr == setting)
            {
                JsonObject errorObj = jsonDoc.createNestedObject("error");

                /* Prepare response */
                jsonDoc["status"]   = "error";
                errorObj["msg"]     = "Key not found.";
                httpStatusCode      = HttpStatus::STATUS_CODE_BAD_REQUEST;
            }
            else if (false == Settings::getInstance().open(false))
            {
                JsonObject errorObj = jsonDoc.createNestedObject("error");

                jsonDoc["status"]   = "error";
                errorObj["msg"]     = "Internal error.";
                httpStatusCode      = HttpStatus::STATUS_CODE_BAD_REQUEST;
            }
            else
            {
                String error;

                if (false == storeSetting(setting, request->arg("value"), error))
                {
                    JsonObject errorObj = jsonDoc.createNestedObject("error");

                    LOG_WARNING(error);

                    jsonDoc["status"]   = "error";
                    errorObj["msg"]     = error;
                    httpStatusCode      = HttpStatus::STATUS_CODE_BAD_REQUEST;
                }
                else
                {
                    JsonObject dataObj = jsonDoc.createNestedObject("data");

                    UTIL_NOT_USED(dataObj);

                    jsonDoc["status"]   = "ok";
                    httpStatusCode      = HttpStatus::STATUS_CODE_OK;
                }

                Settings::getInstance().close();
            }
        }
    }
    else
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = "error";
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }

    if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document has less memory available.");
    }
    else
    {
        LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
    }

    (void)serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

/**
 * Store setting in persistent memory, considering the setting type.
 *
 * @param[in]   parameter   Key value pair
 * @param[in]   value       Value to write
 * @param[out]  error       If a error happended, it will contain the root cause.
 *
 * @return If successful stored, it will return true otherwise false.
 */
static bool storeSetting(KeyValue* parameter, const String& value, String& error)
{
    bool status = true;

    if (nullptr == parameter)
    {
        status = false;
        error   = "Internal error.";
    }
    else
    {
        switch(parameter->getValueType())
        {
        case KeyValue::TYPE_STRING:
            {
                KeyValueString* kvStr = static_cast<KeyValueString*>(parameter);

                /* If it is the hostname, verify it explicit. */
                if (0 == strcmp(Settings::getInstance().getHostname().getKey(), kvStr->getKey()))
                {
                    if (false == isValidHostname(value))
                    {
                        status  = false;
                        error   = "Invalid hostname.";
                    }
                }

                if (true == status)
                {
                    /* Check for min. and max. length */
                    if (kvStr->getMinLength() > value.length())
                    {
                        error  = "String length lower than ";
                        error += kvStr->getMinLength();
                        error += ".";

                        status = false;
                    }
                    else if (kvStr->getMaxLength() < value.length())
                    {
                        error  = "String length greater than ";
                        error += kvStr->getMaxLength();
                        error += ".";

                        status = false;
                    }
                    else
                    {
                        kvStr->setValue(value);
                    }
                }
            }
            break;

        case KeyValue::TYPE_BOOL:
            {
                KeyValueBool* kvBool = static_cast<KeyValueBool*>(parameter);

                if (0 == strcmp(value.c_str(), "false"))
                {
                    kvBool->setValue(false);
                }
                else if (0 == strcmp(value.c_str(), "true"))
                {
                    kvBool->setValue(true);
                }
                else
                {
                    status  = false;
                    error   = "Invalid value.";
                }
            }
            break;

        case KeyValue::TYPE_UINT8:
            {
                KeyValueUInt8*  kvUInt8     = static_cast<KeyValueUInt8*>(parameter);
                uint8_t         uint8Value  = 0;
                bool            convStatus  = Util::strToUInt8(value, uint8Value);

                /* Conversion failed? */
                if (false == convStatus)
                {
                    status  = false;
                    error   = "Invalid value.";
                }
                /* Check for min. and max. length */
                else if (kvUInt8->getMin() > uint8Value)
                {
                    error  = "Value lower than ";
                    error += kvUInt8->getMin();
                    error += ".";

                    status = false;
                }
                else if (kvUInt8->getMax() < uint8Value)
                {
                    error  = "Value greater than ";
                    error += kvUInt8->getMax();
                    error += ".";

                    status = false;
                }
                else
                {
                    kvUInt8->setValue(uint8Value);
                }
            }
            break;

        case KeyValue::TYPE_INT32:
            {
                KeyValueInt32*  kvInt32     = static_cast<KeyValueInt32*>(parameter);
                int32_t         int32Value  = 0;
                bool            convStatus  = Util::strToInt32(value, int32Value);

                /* Conversion failed? */
                if (false == convStatus)
                {
                    status  = false;
                    error   = "Invalid value.";
                }
                /* Check for min. and max. length */
                else if (kvInt32->getMin() > int32Value)
                {
                    error  = "Value lower than ";
                    error += kvInt32->getMin();
                    error += ".";

                    status = false;
                }
                else if (kvInt32->getMax() < int32Value)
                {
                    error  = "Value greater than ";
                    error += kvInt32->getMax();
                    error += ".";

                    status = false;
                }
                else
                {
                    kvInt32->setValue(int32Value);
                }
            }
            break;

        case KeyValue::TYPE_JSON:
            {
                KeyValueJson* kvJson = static_cast<KeyValueJson*>(parameter);

                /* Check for min. and max. length */
                if (kvJson->getMinLength() > value.length())
                {
                    error  = "JSON length lower than ";
                    error += kvJson->getMinLength();
                    error += ".";

                    status = false;
                }
                else if (kvJson->getMaxLength() < value.length())
                {
                    error  = "JSON length greater than ";
                    error += kvJson->getMaxLength();
                    error += ".";

                    status = false;
                }
                else
                {
                    kvJson->setValue(value);
                }
            }
            break;

        case KeyValue::TYPE_UINT32:
            {
                KeyValueUInt32* kvUInt32    = static_cast<KeyValueUInt32*>(parameter);
                uint32_t        uint32Value = 0U;
                bool            convStatus  = Util::strToUInt32(value, uint32Value);

                /* Conversion failed? */
                if (false == convStatus)
                {
                    status  = false;
                    error   = "Invalid value.";
                }
                /* Check for min. and max. length */
                else if (kvUInt32->getMin() > uint32Value)
                {
                    error  = "Value lower than ";
                    error += kvUInt32->getMin();
                    error += ".";

                    status = false;
                }
                else if (kvUInt32->getMax() < uint32Value)
                {
                    error  = "Value greater than ";
                    error += kvUInt32->getMax();
                    error += ".";

                    status = false;
                }
                else
                {
                    kvUInt32->setValue(uint32Value);
                }
            }
            break;

        case KeyValue::TYPE_UNKNOWN:
            /* fallthrough */
        default:
            status  = false;
            error   = "Unknown parameter.";
            break;
        }
    }

    return status;
}

/**
 * Get status information.
 * GET \c "/api/v1/status"
 *
 * @param[in] request   HTTP request
 */
static void handleStatus(AsyncWebServerRequest* request)
{
    String              content;
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;
    const size_t        JSON_DOC_SIZE   = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET != request->method())
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = "error";
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        String      ssid;
        int8_t      rssi            = -100; // dbm
        JsonObject  dataObj         = jsonDoc.createNestedObject("data");
        JsonObject  hwObj           = dataObj.createNestedObject("hardware");
        JsonObject  swObj           = dataObj.createNestedObject("software");
        JsonObject  internalRamObj  = swObj.createNestedObject("internalRam");
        JsonObject  wifiObj         = dataObj.createNestedObject("wifi");

        /* Only in station mode it makes sense to retrieve the RSSI.
         * Otherwise keep it -100 dbm.
         */
        if (WIFI_MODE_STA == WiFi.getMode())
        {
            rssi = WiFi.RSSI();
        }

        if (true == Settings::getInstance().open(true))
        {
            ssid = Settings::getInstance().getWifiSSID().getValue();
            Settings::getInstance().close();
        }

        /* Prepare response */
        jsonDoc["status"]       = "ok";

        hwObj["chipRev"]        = ESP.getChipRevision();
        hwObj["cpuFreqMhz"]     = ESP.getCpuFreqMHz();

        swObj["version"]        = Version::SOFTWARE_VER;
        swObj["revision"]       = Version::SOFTWARE_REV;
        swObj["espSdkVersion"]  = ESP.getSdkVersion();

        internalRamObj["heapSize"]      = ESP.getHeapSize();
        internalRamObj["availableHeap"] = ESP.getFreeHeap();

        wifiObj["ssid"]         = ssid;
        wifiObj["rssi"]         = rssi;                             // dBm
        wifiObj["quality"]      = WiFiUtil::getSignalQuality(rssi); // percent

        httpStatusCode          = HttpStatus::STATUS_CODE_OK;
    }

    if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document has less memory available.");
    }
    else
    {
        LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
    }

    (void)serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

/**
 * List files of given directory (?dir=<path>).
 * 
 * GET \c "/api/v1/fs"
 *
 * @param[in] request   HTTP request
 */
static void handleFilesystem(AsyncWebServerRequest* request)
{
    String              content;
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;
    const size_t        JSON_DOC_SIZE   = 2048U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET != request->method())
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = "error";
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        const String&   path                = request->arg("dir");
        const String&   pageStr             = request->arg("page");
        File            fdRoot              = FILESYSTEM.open(path, "r");
        JsonArray       jsonData            = jsonDoc.createNestedArray("data");
        const uint32_t  DEFAULT_MAX_FILES   = 15U;
        uint32_t        count               = DEFAULT_MAX_FILES;
        uint32_t        page                = 0U;
        uint32_t        preCount            = page * count;

        if (false == pageStr.isEmpty())
        {
            if (true == Util::strToUInt32(pageStr, page))
            {
                preCount = page * count;
            }
        }

        if (false == fdRoot)
        {
            LOG_WARNING("Invalid path.");
        }
        else
        {
            if (false == fdRoot.isDirectory())
            {
                LOG_WARNING("Requested path is not a directory.");
            }
            else
            {
                File fd = fdRoot.openNextFile();

                while((true == fd) && (0U < count))
                {
                    /* Page handling */
                    if (0U < preCount)
                    {
                        --preCount;
                    }
                    else
                    {
                        JsonObject jsonFile = jsonData.createNestedObject();

                        jsonFile["name"] = String(fd.name());
                        jsonFile["size"] = fd.size();

                        if (true == fd.isDirectory())
                        {
                            jsonFile["type"] = "dir";
                        }
                        else
                        {
                            jsonFile["type"] = "file";
                        }

                        --count;
                    }

                    fd.close();

                    if (0U < count)
                    {
                        fd = fdRoot.openNextFile();
                    }
                }
            }

            if (true == jsonDoc.overflowed())
            {
                LOG_WARNING("JSON document has less memory.");
            }

            fdRoot.close();
        }

        /* Prepare response */
        jsonDoc["status"]   = "ok";

        httpStatusCode      = HttpStatus::STATUS_CODE_OK;
    }

    if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document has less memory available.");
    }
    else
    {
        LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
    }

    (void)serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

/**
 * Read file from filesystem (?path=<path>).
 * 
 * GET \c "/api/v1/fs/file"
 *
 * @param[in] request   HTTP request
 */
static void handleFileGet(AsyncWebServerRequest* request)
{
    String              content;
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;
    const size_t        JSON_DOC_SIZE   = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET != request->method())
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = "error";
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;

        if (true == jsonDoc.overflowed())
        {
            LOG_ERROR("JSON document has less memory available.");
        }
        else
        {
            LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
        }

        (void)serializeJsonPretty(jsonDoc, content);
        request->send(httpStatusCode, "application/json", content);
    }
    else
    {
        const String& path = request->arg("path");

        LOG_INFO("File \"%s\" requested.", path.c_str());

        if (false == FILESYSTEM.exists(path))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = "error";
            errorObj["msg"]     = String("Invalid path ") + path;
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;

            if (true == jsonDoc.overflowed())
            {
                LOG_ERROR("JSON document has less memory available.");
            }
            else
            {
                LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
            }

            (void)serializeJsonPretty(jsonDoc, content);
            request->send(httpStatusCode, "application/json", content);
        }
        else
        {
            request->send(FILESYSTEM, path, getContentType(path));
        }
    }

    return;
}

/**
 * Get content type of file.
 * 
 * @param[in] filename  Name of file
 * 
 * @return The file specific content type.
 */
static String getContentType(const String& filename)
{
    String contentType = "text/plain";

    if (filename.endsWith(".html"))
    {
        contentType = "text/html";
    }
    else if (filename.endsWith(".css"))
    {
        contentType = "text/css";
    }
    else if (filename.endsWith(".js"))
    {
        contentType = "application/javascript";
    }
    else if (filename.endsWith(".png"))
    {
        contentType = "image/png";
    }
    else if (filename.endsWith(".gif"))
    {
        contentType = "image/gif";
    }
    else if (filename.endsWith(".jpg"))
    {
        contentType = "image/jpeg";
    }
    else if (filename.endsWith(".ico"))
    {
        contentType = "image/x-icon";
    }
    else if (filename.endsWith(".xml"))
    {
        contentType = "text/xml";
    }
    else if (filename.endsWith(".pdf"))
    {
        contentType = "application/x-pdf";
    }
    else if (filename.endsWith(".zip"))
    {
        contentType = "application/x-zip";
    }
    else if (filename.endsWith(".gz"))
    {
        contentType = "application/x-gzip";
    }

    return contentType;
}

/**
 * Write file to filesystem (?path=<path>).
 * 
 * POST \c "/api/v1/fs/file"
 *
 * @param[in] request   HTTP request
 */
static void handleFilePost(AsyncWebServerRequest* request)
{
    String              content;
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;
    const size_t        JSON_DOC_SIZE   = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_POST != request->method())
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = "error";
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        /* Prepare response */
        (void)jsonDoc.createNestedObject("data");
        jsonDoc["status"]   = "ok";
        httpStatusCode      = HttpStatus::STATUS_CODE_OK;
    }

    if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document has less memory available.");
    }
    else
    {
        LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
    }

    (void)serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

/**
 * File upload handler.
 *
 * @param[in] request   HTTP request.
 * @param[in] filename  Name of the uploaded file.
 * @param[in] index     Current file offset.
 * @param[in] data      Next data part of file, starting at offset.
 * @param[in] len       Data part size in byte.
 * @param[in] final     Is final packet or not.
 */
static void uploadHandler(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final)
{
    static File fd;
    bool        isError = false;

    /* Begin of upload? */
    if (0 == index)
    {
        fd = FILESYSTEM.open(filename, "w");

        if (false == fd)
        {
            isError = true;
        }
        else
        {
            LOG_INFO("Receiving file %s.", filename.c_str());
        }
    }

    if (true == fd)
    {
        (void)fd.write(data, len);
    }

    if ((true == final) &&
        (false == isError))
    {        
        LOG_INFO("File %s successful written.", filename.c_str());

        fd.close();
    }
    else if (true == isError)
    {
        LOG_INFO("File %s upload aborted.", filename.c_str());

        fd.close();
    }

    if (true == isError)
    {
        /* Inform client about abort.*/
        request->send(HttpStatus::STATUS_CODE_BAD_REQUEST, "text/plain", "Upload aborted.");
    }

    return;
}

/**
 * Delete file from filesystem (?path=<path>).
 * 
 * DELETE \c "/api/v1/fs/file"
 *
 * @param[in] request   HTTP request
 */
static void handleFileDelete(AsyncWebServerRequest* request)
{
    String              content;
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;
    const size_t        JSON_DOC_SIZE   = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_DELETE != request->method())
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = "error";
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        const String& path = request->arg("path");

        LOG_INFO("File \"%s\" removal requested.", path.c_str());

        if (false == FILESYSTEM.remove(path))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = "error";
            errorObj["msg"]     = "Failed to remove file.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else
        {
            /* Prepare response */
            (void)jsonDoc.createNestedObject("data");
            jsonDoc["status"]   = "ok";
            httpStatusCode      = HttpStatus::STATUS_CODE_OK;
        }
    }

    if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document has less memory available.");
    }
    else
    {
        LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
    }

    (void)serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

/**
 * Check the given hostname and returns whether it is valid or not.
 * Validation is according to RFC952.
 *
 * @param[in] hostname  Hostname which to validate
 *
 * @return Is valid (true) or not (false).
 */
static bool isValidHostname(const String& hostname)
{
    bool            isValid             = true;
    const size_t    MIN_HOSTNAME_LENGTH = Settings::getInstance().getHostname().getMinLength();
    const size_t    MAX_HOSTNAME_LENGTH = Settings::getInstance().getHostname().getMaxLength();

    if ((MIN_HOSTNAME_LENGTH > hostname.length()) ||
        (MAX_HOSTNAME_LENGTH < hostname.length()))
    {
        isValid = false;
    }
    else
    {
        uint8_t index = 0;

        while((true == isValid) && (index < hostname.length()))
        {
            if (('0' <= hostname[index]) &&
                ('9' >= hostname[index]))
            {
                /* No digit at the begin */
                if (0 == index)
                {
                    isValid = false;
                }
            }
            else if (('A' <= hostname[index]) &&
                     ('Z' >= hostname[index]))
            {
                /* Ok */
                ;
            }
            else if (('a' <= hostname[index]) &&
                     ('z' >= hostname[index]))
            {
                /* Ok */
                ;
            }
            else if ('-' == hostname[index])
            {
                /* No - at the begin */
                if (0 == index)
                {
                    isValid = false;
                }
            }
            else
            {
                isValid = false;
            }

            ++index;
        }
    }

    return isValid;
}

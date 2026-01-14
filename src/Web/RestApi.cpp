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
 * @file   RestApi.cpp
 * @brief  REST pages
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "RestApi.h"
#include "HttpStatus.h"
#include "DisplayMgr.h"
#include "Version.h"
#include "PluginMgr.h"
#include "PluginList.h"
#include "WiFiUtil.h"
#include "FileSystem.h"
#include "RestUtil.h"
#include "SlotList.h"
#include "RestartMgr.h"

#include <Util.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Esp.h>
#include <Logging.h>
#include <SensorDataProvider.h>
#include <SettingsService.h>
#include <FileUtil.h>
#include <MemUtil.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/** Content type element */
typedef struct
{
    const char* fileExtension; /**< File extension used to determine content type */
    const char* contentType;   /**< Content type */

} ContentTypeElem;

/**
 * Status of the Home Assistant MQTT automatic discovery feature.
 */
typedef enum
{
    HA_ENABLED = 0,   /**< Discovery is enabled. */
    HA_DISABLED,      /**< Discovery is disabled. This is also the case if the MqttService is not configured. */
    HA_STATUS_UNKNOWN /**< Status is unknown. */

} HomeAssistantDiscoveryStatus;

/**
 * Single REST API route.
 */
struct RestApiRoute
{
    const char*               page;               /**< Page in the filesystem. */
    WebRequestMethodComposite reqMethodComposite; /**< Request method composite */
    ArRequestHandlerFunction  onRequest;          /**< Request handler function. */
    ArUploadHandlerFunction   onUpload;           /**< Upload handler function. */
    ArBodyHandlerFunction     onBody;             /**< Body handler function. */
};

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static bool                         parseStringBool(const String& str, bool& outValue);
static void                         handleFadeEffect(AsyncWebServerRequest* request);
static void                         getSlotInfo(JsonObject& slot, uint16_t slotId);
static void                         handleSlots(AsyncWebServerRequest* request);
static void                         handleSlot(AsyncWebServerRequest* request);
static void                         handlePluginInstall(AsyncWebServerRequest* request);
static void                         handlePluginUninstall(AsyncWebServerRequest* request);
static void                         handlePlugins(AsyncWebServerRequest* request);
static void                         handleSensors(AsyncWebServerRequest* request);
static void                         handleSettings(AsyncWebServerRequest* request);
static void                         handleSetting(AsyncWebServerRequest* request);
static bool                         storeSetting(KeyValue* parameter, const String& value, String& error);
static void                         handleStatus(AsyncWebServerRequest* request);
static void                         getFiles(File& dir, JsonArray& files, uint32_t& preCount, uint32_t& count, bool isRecursive);
static void                         handleFilesystem(AsyncWebServerRequest* request);
static void                         handleFileGet(AsyncWebServerRequest* request);
static const char*                  getContentType(const String& filename);
static void                         handleFilePost(AsyncWebServerRequest* request);
static void                         uploadHandler(AsyncWebServerRequest* request, const String& filename, size_t index, uint8_t* data, size_t len, bool final);
static void                         handleFileDelete(AsyncWebServerRequest* request);
static bool                         isValidHostname(const String& hostname);
static void                         handlePartitionChange(AsyncWebServerRequest* request);
static HomeAssistantDiscoveryStatus disableHomeAssistantAutomaticDiscovery();
static void                         handleHomeAssistantAutomaticDiscoveryDisable(AsyncWebServerRequest* request);
static HomeAssistantDiscoveryStatus getHomeAssistantAutomaticDiscoveryStatus();
static void                         handleHomeAssistantAutomaticDiscoveryStatus(AsyncWebServerRequest* request);
static bool                         isPathSafe(const String& path);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Table of content types and the file extensions they will be derived from. */
static const ContentTypeElem contentTypeTable[] = {
    { ".html", "text/html" },
    { ".css", "text/css" },
    { ".js", "application/javascript" },
    { ".bmp", "image/bmp" },
    { ".png", "image/png" },
    { ".gif", "image/gif" },
    { ".jpg", "image/jpg" },
    { ".ico", "image/x-icon" },
    { ".gz", "application/x-gzip" }
};

/** REST API routes */
static const RestApiRoute gRestApiRoutes[] = {
    { "/display/fadeEffect", HTTP_GET | HTTP_POST, handleFadeEffect, nullptr, nullptr },
    { "/display/slots", HTTP_GET, handleSlots, nullptr, nullptr },
    { "/display/slot/*", HTTP_GET, handleSlot, nullptr, nullptr },
    { "/plugin/install", HTTP_POST, handlePluginInstall, nullptr, nullptr },
    { "/plugin/uninstall", HTTP_POST, handlePluginUninstall, nullptr, nullptr },
    { "/plugins", HTTP_GET, handlePlugins, nullptr, nullptr },
    { "/sensors", HTTP_GET, handleSensors, nullptr, nullptr },
    { "/settings", HTTP_GET, handleSettings, nullptr, nullptr },
    { "/setting", HTTP_GET | HTTP_POST, handleSetting, nullptr, nullptr },
    { "/status", HTTP_GET, handleStatus, nullptr, nullptr },
    { "/fs/file", HTTP_GET, handleFileGet, nullptr, nullptr },
    { "/fs/file", HTTP_POST, handleFilePost, uploadHandler, nullptr },
    { "/fs/file", HTTP_DELETE, handleFileDelete, nullptr, nullptr },
    { "/fs", HTTP_GET, handleFilesystem, nullptr, nullptr },
    { "/partitionChange", HTTP_POST, handlePartitionChange, nullptr, nullptr },
    { "/homeAssistant/automaticDiscovery/disable", HTTP_POST, handleHomeAssistantAutomaticDiscoveryDisable, nullptr, nullptr },
    { "/homeAssistant/automaticDiscovery/status", HTTP_GET, handleHomeAssistantAutomaticDiscoveryStatus, nullptr, nullptr }
};

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
    String           webLoginUser;
    String           webLoginPassword;
    SettingsService& settings = SettingsService::getInstance();

    if (false == settings.open(true))
    {
        webLoginUser     = settings.getWebLoginUser().getDefault();
        webLoginPassword = settings.getWebLoginPassword().getDefault();
    }
    else
    {
        webLoginUser     = settings.getWebLoginUser().getValue();
        webLoginPassword = settings.getWebLoginPassword().getValue();

        settings.close();
    }

    /* Register all REST API routes. */
    for (size_t idx = 0; idx < UTIL_ARRAY_NUM(gRestApiRoutes); ++idx)
    {
        const RestApiRoute& route      = gRestApiRoutes[idx];
        String              routePage  = BASE_URI;

        routePage                     += route.page;

        (void)srv.on(routePage.c_str(),
                     route.reqMethodComposite,
                     route.onRequest,
                     route.onUpload,
                     route.onBody)
            .setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());
    }
}

/**
 * Handle invalid rest path request.
 *
 * @param[in] request   HTTP request
 */
void RestApi::error(AsyncWebServerRequest* request)
{
    const size_t        JSON_DOC_SIZE = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint32_t            httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;

    if (nullptr == request)
    {
        return;
    }

    RestUtil::prepareRspError(jsonDoc, "Invalid path requested.");

    RestUtil::sendJsonRsp(request, jsonDoc, httpStatusCode);
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Parse a string to a boolean value.
 *
 * @param[in]  str       The string to parse ("true" or "false").
 * @param[out] outValue  The parsed boolean value.
 *
 * @return true if parsing was successful, false otherwise.
 */
static bool parseStringBool(const String& str, bool& outValue)
{
    bool isSuccessful = false;

    if (str == "true")
    {
        outValue     = true;
        isSuccessful = true;
    }
    else if (str == "false")
    {
        outValue     = false;
        isSuccessful = true;
    }
    else
    {
        /* Parsing failed. */
        ;
    }

    return isSuccessful;
}

/**
 * Activate next fade effect.
 * POST \c "/api/v1/display/fadeEffect"
 *
 * @param[in] request   HTTP request
 */
static void handleFadeEffect(AsyncWebServerRequest* request)
{
    uint32_t            httpStatusCode = HttpStatus::STATUS_CODE_OK;
    const size_t        JSON_DOC_SIZE  = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET == request->method())
    {
        JsonVariant dataObj   = RestUtil::prepareRspSuccess(jsonDoc);

        dataObj["fadeEffect"] = DisplayMgr::getInstance().getFadeEffect();
    }
    else if (HTTP_POST == request->method())
    {
        JsonVariant dataObj = RestUtil::prepareRspSuccess(jsonDoc);

        DisplayMgr::getInstance().activateNextFadeEffect(FadeEffectController::FADE_EFFECT_COUNT);

        dataObj["fadeEffect"] = DisplayMgr::getInstance().getFadeEffect();
    }
    else
    {
        RestUtil::prepareRspErrorHttpMethodNotSupported(jsonDoc);
        httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
    }

    RestUtil::sendJsonRsp(request, jsonDoc, httpStatusCode);
}

/**
 * Get slot info in JSON format.
 *
 * @param[out] slot     Slot information
 * @param[in]  slotId   Slot id
 */
static void getSlotInfo(JsonObject& slot, uint16_t slotId)
{
    DisplayMgr&         displayMgr = DisplayMgr::getInstance();
    uint8_t             stickySlot = displayMgr.getStickySlot();
    IPluginMaintenance* plugin     = displayMgr.getPluginInSlot(slotId);
    const char*         name       = (nullptr != plugin) ? plugin->getName() : "";
    uint16_t            uid        = (nullptr != plugin) ? plugin->getUID() : 0U;
    String              alias      = (nullptr != plugin) ? plugin->getAlias() : "";
    bool                isLocked   = displayMgr.isSlotLocked(slotId);
    uint32_t            duration   = displayMgr.getSlotDuration(slotId);
    bool                isDisabled = displayMgr.isSlotDisabled(slotId);

    slot["name"]                   = name;
    slot["uid"]                    = uid;
    slot["alias"]                  = alias;

    if (stickySlot != slotId)
    {
        slot["isSticky"] = false;
    }
    else
    {
        slot["isSticky"] = true;
    }

    slot["isLocked"]   = isLocked;
    slot["duration"]   = duration;
    slot["isDisabled"] = isDisabled;
}

/**
 * Get number of slots and which plugin is installed.
 * GET \c "/api/v1/display/slots"
 *
 * @param[in] request   HTTP request
 */
static void handleSlots(AsyncWebServerRequest* request)
{
    uint32_t            httpStatusCode = HttpStatus::STATUS_CODE_OK;
    const size_t        JSON_DOC_SIZE  = 4096U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET != request->method())
    {
        RestUtil::prepareRspErrorHttpMethodNotSupported(jsonDoc);
        httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        JsonVariant dataObj    = RestUtil::prepareRspSuccess(jsonDoc);
        JsonArray   slotArray  = dataObj.createNestedArray("slots");
        uint8_t     slotId     = 0U;
        DisplayMgr& displayMgr = DisplayMgr::getInstance();
        uint8_t     stickySlot = displayMgr.getStickySlot();

        /* Add max. number of slots */
        dataObj["maxSlots"]    = displayMgr.getMaxSlots();

        /* Add which plugin's are installed. */
        for (slotId = 0U; slotId < displayMgr.getMaxSlots(); ++slotId)
        {
            JsonObject slot = slotArray.createNestedObject();

            getSlotInfo(slot, slotId);
        }
    }

    RestUtil::sendJsonRsp(request, jsonDoc, httpStatusCode);
}

/**
 * Activate a specific slot or set a slot sticky or clear the sticky flag.
 * POST \c "/api/v1/display/slot/<id>"
 *
 * @param[in] request   HTTP request
 */
static void handleSlot(AsyncWebServerRequest* request)
{
    uint32_t            httpStatusCode = HttpStatus::STATUS_CODE_OK;
    const size_t        JSON_DOC_SIZE  = 1024U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (nullptr == request)
    {
        return;
    }

    if ((HTTP_POST != request->method()) &&
        (HTTP_GET != request->method()))
    {
        RestUtil::prepareRspErrorHttpMethodNotSupported(jsonDoc);
        httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        String uriWithSlotId  = RestApi::BASE_URI;

        uriWithSlotId        += "/display/slot/";

        if (false == request->url().startsWith(uriWithSlotId))
        {
            RestUtil::prepareRspError(jsonDoc, "Invalid slot id.");
            httpStatusCode = HttpStatus::STATUS_CODE_METHOD_NOT_ALLOWED;
        }
        else
        {
            uint8_t slotId       = SlotList::SLOT_ID_INVALID;
            size_t  baseUriLen   = uriWithSlotId.length();
            bool    slotIdStatus = Util::strToUInt8(request->url().substring(baseUriLen), slotId);

            if (false == slotIdStatus)
            {
                RestUtil::prepareRspError(jsonDoc, "Invalid slot id.");
                httpStatusCode = HttpStatus::STATUS_CODE_METHOD_NOT_ALLOWED;
            }
            /* GET request? */
            else if (HTTP_GET != request->method())
            {
                JsonObject dataObj = RestUtil::prepareRspSuccess(jsonDoc);

                getSlotInfo(dataObj, slotId);
            }
            /* POST request */
            else
            {
                DisplayMgr& displayMgr = DisplayMgr::getInstance();

                /* Activate a slot (no arguments)? */
                if ((false == request->hasArg("sticky")) &&
                    (false == request->hasArg("disable")))
                {
                    if (false == displayMgr.activateSlot(slotId))
                    {
                        RestUtil::prepareRspError(jsonDoc, "Request rejected.");
                        httpStatusCode = HttpStatus::STATUS_CODE_METHOD_NOT_ALLOWED;
                    }
                    else
                    {
                        JsonVariant dataObj = RestUtil::prepareRspSuccess(jsonDoc);

                        UTIL_NOT_USED(dataObj);
                    }
                }
                /* Arguments are available, check them. */
                else
                {
                    bool isSlotConfigDirty = false;

                    /* Handle sticky flag. */
                    if (true == request->hasArg("sticky"))
                    {
                        const String& stickyFlagStr = request->arg("sticky");
                        bool          stickyFlag    = false;

                        if (false == parseStringBool(stickyFlagStr, stickyFlag))
                        {
                            RestUtil::prepareRspError(jsonDoc, "Invalid sticky flag.");
                            httpStatusCode = HttpStatus::STATUS_CODE_METHOD_NOT_ALLOWED;
                        }

                        if (HttpStatus::STATUS_CODE_OK == httpStatusCode)
                        {
                            /* Remove sticky flag? */
                            if (false == stickyFlag)
                            {
                                if (slotId != displayMgr.getStickySlot())
                                {
                                    RestUtil::prepareRspError(jsonDoc, "Slot is not sticky.");
                                    httpStatusCode = HttpStatus::STATUS_CODE_METHOD_NOT_ALLOWED;
                                }
                                else
                                {
                                    displayMgr.clearSticky();
                                    isSlotConfigDirty = true;
                                }
                            }
                            else if (false == displayMgr.setSlotSticky(slotId))
                            {
                                RestUtil::prepareRspError(jsonDoc, "Slot is empty or disabled.");
                                httpStatusCode = HttpStatus::STATUS_CODE_METHOD_NOT_ALLOWED;
                            }
                            else
                            {
                                /* Sticky flag successful set. */
                                isSlotConfigDirty = true;
                            }
                        }
                    }

                    /* Handle disable flag. */
                    if ((HttpStatus::STATUS_CODE_OK == httpStatusCode) &&
                        (true == request->hasArg("disable")))
                    {
                        const String& disableFlagStr = request->arg("disable");
                        bool          disableFlag    = false;

                        if (false == parseStringBool(disableFlagStr, disableFlag))
                        {
                            RestUtil::prepareRspError(jsonDoc, "Invalid disable flag.");
                            httpStatusCode = HttpStatus::STATUS_CODE_METHOD_NOT_ALLOWED;
                        }

                        if (HttpStatus::STATUS_CODE_OK == httpStatusCode)
                        {
                            bool slotIsDisabled = displayMgr.isSlotDisabled(slotId);

                            if (slotIsDisabled != disableFlag)
                            {
                                /* Enable slot? */
                                if (false == disableFlag)
                                {
                                    displayMgr.enableSlot(slotId);
                                    isSlotConfigDirty = true;
                                }
                                /* Disable slot. */
                                else
                                {
                                    if (false == displayMgr.disableSlot(slotId))
                                    {
                                        RestUtil::prepareRspError(jsonDoc, "Slot is sticky.");
                                        httpStatusCode = HttpStatus::STATUS_CODE_METHOD_NOT_ALLOWED;
                                    }
                                    else
                                    {
                                        isSlotConfigDirty = true;
                                    }
                                }
                            }
                        }
                    }

                    if (HttpStatus::STATUS_CODE_OK == httpStatusCode)
                    {
                        JsonObject dataObj = RestUtil::prepareRspSuccess(jsonDoc);

                        getSlotInfo(dataObj, slotId);

                        if (true == isSlotConfigDirty)
                        {
                            /* Ensure that the changes will be available after power-up. */
                            PluginMgr::getInstance().save();
                        }
                    }
                }
            }
        }
    }

    RestUtil::sendJsonRsp(request, jsonDoc, httpStatusCode);
}

/**
 * Install plugin
 * POST \c "/api/v1/plugin/install?name=<plugin-name>"
 *
 * @param[in] request   HTTP request
 */
static void handlePluginInstall(AsyncWebServerRequest* request)
{
    const size_t        JSON_DOC_SIZE = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint32_t            httpStatusCode = HttpStatus::STATUS_CODE_OK;

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_POST != request->method())
    {
        RestUtil::prepareRspErrorHttpMethodNotSupported(jsonDoc);
        httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        /* Plugin name missing? */
        if (false == request->hasArg("name"))
        {
            RestUtil::prepareRspError(jsonDoc, "Plugin name is missing.");
            httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else
        {
            const String&       pluginName = request->arg("name");
            IPluginMaintenance* plugin     = PluginMgr::getInstance().install(pluginName.c_str());

            /* Plugin not found? */
            if (nullptr == plugin)
            {
                RestUtil::prepareRspError(jsonDoc, "Plugin unknown.");
                httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
            }
            /* Plugin successful installed. */
            else
            {
                JsonVariant dataObj = RestUtil::prepareRspSuccess(jsonDoc);

                plugin->enable();

                /* Save current installed plugins to persistent memory. */
                PluginMgr::getInstance().save();

                /* Prepare response */
                dataObj["slotId"] = DisplayMgr::getInstance().getSlotIdByPluginUID(plugin->getUID());
                dataObj["uid"]    = plugin->getUID();
            }
        }
    }

    RestUtil::sendJsonRsp(request, jsonDoc, httpStatusCode);
}

/**
 * Uninstall plugin
 * POST \c "/api/v1/plugin/uninstall?name=<plugin-name>&slotId=<slot-id>"
 *
 * @param[in] request   HTTP request
 */
static void handlePluginUninstall(AsyncWebServerRequest* request)
{
    const size_t        JSON_DOC_SIZE = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint32_t            httpStatusCode = HttpStatus::STATUS_CODE_OK;

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_POST != request->method())
    {
        RestUtil::prepareRspErrorHttpMethodNotSupported(jsonDoc);
        httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        /* Plugin name missing? */
        if (false == request->hasArg("name"))
        {
            RestUtil::prepareRspError(jsonDoc, "Plugin name is missing.");
            httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        /* Slot id missing? */
        else if (false == request->hasArg("slotId"))
        {
            RestUtil::prepareRspError(jsonDoc, "Slot id is missing.");
            httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else
        {
            uint8_t slotId       = SlotList::SLOT_ID_INVALID;
            bool    slotIdStatus = Util::strToUInt8(request->arg("slotId"), slotId);

            if (false == slotIdStatus)
            {
                RestUtil::prepareRspError(jsonDoc, "Invalid slot id.");
                httpStatusCode = HttpStatus::STATUS_CODE_METHOD_NOT_ALLOWED;
            }
            else
            {
                const String&       pluginName = request->arg("name");
                IPluginMaintenance* plugin     = DisplayMgr::getInstance().getPluginInSlot(slotId);

                if (nullptr == plugin)
                {
                    RestUtil::prepareRspError(jsonDoc, "No plugin in slot.");
                    httpStatusCode = HttpStatus::STATUS_CODE_METHOD_NOT_ALLOWED;
                }
                else if (0 != pluginName.compareTo(plugin->getName()))
                {
                    RestUtil::prepareRspError(jsonDoc, "Wrong plugin in slot.");
                    httpStatusCode = HttpStatus::STATUS_CODE_METHOD_NOT_ALLOWED;
                }
                else if (true == DisplayMgr::getInstance().isSlotLocked(slotId))
                {
                    RestUtil::prepareRspError(jsonDoc, "Slot is locked.");
                    httpStatusCode = HttpStatus::STATUS_CODE_METHOD_NOT_ALLOWED;
                }
                else if (false == PluginMgr::getInstance().uninstall(plugin))
                {
                    RestUtil::prepareRspError(jsonDoc, "Failed to uninstall.");
                    httpStatusCode = HttpStatus::STATUS_CODE_METHOD_NOT_ALLOWED;
                }
                else
                {
                    /* Save current installed plugins to persistent memory. */
                    PluginMgr::getInstance().save();

                    (void)RestUtil::prepareRspSuccess(jsonDoc);
                }
            }
        }
    }

    RestUtil::sendJsonRsp(request, jsonDoc, httpStatusCode);
}

/**
 * List all available plugins.
 * GET \c "/api/v1/plugins"
 *
 * @param[in] request   HTTP request
 */
static void handlePlugins(AsyncWebServerRequest* request)
{
    const size_t        JSON_DOC_SIZE = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint32_t            httpStatusCode = HttpStatus::STATUS_CODE_OK;

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET != request->method())
    {
        RestUtil::prepareRspErrorHttpMethodNotSupported(jsonDoc);
        httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        JsonVariant                dataObj              = RestUtil::prepareRspSuccess(jsonDoc);
        JsonArray                  pluginArray          = dataObj.createNestedArray("plugins");
        uint8_t                    pluginTypeListLength = 0U;
        const PluginList::Element* pluginTypeList       = PluginList::getList(pluginTypeListLength);
        uint8_t                    idx                  = 0U;

        while (pluginTypeListLength > idx)
        {
            if (false == pluginArray.add(pluginTypeList[idx].name))
            {
                break;
            }

            ++idx;
        }
    }

    RestUtil::sendJsonRsp(request, jsonDoc, httpStatusCode);
}

/**
 * List all sensors.
 * GET \c "/api/v1/sensors"
 *
 * @param[in] request   HTTP request
 */
static void handleSensors(AsyncWebServerRequest* request)
{
    const size_t        JSON_DOC_SIZE = 2048U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint32_t            httpStatusCode = HttpStatus::STATUS_CODE_OK;

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET != request->method())
    {
        RestUtil::prepareRspErrorHttpMethodNotSupported(jsonDoc);
        httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        JsonVariant         dataObj        = RestUtil::prepareRspSuccess(jsonDoc);
        JsonArray           sensorsArray   = dataObj.createNestedArray("sensors");
        SensorDataProvider& sensorDataProv = SensorDataProvider::getInstance();
        uint8_t             numSensors     = sensorDataProv.getNumSensors();
        uint8_t             sensorIdx      = 0U;

        for (sensorIdx = 0; sensorIdx < numSensors; ++sensorIdx)
        {
            ISensor* sensor = sensorDataProv.getSensor(sensorIdx);

            if (nullptr != sensor)
            {
                uint8_t    numChannels   = sensor->getNumChannels();
                JsonObject sensorObj     = sensorsArray.createNestedObject();

                sensorObj["index"]       = sensorIdx;
                sensorObj["name"]        = sensor->getName();
                sensorObj["isAvailable"] = sensor->isAvailable();

                /* Block is only used, to have the channels in the correct JSON order. */
                {
                    uint8_t   channelIdx    = 0U;
                    JsonArray channelsArray = sensorObj.createNestedArray("channels");

                    for (channelIdx = 0U; channelIdx < numChannels; ++channelIdx)
                    {
                        ISensorChannel* channel    = sensor->getChannel(channelIdx);
                        JsonObject      channelObj = channelsArray.createNestedObject();

                        if (nullptr != channel)
                        {
                            channelObj["index"] = channelIdx;
                            channelObj["name"]  = ISensorChannel::channelTypeToName(channel->getType());
                        }
                    }
                }
            }
        }
    }

    RestUtil::sendJsonRsp(request, jsonDoc, httpStatusCode);
}

/**
 * List settings by keys.
 * GET \c "/api/v1/settings"
 *
 * @param[in] request   HTTP request
 */
static void handleSettings(AsyncWebServerRequest* request)
{
    const size_t        JSON_DOC_SIZE = 1024U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint32_t            httpStatusCode = HttpStatus::STATUS_CODE_OK;

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET != request->method())
    {
        RestUtil::prepareRspErrorHttpMethodNotSupported(jsonDoc);
        httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        JsonVariant dataObj       = RestUtil::prepareRspSuccess(jsonDoc);
        JsonArray   settingsArray = dataObj.createNestedArray("settings");
        size_t      settingIdx    = 0U;
        size_t      settingsCount = 0U;
        KeyValue**  settings      = SettingsService::getInstance().getList(settingsCount);

        for (settingIdx = 0; settingIdx < settingsCount; ++settingIdx)
        {
            KeyValue* setting = settings[settingIdx];

            if (nullptr != setting)
            {
                if (false == settingsArray.add(setting->getKey()))
                {
                    break;
                }
            }
        }
    }

    RestUtil::sendJsonRsp(request, jsonDoc, httpStatusCode);
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
    const size_t        JSON_DOC_SIZE = 2048U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint32_t            httpStatusCode = HttpStatus::STATUS_CODE_OK;
    SettingsService&    settings       = SettingsService::getInstance();

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET == request->method())
    {
        if (false == request->hasArg("key"))
        {
            RestUtil::prepareRspError(jsonDoc, "Key is missing.");
            httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else if (false == settings.open(true))
        {
            RestUtil::prepareRspError(jsonDoc, "Internal error.");
            httpStatusCode = HttpStatus::STATUS_CODE_BAD_REQUEST;
        }
        else
        {
            JsonVariant   dataObj = RestUtil::prepareRspSuccess(jsonDoc);
            const String& key     = request->arg("key");
            KeyValue*     setting = settings.getSettingByKey(key.c_str());

            dataObj["key"]        = setting->getKey();
            dataObj["name"]       = setting->getName();

            switch (setting->getValueType())
            {
            case KeyValue::TYPE_STRING: {
                KeyValueString* kvStr = static_cast<KeyValueString*>(setting);

                dataObj["value"]      = kvStr->getValue();
                dataObj["minlength"]  = kvStr->getMinLength();
                dataObj["maxlength"]  = kvStr->getMaxLength();
                dataObj["isSecret"]   = kvStr->isSecret();
            }
            break;

            case KeyValue::TYPE_BOOL: {
                KeyValueBool* kvBool = static_cast<KeyValueBool*>(setting);

                dataObj["value"]     = kvBool->getValue();
            }
            break;

            case KeyValue::TYPE_UINT8: {
                KeyValueUInt8* kvUInt8 = static_cast<KeyValueUInt8*>(setting);

                dataObj["value"]       = kvUInt8->getValue();
                dataObj["min"]         = kvUInt8->getMin();
                dataObj["max"]         = kvUInt8->getMax();
            }
            break;

            case KeyValue::TYPE_INT32: {
                KeyValueInt32* kvInt32 = static_cast<KeyValueInt32*>(setting);

                dataObj["value"]       = kvInt32->getValue();
                dataObj["min"]         = kvInt32->getMin();
                dataObj["max"]         = kvInt32->getMax();
            }
            break;

            case KeyValue::TYPE_JSON: {
                KeyValueJson*        kvJson   = static_cast<KeyValueJson*>(setting);
                JsonObject           valueObj = dataObj.createNestedObject("value");
                DynamicJsonDocument  jsonBuffer(JSON_DOC_SIZE);
                DeserializationError error = deserializeJson(jsonBuffer, kvJson->getValue());

                if (DeserializationError::Ok != error.code())
                {
                    LOG_WARNING("JSON deserialization failed: %s", error.c_str());
                }
                else
                {
                    /* Copy deserialized JSON object. */
                    valueObj.set(jsonBuffer.as<JsonObject>());
                }

                dataObj["minlength"] = kvJson->getMinLength();
                dataObj["maxlength"] = kvJson->getMaxLength();
            }
            break;

            case KeyValue::TYPE_UINT32: {
                KeyValueUInt32* kvUInt32 = static_cast<KeyValueUInt32*>(setting);

                dataObj["value"]         = kvUInt32->getValue();
                dataObj["min"]           = kvUInt32->getMin();
                dataObj["max"]           = kvUInt32->getMax();
            }
            break;

            default:
                break;
            }

            settings.close();
        }
    }
    else if (HTTP_POST == request->method())
    {
        if (false == request->hasArg("key"))
        {
            RestUtil::prepareRspError(jsonDoc, "Key is missing.");
            httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else if (false == request->hasArg("value"))
        {
            RestUtil::prepareRspError(jsonDoc, "Value is missing.");
            httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else
        {
            const String& key     = request->arg("key");
            KeyValue*     setting = settings.getSettingByKey(key.c_str());

            if (nullptr == setting)
            {
                RestUtil::prepareRspError(jsonDoc, "Key not found.");
                httpStatusCode = HttpStatus::STATUS_CODE_BAD_REQUEST;
            }
            else if (false == settings.open(false))
            {
                RestUtil::prepareRspError(jsonDoc, "Internal error.");
                httpStatusCode = HttpStatus::STATUS_CODE_BAD_REQUEST;
            }
            else
            {
                String errorMsg;

                if (false == storeSetting(setting, request->arg("value"), errorMsg))
                {
                    RestUtil::prepareRspError(jsonDoc, errorMsg.c_str());
                    httpStatusCode = HttpStatus::STATUS_CODE_BAD_REQUEST;

                    LOG_WARNING(errorMsg);
                }
                else
                {
                    (void)RestUtil::prepareRspSuccess(jsonDoc);
                }

                settings.close();
            }
        }
    }
    else
    {
        RestUtil::prepareRspErrorHttpMethodNotSupported(jsonDoc);
        httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
    }

    RestUtil::sendJsonRsp(request, jsonDoc, httpStatusCode);
}

/**
 * Store setting in persistent memory, considering the setting type.
 *
 * @param[in]   parameter   Key value pair
 * @param[in]   value       Value to write
 * @param[out]  error       If a error happened, it will contain the root cause.
 *
 * @return If successful stored, it will return true otherwise false.
 */
static bool storeSetting(KeyValue* parameter, const String& value, String& error)
{
    bool status = true;

    if (nullptr == parameter)
    {
        status = false;
        error  = "Internal error.";
    }
    else
    {
        SettingsService& settings = SettingsService::getInstance();

        switch (parameter->getValueType())
        {
        case KeyValue::TYPE_STRING: {
            KeyValueString* kvStr = static_cast<KeyValueString*>(parameter);

            /* If it is the hostname, verify it explicit. */
            if (0 == strcmp(settings.getHostname().getKey(), kvStr->getKey()))
            {
                if (false == isValidHostname(value))
                {
                    status = false;
                    error  = "Invalid hostname.";
                }
            }

            if (true == status)
            {
                /* Check for min. and max. length */
                if (kvStr->getMinLength() > value.length())
                {
                    error   = "String length lower than ";
                    error  += kvStr->getMinLength();
                    error  += ".";

                    status  = false;
                }
                else if (kvStr->getMaxLength() < value.length())
                {
                    error   = "String length greater than ";
                    error  += kvStr->getMaxLength();
                    error  += ".";

                    status  = false;
                }
                else
                {
                    kvStr->setValue(value);
                }
            }
        }
        break;

        case KeyValue::TYPE_BOOL: {
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
                status = false;
                error  = "Invalid value.";
            }
        }
        break;

        case KeyValue::TYPE_UINT8: {
            KeyValueUInt8* kvUInt8    = static_cast<KeyValueUInt8*>(parameter);
            uint8_t        uint8Value = 0;
            bool           convStatus = Util::strToUInt8(value, uint8Value);

            /* Conversion failed? */
            if (false == convStatus)
            {
                status = false;
                error  = "Invalid value.";
            }
            /* Check for min. and max. length */
            else if (kvUInt8->getMin() > uint8Value)
            {
                error   = "Value lower than ";
                error  += kvUInt8->getMin();
                error  += ".";

                status  = false;
            }
            else if (kvUInt8->getMax() < uint8Value)
            {
                error   = "Value greater than ";
                error  += kvUInt8->getMax();
                error  += ".";

                status  = false;
            }
            else
            {
                kvUInt8->setValue(uint8Value);
            }
        }
        break;

        case KeyValue::TYPE_INT32: {
            KeyValueInt32* kvInt32    = static_cast<KeyValueInt32*>(parameter);
            int32_t        int32Value = 0;
            bool           convStatus = Util::strToInt32(value, int32Value);

            /* Conversion failed? */
            if (false == convStatus)
            {
                status = false;
                error  = "Invalid value.";
            }
            /* Check for min. and max. length */
            else if (kvInt32->getMin() > int32Value)
            {
                error   = "Value lower than ";
                error  += kvInt32->getMin();
                error  += ".";

                status  = false;
            }
            else if (kvInt32->getMax() < int32Value)
            {
                error   = "Value greater than ";
                error  += kvInt32->getMax();
                error  += ".";

                status  = false;
            }
            else
            {
                kvInt32->setValue(int32Value);
            }
        }
        break;

        case KeyValue::TYPE_JSON: {
            KeyValueJson* kvJson = static_cast<KeyValueJson*>(parameter);

            /* Check for min. and max. length */
            if (kvJson->getMinLength() > value.length())
            {
                error   = "JSON length lower than ";
                error  += kvJson->getMinLength();
                error  += ".";

                status  = false;
            }
            else if (kvJson->getMaxLength() < value.length())
            {
                error   = "JSON length greater than ";
                error  += kvJson->getMaxLength();
                error  += ".";

                status  = false;
            }
            else
            {
                kvJson->setValue(value);
            }
        }
        break;

        case KeyValue::TYPE_UINT32: {
            KeyValueUInt32* kvUInt32    = static_cast<KeyValueUInt32*>(parameter);
            uint32_t        uint32Value = 0U;
            bool            convStatus  = Util::strToUInt32(value, uint32Value);

            /* Conversion failed? */
            if (false == convStatus)
            {
                status = false;
                error  = "Invalid value.";
            }
            /* Check for min. and max. length */
            else if (kvUInt32->getMin() > uint32Value)
            {
                error   = "Value lower than ";
                error  += kvUInt32->getMin();
                error  += ".";

                status  = false;
            }
            else if (kvUInt32->getMax() < uint32Value)
            {
                error   = "Value greater than ";
                error  += kvUInt32->getMax();
                error  += ".";

                status  = false;
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
            status = false;
            error  = "Unknown parameter.";
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
    uint32_t            httpStatusCode = HttpStatus::STATUS_CODE_OK;
    const size_t        JSON_DOC_SIZE  = 1024U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET != request->method())
    {
        RestUtil::prepareRspErrorHttpMethodNotSupported(jsonDoc);
        httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        String           ssid;
        int8_t           rssi           = -100; /* [dbm] */
        JsonVariant      dataObj        = RestUtil::prepareRspSuccess(jsonDoc);
        JsonObject       hwObj          = dataObj.createNestedObject("hardware");
        JsonObject       swObj          = dataObj.createNestedObject("software");
        JsonObject       internalRamObj = swObj.createNestedObject("internalRam");
        JsonObject       wifiObj        = dataObj.createNestedObject("wifi");
        SettingsService& settings       = SettingsService::getInstance();

        /* Only in station mode it makes sense to retrieve the RSSI.
         * Otherwise keep it -100 dbm.
         */
        if (WIFI_MODE_STA == WiFi.getMode())
        {
            rssi = WiFi.RSSI();
        }

        if (true == settings.open(true))
        {
            ssid = settings.getWifiSSID().getValue();
            settings.close();
        }

        /* Prepare response */
        hwObj["chipRev"]                       = ESP.getChipRevision();
        hwObj["cpuFreqMhz"]                    = ESP.getCpuFreqMHz();

        swObj["version"]                       = Version::getSoftwareVersion();
        swObj["revision"]                      = Version::getSoftwareRevision();
        swObj["espSdkVersion"]                 = ESP.getSdkVersion();

        internalRamObj["heapSize"]             = MemUtil::getTotalHeapSize();        /* [byte] */
        internalRamObj["availableHeapSize"]    = MemUtil::getFreeHeapSize();         /* [byte] */
        internalRamObj["minFreeHeapSize"]      = MemUtil::getMinFreeHeapSize();      /* [byte] */
        internalRamObj["largestFreeBlockSize"] = MemUtil::getLargestFreeBlockSize(); /* [byte] */
        internalRamObj["isPsramAvailable"]     = MemUtil::isPsramAvailable();        /* [byte] */

        wifiObj["ssid"]                        = ssid;
        wifiObj["rssi"]                        = rssi;                             /* [dbm] */
        wifiObj["quality"]                     = WiFiUtil::getSignalQuality(rssi); /* [%] */
    }

    RestUtil::sendJsonRsp(request, jsonDoc, httpStatusCode);
}

/**
 * Get the files in the directory (optional: recursively) and fill the JSON array flat.
 * It will start to collect the files/directories after "preCount" were found.
 * It will stop to collect after "count" files.
 *
 * @param[in]       dir         The directory to start with.
 * @param[out]      files       JSON array for collecting the files.
 * @param[in,out]   preCount    This amount of files will be skipped.
 * @param[in,out]   count       Amount of files which max. to collect.
 * @param[in]       isRecursive If true, it will get all files recursive from root to all leafs.
 */
static void getFiles(File& dir, JsonArray& files, uint32_t& preCount, uint32_t& count, bool isRecursive)
{
    File fd = dir.openNextFile();

    while ((true == fd) && (0U < count))
    {
        /* Skip the first number of files. */
        if (0U < preCount)
        {
            /* One file/directory skipped. */
            --preCount;

            /* Dive into every directory recursively. */
            if ((true == isRecursive) &&
                (true == fd.isDirectory()))
            {
                getFiles(fd, files, preCount, count, isRecursive);
            }
        }
        else
        {
            JsonObject jsonFile = files.createNestedObject();

            /* One file/directory collected. */
            --count;

            jsonFile["name"] = String(fd.path());
            jsonFile["size"] = fd.size();

            if (true == fd.isDirectory())
            {
                jsonFile["type"] = "dir";

                if (true == isRecursive)
                {
                    /* Dive into every directory recursively. */
                    getFiles(fd, files, preCount, count, isRecursive);
                }
            }
            else
            {
                jsonFile["type"] = "file";
            }
        }

        fd.close();

        /* Still possible to collect more? */
        if (0U < count)
        {
            fd = dir.openNextFile();
        }
    }

    /* Cleanup */
    if (true == fd)
    {
        fd.close();
    }
}

/**
 * List files of given directory "?dir=<path>".
 *
 * GET \c "/api/v1/fs"
 *
 * @param[in] request   HTTP request
 */
static void handleFilesystem(AsyncWebServerRequest* request)
{
    String              content;
    uint32_t            httpStatusCode = HttpStatus::STATUS_CODE_OK;
    const size_t        JSON_DOC_SIZE  = 2048U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET != request->method())
    {
        RestUtil::prepareRspErrorHttpMethodNotSupported(jsonDoc);
        httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        const String&  path              = request->arg("dir");
        const String&  pageStr           = request->arg("page");
        File           fdRoot            = FILESYSTEM.open(path, "r");
        JsonArray      jsonData          = jsonDoc.createNestedArray("data");
        const uint32_t DEFAULT_MAX_FILES = 15U;
        uint32_t       count             = DEFAULT_MAX_FILES;
        uint32_t       page              = 0U;
        uint32_t       preCount          = page * count;

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
                LOG_INFO("List %s (page = %u)", path.c_str(), page);
                getFiles(fdRoot, jsonData, preCount, count, false);
            }

            fdRoot.close();
        }

        /* Prepare response */
        jsonDoc["status"] = "ok";
    }

    RestUtil::sendJsonRsp(request, jsonDoc, httpStatusCode);
}

/**
 * Read file from filesystem "?path=<path>".
 *
 * GET \c "/api/v1/fs/file"
 *
 * @param[in] request   HTTP request
 */
static void handleFileGet(AsyncWebServerRequest* request)
{
    const size_t        JSON_DOC_SIZE = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET != request->method())
    {
        RestUtil::prepareRspErrorHttpMethodNotSupported(jsonDoc);

        RestUtil::sendJsonRsp(request, jsonDoc, HttpStatus::STATUS_CODE_NOT_FOUND);
    }
    else
    {
        const String& path = request->arg("path");

        LOG_INFO("File \"%s\" requested.", path.c_str());

        if (false == FILESYSTEM.exists(path))
        {
            String errorMsg  = "Invalid path ";
            errorMsg        += path;

            RestUtil::prepareRspError(jsonDoc, errorMsg.c_str());

            RestUtil::sendJsonRsp(request, jsonDoc, HttpStatus::STATUS_CODE_NOT_FOUND);
        }
        else
        {
            request->send(FILESYSTEM, path, getContentType(path));
        }
    }
}

/**
 * Get content type of file.
 *
 * @param[in] filename  Name of file
 *
 * @return The file specific content type.
 */
static const char* getContentType(const String& filename)
{
    const char* contentType = "text/plain";
    uint8_t     idx         = 0U;

    while (UTIL_ARRAY_NUM(contentTypeTable) > idx)
    {
        if (true == filename.endsWith(contentTypeTable[idx].fileExtension))
        {
            contentType = contentTypeTable[idx].contentType;
            break;
        }

        ++idx;
    }

    return contentType;
}

/**
 * Write file to filesystem "?path=<path>".
 *
 * POST \c "/api/v1/fs/file"
 *
 * @param[in] request   HTTP request
 */
static void handleFilePost(AsyncWebServerRequest* request)
{
    uint32_t            httpStatusCode = HttpStatus::STATUS_CODE_OK;
    const size_t        JSON_DOC_SIZE  = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_POST != request->method())
    {
        RestUtil::prepareRspErrorHttpMethodNotSupported(jsonDoc);
        httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        (void)RestUtil::prepareRspSuccess(jsonDoc);
    }

    RestUtil::sendJsonRsp(request, jsonDoc, httpStatusCode);
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
static void uploadHandler(AsyncWebServerRequest* request, const String& filename, size_t index, uint8_t* data, size_t len, bool final)
{
    bool isError = false;

    /* Begin of upload? */
    if (0 == index)
    {
        /* Create directories if not exist. */
        if (false == FileUtil::createDirectories(filename))
        {
            isError = true;
        }
        else
        {
            request->_tempFile = FILESYSTEM.open(filename, "w");

            if (false == request->_tempFile)
            {
                isError = true;
            }
            else
            {
                LOG_INFO("Receiving file %s.", filename.c_str());
            }
        }
    }

    if (true == request->_tempFile)
    {
        (void)request->_tempFile.write(data, len);
    }

    if ((true == final) &&
        (false == isError))
    {
        LOG_INFO("File %s successful written.", filename.c_str());

        request->_tempFile.close();
    }
    else if (true == isError)
    {
        LOG_INFO("File %s upload aborted.", filename.c_str());

        request->_tempFile.close();
    }

    if (true == isError)
    {
        /* Inform client about abort.*/
        request->send(HttpStatus::STATUS_CODE_BAD_REQUEST, "text/plain", "Upload aborted.");
    }
}

/**
 * Delete file from filesystem "?path=<path>".
 * Delete directories is not supported.
 * A single wildcard '*' is supported at the start, middle or end of the filename, but not in the directory part.
 *
 * DELETE \c "/api/v1/fs/file"
 *
 * @param[in] request   HTTP request
 */
static void handleFileDelete(AsyncWebServerRequest* request)
{
    uint32_t            httpStatusCode = HttpStatus::STATUS_CODE_OK;
    const size_t        JSON_DOC_SIZE  = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_DELETE != request->method())
    {
        RestUtil::prepareRspErrorHttpMethodNotSupported(jsonDoc);
        httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        const String& path      = request->arg("path");
        int32_t       lastSlash = path.lastIndexOf('/');
        String        dir       = (0 <= lastSlash) ? path.substring(0U, static_cast<size_t>(lastSlash)) : "/";
        String        filename  = (0 <= lastSlash) ? path.substring(static_cast<size_t>(lastSlash + 1)) : path;

        /* Validate path to prevent directory traversal attacks.
         * Note: We allow wildcards in filename, but check directory part for traversal.
         */
        if (false == isPathSafe(dir))
        {
            RestUtil::prepareRspError(jsonDoc, "Invalid path: directory traversal not allowed.");
            httpStatusCode = HttpStatus::STATUS_CODE_BAD_REQUEST;

            RestUtil::sendJsonRsp(request, jsonDoc, httpStatusCode);
        }
        else
        {
            const char* WILDCARD    = "*";
            int32_t     wildcardPos = filename.indexOf('*');
            bool        anyRemoved  = false;

            LOG_INFO("File \"%s\" removal requested.", path.c_str());

            /* Wildcard used, but not allowed in directory part. */
            if (0 <= dir.indexOf(WILDCARD))
            {
                RestUtil::prepareRspError(jsonDoc, "Wildcard not allowed in directory part.");
                httpStatusCode = HttpStatus::STATUS_CODE_BAD_REQUEST;
            }
            else if (0 <= wildcardPos)
            {
                File dirFile = FILESYSTEM.open(dir, "r");

                if ((false == dirFile) || (false == dirFile.isDirectory()))
                {
                    RestUtil::prepareRspError(jsonDoc, "Invalid directory.");
                    httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
                }
                else
                {
                    File file = dirFile.openNextFile();

                    while (true == file)
                    {
                        String fname = String(file.name());

                        if (false == file.isDirectory())
                        {
                            bool   match    = false;
                            String fullPath = String(file.path());

                            /* Check if filename matches wildcard pattern. */
                            if (filename == WILDCARD)
                            {
                                match = true;
                            }
                            else
                            {
                                int32_t startPos   = filename.indexOf(WILDCARD);
                                String  prefix     = filename.substring(0U, static_cast<size_t>(startPos));
                                String  suffix     = filename.substring(static_cast<size_t>(startPos + 1));
                                int32_t fnameSlash = fname.lastIndexOf('/');
                                String  fnameOnly;

                                if (0 <= fnameSlash)
                                {
                                    fnameOnly = fname.substring(static_cast<size_t>(fnameSlash + 1));
                                }
                                else
                                {
                                    fnameOnly = fname;
                                }

                                if ((0U == prefix.length()) && (0U == suffix.length()))
                                {
                                    match = true;
                                }
                                else if (0U == prefix.length())
                                {
                                    match = fnameOnly.endsWith(suffix);
                                }
                                else if (0U == suffix.length())
                                {
                                    match = fnameOnly.startsWith(prefix);
                                }
                                else
                                {
                                    match = (fnameOnly.startsWith(prefix) && fnameOnly.endsWith(suffix));
                                }
                            }

                            file.close();

                            if (true == match)
                            {
                                LOG_DEBUG("Remove file \"%s\".", fullPath.c_str());

                                if (true == FILESYSTEM.remove(fullPath))
                                {
                                    anyRemoved = true;
                                }
                            }
                        }
                        else
                        {
                            file.close();
                        }

                        file = dirFile.openNextFile();
                    }
                    dirFile.close();

                    if (true == anyRemoved)
                    {
                        (void)RestUtil::prepareRspSuccess(jsonDoc);
                    }
                    else
                    {
                        RestUtil::prepareRspError(jsonDoc, "No matching files found to remove.");
                        httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
                    }
                }
            }
            else
            {
                if (false == FILESYSTEM.remove(path))
                {
                    RestUtil::prepareRspError(jsonDoc, "Failed to remove file.");
                    httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
                }
                else
                {
                    (void)RestUtil::prepareRspSuccess(jsonDoc);
                }
            }
        }
    }

    RestUtil::sendJsonRsp(request, jsonDoc, httpStatusCode);
}

/**
 * Check the given hostname and returns whether it is valid or not.
 * Validation is according to RFC952 (https://www.rfc-editor.org/rfc/rfc952.txt):
 * - Must start with a letter (A-Z, a-z)
 * - May contain letters, digits (0-9), and hyphens (-)
 * - Must not end with a hyphen
 * - Length must be within configured limits
 *
 * @param[in] hostname  Hostname which to validate
 *
 * @return Is valid (true) or not (false).
 */
static bool isValidHostname(const String& hostname)
{
    bool             isValid             = true;
    SettingsService& settings            = SettingsService::getInstance();
    const size_t     MIN_HOSTNAME_LENGTH = settings.getHostname().getMinLength();
    const size_t     MAX_HOSTNAME_LENGTH = settings.getHostname().getMaxLength();

    if ((MIN_HOSTNAME_LENGTH > hostname.length()) ||
        (MAX_HOSTNAME_LENGTH < hostname.length()))
    {
        isValid = false;
    }
    else
    {
        uint8_t index = 0U;

        while ((true == isValid) && (index < hostname.length()))
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
                /* No leading nor trailing hyphen. */
                if ((0U == index) ||
                    (hostname.length() - 1U == index))
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

/**
 * Set the factory partition active to be the next boot partition and notify the client whether it was successful or not.
 * If it was successful the board will be restarted.
 *
 * @param[in] request   HTTP request
 */
static void handlePartitionChange(AsyncWebServerRequest* request)
{
    uint32_t            httpStatusCode = HttpStatus::STATUS_CODE_OK;
    const size_t        JSON_DOC_SIZE  = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    const uint32_t      RESTART_DELAY = 100U; /* ms */

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_POST != request->method())
    {
        RestUtil::prepareRspErrorHttpMethodNotSupported(jsonDoc);
        httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        RestartMgr&                  restartMgr = RestartMgr::getInstance();
        RestartMgr::RestartReqStatus status     = restartMgr.reqRestart(RESTART_DELAY, true);

        switch (status)
        {
        case RestartMgr::RESTART_REQ_STATUS_OK:
            (void)RestUtil::prepareRspSuccess(jsonDoc);
            break;

        case RestartMgr::RESTART_REQ_STATUS_ERR:
            RestUtil::prepareRspError(jsonDoc, "Cannot switch to factory partition. Error unknown!");
            httpStatusCode = HttpStatus::STATUS_CODE_INTERNAL_SERVER_ERROR;
            break;


        case RestartMgr::RESTART_REQ_STATUS_FACTORY_PARTITION_NOT_FOUND:
            RestUtil::prepareRspError(jsonDoc, "Factory partition not found!");
            httpStatusCode = HttpStatus::STATUS_CODE_INTERNAL_SERVER_ERROR;
            break;

        case RestartMgr::RESTART_REQ_STATUS_FACTORY_SET_FAILED:
            RestUtil::prepareRspError(jsonDoc, "Failed to set factory partition as boot partition!");
            httpStatusCode = HttpStatus::STATUS_CODE_INTERNAL_SERVER_ERROR;
            break;


        default:
            break;
        }
    }

    RestUtil::sendJsonRsp(request, jsonDoc, httpStatusCode);
}

/**
 * Disable HomeAssistant MQTT automatic discovery to avoid that the welcome plugin will be discovered in case of a filesystem update.
 *
 * @return The status of the HomeAssistant MQTT automatic discovery after this operation.
 */
static HomeAssistantDiscoveryStatus disableHomeAssistantAutomaticDiscovery()
{

    HomeAssistantDiscoveryStatus status      = HA_STATUS_UNKNOWN;
    SettingsService&             settings    = SettingsService::getInstance();

    /* Key see HomeAssistantMqtt::KEY_HA_DISCOVERY_ENABLE
     * Include the header is not possible, because MQTT might not be compiled in.
     */
    KeyValue* kvHomeAssistantEnableDiscovery = settings.getSettingByKey("ha_ena");

    if ((nullptr != kvHomeAssistantEnableDiscovery))
    {
        if (KeyValue::TYPE_BOOL == kvHomeAssistantEnableDiscovery->getValueType())
        {
            if (true == settings.open(false))
            {
                KeyValueBool* homeAssistantEnableDiscovery = static_cast<KeyValueBool*>(kvHomeAssistantEnableDiscovery);

                homeAssistantEnableDiscovery->setValue(false);
                settings.close();

                status = HA_DISABLED;
                LOG_INFO("Home Assistant MQTT automatic discovery disabled for filesystem update.");
            }
        }
    }
    else
    {
        status = HA_DISABLED;
    }

    return status;
}

/**
 * Disable the Home Assistant MQTT automatic discovery and notifiy the client whether it was successful or not.
 *
 * @param[in] request   HTTP request
 */
static void handleHomeAssistantAutomaticDiscoveryDisable(AsyncWebServerRequest* request)
{
    uint32_t            httpStatusCode = HttpStatus::STATUS_CODE_OK;
    const size_t        JSON_DOC_SIZE  = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_POST != request->method())
    {
        RestUtil::prepareRspErrorHttpMethodNotSupported(jsonDoc);
        httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        switch (disableHomeAssistantAutomaticDiscovery())
        {
        case HA_DISABLED:
            (void)RestUtil::prepareRspSuccess(jsonDoc);
            break;

        case HA_ENABLED:
            RestUtil::prepareRspError(jsonDoc, "Home Assistant MQTT automatic discovery could not be disabled.");
            httpStatusCode = HttpStatus::STATUS_CODE_INTERNAL_SERVER_ERROR;
            break;

        case HA_STATUS_UNKNOWN:
            RestUtil::prepareRspError(jsonDoc, "Could not access setting. Status of Home Assistant MQTT automatic discovery is unnkown.");
            httpStatusCode = HttpStatus::STATUS_CODE_INTERNAL_SERVER_ERROR;

        default:
            break;
        }
    }

    RestUtil::sendJsonRsp(request, jsonDoc, httpStatusCode);
}

/**
 * Get the status of the Home Assistant MQTT automatic discovery.
 *
 * @return The status of the Home Assistant MQTT automatic discovery.
 */
static HomeAssistantDiscoveryStatus getHomeAssistantAutomaticDiscoveryStatus()
{
    HomeAssistantDiscoveryStatus status      = HA_STATUS_UNKNOWN;
    SettingsService&             settings    = SettingsService::getInstance();

    /* Key see HomeAssistantMqtt::KEY_HA_DISCOVERY_ENABLE
     * Include the header is not possible, because MQTT might not be compiled in.
     */
    KeyValue* kvHomeAssistantEnableDiscovery = settings.getSettingByKey("ha_ena");

    if ((nullptr != kvHomeAssistantEnableDiscovery))
    {
        if (KeyValue::TYPE_BOOL == kvHomeAssistantEnableDiscovery->getValueType())
        {
            if (true == settings.open(true))
            {
                KeyValueBool* homeAssistantEnableDiscovery = static_cast<KeyValueBool*>(kvHomeAssistantEnableDiscovery);

                if (true == homeAssistantEnableDiscovery->getValue())
                {
                    status = HA_ENABLED;
                }
                else
                {
                    status = HA_DISABLED;
                }

                settings.close();
            }
        }
    }
    else
    {
        status = HA_DISABLED;
    }

    return status;
}

/**
 * Check whether the Home Assistant MQTT automatic discovery is enabled or not and notify the client of the result.
 *
 * @param[in] request   HTTP request
 */
static void handleHomeAssistantAutomaticDiscoveryStatus(AsyncWebServerRequest* request)
{
    uint32_t            httpStatusCode = HttpStatus::STATUS_CODE_OK;
    const size_t        JSON_DOC_SIZE  = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET != request->method())
    {
        RestUtil::prepareRspErrorHttpMethodNotSupported(jsonDoc);
        httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        switch (getHomeAssistantAutomaticDiscoveryStatus())
        {
        case HA_ENABLED: {
            JsonObject data = RestUtil::prepareRspSuccess(jsonDoc);
            data["status"]  = "enabled";
            break;
        }

        case HA_DISABLED: {
            JsonObject data = RestUtil::prepareRspSuccess(jsonDoc);
            data["status"]  = "disabled";
            break;
        }

        case HA_STATUS_UNKNOWN:
            RestUtil::prepareRspError(jsonDoc, "Could not access setting. Status of Home Assistant MQTT automatic discovery is unknown.");
            httpStatusCode = HttpStatus::STATUS_CODE_INTERNAL_SERVER_ERROR;
            break;

        default:
            break;
        }
    }

    RestUtil::sendJsonRsp(request, jsonDoc, httpStatusCode);
}

/**
 * Validate filesystem path to prevent directory traversal attacks.
 *
 * @param[in] path  Path to validate
 *
 * @return true if path is safe, false if path contains traversal sequences.
 */
static bool isPathSafe(const String& path)
{
    bool isSafe = true;

    /* Reject paths containing directory traversal sequences */
    if (0 <= path.indexOf(".."))
    {
        isSafe = false;
    }
    /* Ensure path starts with / for absolute paths within filesystem */
    else if ((false == path.isEmpty()) && ('/' != path.charAt(0)))
    {
        isSafe = false;
    }
    else
    {
        /* Path is safe. */
        ;
    }

    return isSafe;
}

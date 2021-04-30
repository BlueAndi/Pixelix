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
#include "LedMatrix.h"
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

static void handleStatus(AsyncWebServerRequest* request);
static void handleSlots(AsyncWebServerRequest* request);
static void handlePlugin(AsyncWebServerRequest* request);
static void handleButton(AsyncWebServerRequest* request);
static void handleFilesystem(AsyncWebServerRequest* request);
static void handleFileGet(AsyncWebServerRequest* request);
static String getContentType(const String& filename);
static void handleFilePost(AsyncWebServerRequest* request);
static void uploadHandler(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);
static void handleFileDelete(AsyncWebServerRequest* request);

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
    (void)srv.on("/rest/api/v1/status", handleStatus);
    (void)srv.on("/rest/api/v1/display/slots", handleSlots);
    (void)srv.on("/rest/api/v1/plugin", handlePlugin);
    (void)srv.on("/rest/api/v1/button", handleButton);
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
    jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
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
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
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
        jsonDoc["status"]       = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);

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
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
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
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);
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
 * Install/Uninstall plugins
 * List plugins:     GET \c "/api/v1/plugin?list"
 * Install plugin:   POST \c "/api/v1/plugin?install=<plugin-name>"
 * Uninstall plugin: POST \c "/api/v1/plugin?uninstall=<plugin-name>&slotId=<slot-id>"
 *
 * @param[in] request   HTTP request
 */
static void handlePlugin(AsyncWebServerRequest* request)
{
    String              content;
    const size_t        JSON_DOC_SIZE   = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;

    if (nullptr == request)
    {
        return;
    }

    if (HTTP_GET == request->method())
    {
        /* List all plugins? */
        if (true == request->hasArg("list"))
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
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);
            httpStatusCode      = HttpStatus::STATUS_CODE_OK;
        }
        else
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Unknown argument.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
    }
    else if (HTTP_POST == request->method())
    {
        /* Plugin installation? */
        if (true == request->hasArg("install"))
        {
            String              pluginName  = request->arg("install");
            IPluginMaintenance* plugin      = PluginMgr::getInstance().install(pluginName);

            /* Plugin not found? */
            if (nullptr == plugin)
            {
                JsonObject errorObj = jsonDoc.createNestedObject("error");

                /* Prepare response */
                jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
                errorObj["msg"]     = "Plugin unknown.";
                httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
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
                jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);
                httpStatusCode      = HttpStatus::STATUS_CODE_OK;
            }
        }
        /* Plugin uninstallation? */
        else if (true == request->hasArg("uninstall"))
        {
            /* "slotId" argument missing? */
            if (false == request->hasArg("slotId"))
            {
                JsonObject errorObj = jsonDoc.createNestedObject("error");

                /* Prepare response */
                jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
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
                    jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
                    errorObj["msg"]     = "Invalid slot id.";
                    httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
                }
                else
                {
                    String              pluginName  = request->arg("uninstall");
                    IPluginMaintenance* plugin      = DisplayMgr::getInstance().getPluginInSlot(slotId);

                    if (nullptr == plugin)
                    {
                        JsonObject errorObj = jsonDoc.createNestedObject("error");

                        /* Prepare response */
                        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
                        errorObj["msg"]     = "No plugin in slot.";
                        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
                    }
                    else if (0 != pluginName.compareTo(plugin->getName()))
                    {
                        JsonObject errorObj = jsonDoc.createNestedObject("error");

                        /* Prepare response */
                        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
                        errorObj["msg"]     = "Wrong plugin in slot.";
                        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
                    }
                    else if (true == DisplayMgr::getInstance().isSlotLocked(slotId))
                    {
                        JsonObject errorObj = jsonDoc.createNestedObject("error");

                        /* Prepare response */
                        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
                        errorObj["msg"]     = "Slot is locked.";
                        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
                    }
                    else if (false == PluginMgr::getInstance().uninstall(plugin))
                    {
                        JsonObject errorObj = jsonDoc.createNestedObject("error");

                        /* Prepare response */
                        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
                        errorObj["msg"]     = "Failed to uninstall.";
                        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
                    }
                    else
                    {
                        /* Save current installed plugins to persistent memory. */
                        PluginMgr::getInstance().save();

                        /* Prepare response */
                        (void)jsonDoc.createNestedObject("data");
                        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);
                        httpStatusCode      = HttpStatus::STATUS_CODE_OK;
                    }
                }
            }
        }
        /* Unknown command */
        else
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Unknown argument.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
    }
    else
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
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
 * Trigger virtual user button.
 * Activate next slot:              GET \c "/api/v1/button"
 * Switch to the next FadeEffect:   POST \c "/api/v1/button?fadeEffect=<FadeEffectId>"
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

    if (HTTP_POST == request->method())
    {
        JsonObject  dataObj = jsonDoc.createNestedObject("data");
        
        /* Fade effect? */
        if (false == request->hasArg("fadeEffect"))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "fadeEffect is missing.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else
        {
            String effect = request->arg("fadeEffect");

            DisplayMgr::getInstance().activateNextFadeEffect(static_cast<DisplayMgr::FadeEffect>(effect.toInt()));

            /* Prepare response */
            dataObj["fadeEffect"]   = effect.toInt();
            jsonDoc["status"]       = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);
            httpStatusCode          = HttpStatus::STATUS_CODE_OK;
        }
    }
    else if (HTTP_GET == request->method())
    {
        JsonObject  dataObj = jsonDoc.createNestedObject("data");

        UTIL_NOT_USED(dataObj);
        DisplayMgr::getInstance().activateNextSlot();
        DisplayMgr::FadeEffect currentFadeEffect = DisplayMgr::getInstance().getFadeEffect();

        /* Prepare response */
        jsonDoc["status"]       = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);
        dataObj["fadeEffect"]   = currentFadeEffect;
        httpStatusCode          = HttpStatus::STATUS_CODE_OK;
    }
    else
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
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
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
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
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);

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
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
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
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
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
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        JsonObject dataObj = jsonDoc.createNestedObject("data");

        UTIL_NOT_USED(dataObj);

        /* Prepare response */
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);

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
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
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
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Failed to remove file.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else
        {
            JsonObject dataObj = jsonDoc.createNestedObject("data");

            UTIL_NOT_USED(dataObj);

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);

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

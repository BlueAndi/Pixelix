/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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

#include <Util.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Esp.h>

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

static uint8_t getSignalQuality(int8_t rssi);
static void handleStatus(AsyncWebServerRequest* request);
static void handleSlots(AsyncWebServerRequest* request);
static void handlePlugin(AsyncWebServerRequest* request);

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
    
    return;
}

/**
 * Handle invalid rest path request.
 * 
 * @param[in] request   HTTP request
 */
void RestApi::error(AsyncWebServerRequest* request)
{
    String                  content;
    StaticJsonDocument<256> jsonDoc;
    uint32_t                httpStatusCode  = HttpStatus::STATUS_CODE_OK;
    JsonObject              errorObj        = jsonDoc.createNestedObject("error");

    if (nullptr == request)
    {
        return;
    }

    /* Prepare response */
    jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
    errorObj["msg"]     = "Invalid path requested.";
    httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    
    serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Get the wifi signal quality, derrived from the RSSI.
 * 
 * @param[in] rssi  RSSI in dBm
 * 
 * @return Signal quality in percent
 */
static uint8_t getSignalQuality(int8_t rssi)
{
    uint8_t         signalQuality   = 0U;
    const int8_t    RSSI_HIGH       = -50;  // dBm
    const int8_t    RSSI_UNUSABLE   = -100; // dBm

    if (RSSI_HIGH <= rssi)
    {
        signalQuality = 100U;
    }
    else if (RSSI_UNUSABLE >= rssi)
    {
        signalQuality = 0U;
    }
    else
    {
        signalQuality = static_cast<uint8_t>(2 * (rssi + 100));
    }

    return signalQuality;
}

/**
 * Get status information.
 * GET \c "/api/v1/status"
 * 
 * @param[in] request   HTTP request
 */
static void handleStatus(AsyncWebServerRequest* request)
{
    String                  content;
    StaticJsonDocument<256> jsonDoc;
    uint32_t                httpStatusCode  = HttpStatus::STATUS_CODE_OK;

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
        int8_t      rssi            = WiFi.RSSI();
        JsonObject  dataObj         = jsonDoc.createNestedObject("data");
        JsonObject  hwObj           = dataObj.createNestedObject("hardware");
        JsonObject  swObj           = dataObj.createNestedObject("software");
        JsonObject  internalRamObj  = swObj.createNestedObject("internalRam");
        JsonObject  wifiObj         = dataObj.createNestedObject("wifi");

        if (true == Settings::getInstance().open(true))
        {
            ssid = Settings::getInstance().getWifiSSID();
            Settings::getInstance().close();
        }

        /* Prepare response */
        jsonDoc["status"]       = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);

        hwObj["chipRev"]        = ESP.getChipRevision();
        hwObj["cpuFreqMhz"]     = ESP.getCpuFreqMHz();

        swObj["version"]        = Version::SOFTWARE;
        swObj["espSdkVersion"]  = ESP.getSdkVersion();

        internalRamObj["heapSize"]      = ESP.getHeapSize();
        internalRamObj["availableHeap"] = ESP.getFreeHeap();

        wifiObj["ssid"]         = ssid;
        wifiObj["rssi"]         = rssi;                     // dBm
        wifiObj["quality"]      = getSignalQuality(rssi);   // percent

        httpStatusCode          = HttpStatus::STATUS_CODE_OK;
    }

    serializeJsonPretty(jsonDoc, content);
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
    String                  content;
    StaticJsonDocument<200> jsonDoc;
    uint32_t                httpStatusCode  = HttpStatus::STATUS_CODE_OK;

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
        JsonArray   slotArray   = dataObj.createNestedArray("slot");
        uint8_t     index       = 0U;

        /* Add max. number of slots */
        dataObj["slots"] = DisplayMgr::getInstance().MAX_SLOTS;

        /* Add which plugin's are installed. */
        for(index = 0U; index < DisplayMgr::MAX_SLOTS; ++index)
        {
            Plugin* plugin = DisplayMgr::getInstance().getPluginInSlot(index);

            if (nullptr == plugin)
            {
                slotArray.add("empty");
            }
            else
            {
                slotArray.add(plugin->getName());
            }
        }

        /* Prepare response */
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);
        httpStatusCode      = HttpStatus::STATUS_CODE_OK;
    }

    serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

/**
 * Install/Uninstall plugins
 * Install plugin:   POST \c "/api/v1/plugin?install=<plugin-name>"
 * Uninstall plugin: POST \c "/api/v1/plugin?uninstall=<plugin-name>&slotId=<slot-id>"
 * 
 * @param[in] request   HTTP request
 */
static void handlePlugin(AsyncWebServerRequest* request)
{
    String                  content;
    StaticJsonDocument<200> jsonDoc;
    uint32_t                httpStatusCode  = HttpStatus::STATUS_CODE_OK;

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
    /* Plugin installation? */
    else if (true == request->hasArg("install"))
    {
        String  pluginName  = request->arg("install");
        Plugin* plugin      = nullptr;

        if (0 == pluginName.compareTo("JustTextPlugin"))
        {
            plugin = PluginMgr::getInstance().installJustTextPlugin();
        }
        else if (0 == pluginName.compareTo("IconTextPlugin"))
        {
            plugin = PluginMgr::getInstance().installIconTextPlugin();
        }
        else if (0 == pluginName.compareTo("IconTextLampPlugin"))
        {
            plugin = PluginMgr::getInstance().installIconTextLampPlugin();
        }
        else if (0 == pluginName.compareTo("FirePlugin"))
        {
            plugin = PluginMgr::getInstance().installFirePlugin();
        }
        else if (0 == pluginName.compareTo("GameOfLifePlugin"))
        {
            plugin = PluginMgr::getInstance().installGameOfLifePlugin();
        }
        else
        {
            ;
        }

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

            /* Prepare response */
            dataObj["slotId"]   = plugin->getSlotId();
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
                String  pluginName  = request->arg("uninstall");
                Plugin* plugin      = DisplayMgr::getInstance().getPluginInSlot(slotId);

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
                else
                {
                    PluginMgr::getInstance().uninstall(plugin);

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
        errorObj["msg"]     = "Install or uninstall?";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }

    serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
static void status(AsyncWebServerRequest* request);
static void slots(AsyncWebServerRequest* request);

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
    (void)srv.on("/rest/api/v1/status", status);
    (void)srv.on("/rest/api/v1/display/slots", slots);
    
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

    if (NULL == request)
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
    uint8_t         signalQuality   = 0u;
    const int8_t    RSSI_HIGH       = -50;  // dBm
    const int8_t    RSSI_UNUSABLE   = -100; // dBm

    if (RSSI_HIGH <= rssi)
    {
        signalQuality = 100u;
    }
    else if (RSSI_UNUSABLE >= rssi)
    {
        signalQuality = 0u;
    }
    else
    {
        signalQuality = static_cast<uint8_t>(2 * (rssi + 100));
    }

    return signalQuality;
}

/**
 * Get status information.
 * GET /api/v1/status
 * 
 * @param[in] request   HTTP request
 */
static void status(AsyncWebServerRequest* request)
{
    String                  content;
    StaticJsonDocument<256> jsonDoc;
    uint32_t                httpStatusCode  = HttpStatus::STATUS_CODE_OK;

    if (NULL == request)
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
 * Get number of slots
 * GET /api/v1/display/slots
 * 
 * @param[in] request   HTTP request
 */
static void slots(AsyncWebServerRequest* request)
{
    String                  content;
    StaticJsonDocument<200> jsonDoc;
    uint32_t                httpStatusCode  = HttpStatus::STATUS_CODE_OK;

    if (NULL == request)
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
        JsonObject  dataObj = jsonDoc.createNestedObject("data");

        dataObj["slots"] = DisplayMgr::getInstance().MAX_SLOTS;

        /* Prepare response */
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);
        httpStatusCode      = HttpStatus::STATUS_CODE_OK;
    }

    serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

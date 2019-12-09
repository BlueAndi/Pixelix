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

#include <WiFi.h>
#include <ArduinoJson.h>
#include <Esp.h>

extern "C"
{
#include <crypto/base64.h>
}

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/**
 * REST request status code.
 */
typedef enum
{
    STATUS_CODE_OK = 0,     /**< Successful */
    STATUS_CODE_NOT_FOUND   /**< Requested URI not found. */

} StatusCode;

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static bool toUInt8(const String& str, uint8_t& value);
static bool toUInt16(const String& str, uint16_t& value);
static uint8_t getSignalQuality(int8_t rssi);
static void error(AsyncWebServerRequest* request);
static void status(AsyncWebServerRequest* request);
static void slots(AsyncWebServerRequest* request);
static void slotText(AsyncWebServerRequest* request);
static void slotBitmap(AsyncWebServerRequest* request);
static void slotLamp(AsyncWebServerRequest* request);

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
    srv.on("/rest/api/v1/status", status);
    srv.on("/rest/api/v1/display/slots", slots);
    srv.on("^\\/rest\\/api\\/v1\\/display\\/slot\\/([0-9]+)\\/text$", slotText);
    srv.on("^\\/rest\\/api\\/v1\\/display\\/slot\\/([0-9]+)\\/bitmap$", slotBitmap);
    srv.on("^\\/rest\\/api\\/v1\\/display\\/slot\\/([0-9]+)\\/lamp\\/([0-9]+)\\/state$", slotLamp);
    
    /* Register a page for invalid REST path requests. */
    srv.on("/rest/*", error);

    return;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Convert a string to uint8_t.
 * 
 * @param[in]   str     String
 * @param[out]  value   Converted value
 * 
 * @return If conversion fails, it will return false otherwise true.
 */
static bool toUInt8(const String& str, uint8_t& value)
{
    bool    success = false;
    int32_t tmp     = str.toInt();

    if ((0 <= tmp) ||
        (UINT8_MAX >= tmp))
    {
        value = static_cast<uint8_t>(tmp);
        success = true;
    }

    return success;
}

/**
 * Convert a string to uint16_t.
 * 
 * @param[in]   str     String
 * @param[out]  value   Converted value
 * 
 * @return If conversion fails, it will return false otherwise true.
 */
static bool toUInt16(const String& str, uint16_t& value)
{
    bool    success = false;
    int32_t tmp     = str.toInt();

    if ((0 <= tmp) ||
        (UINT16_MAX >= tmp))
    {
        value = static_cast<uint16_t>(tmp);
        success = true;
    }

    return success;
}

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
 * Get error information for invalid rest path requests.
 * 
 * @param[in] request   HTTP request
 */
static void error(AsyncWebServerRequest* request)
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
    jsonDoc["status"]   = static_cast<uint8_t>(STATUS_CODE_NOT_FOUND);
    errorObj["msg"]     = "Invalid path requested.";
    httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    
    serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
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
        jsonDoc["status"]   = static_cast<uint8_t>(STATUS_CODE_NOT_FOUND);
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
        jsonDoc["status"]       = static_cast<uint8_t>(STATUS_CODE_OK);

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
        jsonDoc["status"]   = static_cast<uint8_t>(STATUS_CODE_NOT_FOUND);
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        JsonObject  dataObj = jsonDoc.createNestedObject("data");

        dataObj["slots"] = DisplayMgr::getInstance().MAX_SLOTS;

        /* Prepare response */
        jsonDoc["status"]   = static_cast<uint8_t>(STATUS_CODE_OK);
        httpStatusCode      = HttpStatus::STATUS_CODE_OK;
    }

    serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

/**
 * Set text of a slot.
 * POST /display/slot/<slot-id>/text?show=<text>
 * 
 * @param[in] request   HTTP request
 */
static void slotText(AsyncWebServerRequest* request)
{
    String                  content;
    StaticJsonDocument<200> jsonDoc;
    uint32_t                httpStatusCode  = HttpStatus::STATUS_CODE_OK;
    
    if (NULL == request)
    {
        return;
    }

    if (HTTP_POST != request->method())
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = static_cast<uint8_t>(STATUS_CODE_NOT_FOUND);
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        uint8_t slotId = DisplayMgr::getInstance().MAX_SLOTS;
        
        /* Slot id invalid? */
        if ((false == toUInt8(request->pathArg(0), slotId)) ||
            (DisplayMgr::getInstance().MAX_SLOTS <= slotId))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Slot id not supported.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        /* "show" argument missing? */
        else if (false == request->hasArg("show"))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Show is missing.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else
        {
            String text = request->arg("show");

            DisplayMgr::getInstance().lock();
            DisplayMgr::getInstance().setText(slotId, text);
            DisplayMgr::getInstance().unlock();

            (void)jsonDoc.createNestedObject("data");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(STATUS_CODE_OK);
            httpStatusCode      = HttpStatus::STATUS_CODE_OK;
        }
    }

    serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

/**
 * Set bitmap of a slot.
 * POST /display/slot/<slot-id>/bitmap?width=<width-in-pixel>&height=<height-in-pixel>&data=<data-uint16_t>
 * 
 * @param[in] request   HTTP request
 */
static void slotBitmap(AsyncWebServerRequest* request)
{
    String                  content;
    StaticJsonDocument<200> jsonDoc;
    uint32_t                httpStatusCode  = HttpStatus::STATUS_CODE_OK;

    if (NULL == request)
    {
        return;
    }

    if (HTTP_POST != request->method())
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = static_cast<uint8_t>(STATUS_CODE_NOT_FOUND);
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        uint8_t     slotId  = DisplayMgr::getInstance().MAX_SLOTS;
        uint16_t    width   = 0u;
        uint16_t    height  = 0u;

        /* Slot id invalid? */
        if ((false == toUInt8(request->pathArg(0), slotId)) ||
            (DisplayMgr::getInstance().MAX_SLOTS <= slotId))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Slot id not supported.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        /* "width" argument missing? */
        else if (false == request->hasArg("width"))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Width is missing.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        /* "height" argument missing? */
        else if (false == request->hasArg("height"))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Height is missing.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        /* "data" argument missing? */
        else if (false == request->hasArg("data"))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Data is missing.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        /* Invalid width? */
        else if (false == toUInt16(request->arg("width"), width))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Invalid width.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        /* Invalid height? */
        else if (false == toUInt16(request->arg("height"), height))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Invalid height.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else
        {
            String          dataBase64Str       = request->arg("data");
            size_t          dataBase64ArraySize = dataBase64Str.length();
            const uint8_t*  dataBase64Array     = reinterpret_cast<const uint8_t*>(dataBase64Str.c_str());
            size_t          bitmapSize          = 0;
            uint16_t*       bitmap              = reinterpret_cast<uint16_t*>(base64_decode(dataBase64Array, dataBase64ArraySize, &bitmapSize));

            DisplayMgr::getInstance().lock();
            DisplayMgr::getInstance().setBitmap(slotId, bitmap, width, height);
            DisplayMgr::getInstance().unlock();

            delete bitmap;

            (void)jsonDoc.createNestedObject("data");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(STATUS_CODE_OK);
            httpStatusCode      = HttpStatus::STATUS_CODE_OK;
        }
    }

    serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

/**
 * Set lamp state of a slot.
 * POST /display/slot/<slot-id>/lamp/<lamp-id>/state?set=<on/off>
 * 
 * @param[in] request   HTTP request
 */
static void slotLamp(AsyncWebServerRequest* request)
{
    String                  content;
    StaticJsonDocument<200> jsonDoc;
    uint32_t                httpStatusCode  = HttpStatus::STATUS_CODE_OK;

    if (NULL == request)
    {
        return;
    }

    if (HTTP_POST != request->method())
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = static_cast<uint8_t>(STATUS_CODE_NOT_FOUND);
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        uint8_t slotId = DisplayMgr::getInstance().MAX_SLOTS;
        uint8_t lampId = 0u;

        /* Slot id invalid? */
        if ((false == toUInt8(request->pathArg(0), slotId)) ||
            (DisplayMgr::getInstance().MAX_SLOTS <= slotId))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Slot id not supported.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        /* Lamp id invalid? */
        else if (false == toUInt8(request->pathArg(1), lampId))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Lamp id not supported.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else if ((false == request->hasArg("set")) ||
                 ((request->arg("set") != "off") &&
                  (request->arg("set") != "on")))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Command not supported.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else
        {
            bool lampState = false;

            if (request->arg("set") == "on")
            {
                lampState = true;
            }
            
            DisplayMgr::getInstance().lock();
            DisplayMgr::getInstance().setLamp(slotId, lampId, lampState);
            DisplayMgr::getInstance().unlock();

            (void)jsonDoc.createNestedObject("data");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(STATUS_CODE_OK);
            httpStatusCode      = HttpStatus::STATUS_CODE_OK;
        }
    }

    serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

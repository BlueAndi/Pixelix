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
@brief  REST API
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
@see RestApi.h

*******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "RestApi.h"
#include "Html.h"
#include <ArduinoJson.h>
#include "Settings.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Get number of array elements. */
#define ARRAY_NUM(__arr)    (sizeof(__arr) / sizeof((__arr)[0]))

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

static void status(void);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/**
 * Web server
 */
static WebServer*   gWebServer  = NULL;

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

void RestApi::init(WebServer& srv)
{
    String  path;

    gWebServer = &srv;

    gWebServer->on("/api/v1/status", status);

    return;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Get status information.
 */
static void status(void)
{
    String                  content;
    StaticJsonDocument<200> jsonDoc;
    uint32_t                httpStatusCode  = Html::STATUS_CODE_OK;

    if (NULL == gWebServer)
    {
        return;
    }

    if (HTTP_GET == gWebServer->method())
    {
        JsonObject  dataObj = jsonDoc.createNestedObject("data");
        JsonObject  wifiObj = dataObj.createNestedObject("wifi");

        jsonDoc["status"]   = STATUS_CODE_OK;
        wifiObj["ssid"]     = Settings::getInstance().getWifiSSID();

        httpStatusCode = Html::STATUS_CODE_OK;
    }
    else
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        jsonDoc["status"]   = STATUS_CODE_NOT_FOUND;
        errorObj["msg"]     = "HTTP method not supported.";

        httpStatusCode = Html::STATUS_CODE_NOT_FOUND;
    }

    serializeJsonPretty(jsonDoc, content);
    gWebServer->send(httpStatusCode, "application/json", content);

    return;
}

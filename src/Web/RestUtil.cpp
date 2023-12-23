/* MIT License
 *
 * Copyright (c) 2019 - 2023 Andreas Merkle <web@blue-andi.de>
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
 * @brief  REST API Utilities
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "RestUtil.h"

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

JsonVariant RestUtil::prepareRspSuccess(JsonDocument& jsonDoc)
{
    JsonObject dataObj = jsonDoc.createNestedObject("data");

    jsonDoc["status"]   = "ok";

    return dataObj;
}

void RestUtil::prepareRspError(JsonDocument& jsonDoc, const char* msg)
{
    JsonObject errorObj = jsonDoc.createNestedObject("error");

    errorObj["msg"]     = msg;
    jsonDoc["status"]   = "error";
}

void RestUtil::prepareRspErrorHttpMethodNotSupported(JsonDocument& jsonDoc)
{
    prepareRspError(jsonDoc, "HTTP method not supported.");
}

void RestUtil::sendJsonRsp(AsyncWebServerRequest* request, const JsonDocument& jsonDoc, uint32_t httpStatusCode)
{
    if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document has less memory available.");
    }
    else
    {
        LOG_DEBUG("JSON document size: %u", jsonDoc.memoryUsage());
    }

    if (nullptr != request)
    {
        String content;

        (void)serializeJsonPretty(jsonDoc, content);
        request->send(httpStatusCode, "application/json", content);
    }
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

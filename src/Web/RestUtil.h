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
 *
 * @addtogroup web
 *
 * @{
 */

#ifndef REST_UTIL_H
#define REST_UTIL_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

/** REST API Utilities */
namespace RestUtil
{

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Prepare JSON document for success response.
 * 
 * @param[out]  jsonDoc JSON document
 * 
 * @return JSON object where to add additional data.
 */
JsonVariant prepareRspSuccess(JsonDocument& jsonDoc);

/**
 * Prepare JSON document for error response.
 * 
 * @param[out]  jsonDoc JSON document where to add error response.
 * @param[in]   msg     Error message
 */
void prepareRspError(JsonDocument& jsonDoc, const char* msg);

/**
 * Prepare JSON document for concrete error response: HTTP method not supported.
 * 
 * @param[out]  jsonDoc JSON document where to add error response.
 */
void prepareRspErrorHttpMethodNotSupported(JsonDocument& jsonDoc);

/**
 * Send a application/json response to the client back.
 * 
 * @param[in] request           Client request
 * @param[in] jsonDoc           JSON response document
 * @param[in] httpStatusCode    HTTP status code
 */
void sendJsonRsp(AsyncWebServerRequest* request, const JsonDocument& jsonDoc, uint32_t httpStatusCode);

}

#endif  /* REST_UTIL_H */

/** @} */
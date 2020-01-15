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
 * @brief  JustText plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "JustTextPlugin.h"
#include "RestApi.h"

#include <Logging.h>
#include <ArduinoJson.h>
#include <functional>

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

void JustTextPlugin::registerWebInterface(AsyncWebServer& srv, const String& baseUri)
{
    m_url = baseUri + "/text";

    m_callbackWebHandler = &srv.on( m_url.c_str(),
                                    [this](AsyncWebServerRequest *request)
                                    {
                                        this->webReqHandler(request);
                                    });

    LOG_INFO("[%s] Register: %s", getName(), m_url.c_str());

    return;
}

void JustTextPlugin::unregisterWebInterface(AsyncWebServer& srv)
{
    LOG_INFO("[%s] Unregister: %s", getName(), m_url.c_str());

    if (false == srv.removeHandler(m_callbackWebHandler))
    {
        LOG_WARNING("Couldn't remove %s handler.", getName());
    }

    m_callbackWebHandler = nullptr;

    return;
}

void JustTextPlugin::update(IGfx& gfx)
{
    gfx.fillScreen(ColorDef::convert888To565(ColorDef::BLACK));
    m_textWidget.update(gfx);

    return;
}

void JustTextPlugin::setText(const String& formatText)
{
    m_textWidget.setFormatStr(formatText);
    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void JustTextPlugin::webReqHandler(AsyncWebServerRequest *request)
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
    else
    {
        /* "show" argument missing? */
        if (false == request->hasArg("show"))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Show is missing.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else
        {
            String text = request->arg("show");

            setText(text);

            /* Prepare response */
            (void)jsonDoc.createNestedObject("data");
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);
            httpStatusCode      = HttpStatus::STATUS_CODE_OK;
        }
    }

    serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

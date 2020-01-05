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
 * @brief  Icon and text plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IconTextPlugin.h"
#include "RestApi.h"

#include <Logging.h>
#include <ArduinoJson.h>

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

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/* Initialize the list of instances. */
DLinkedList<IconTextPlugin*>    IconTextPlugin::m_instances;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void IconTextPlugin::active(IGfx& gfx)
{    
    if (NULL == m_iconCanvas)
    {
        m_iconCanvas = new Canvas(ICON_WIDTH, ICON_HEIGHT, 0, 0);
        
        if (NULL != m_iconCanvas)
        {
            m_iconCanvas->addWidget(m_bitmapWidget);
        }
    }

    if (NULL == m_textCanvas)
    {
        m_textCanvas = new Canvas(gfx.width() - ICON_WIDTH, gfx.height(), ICON_WIDTH, 0);

        if (NULL != m_textCanvas)
        {
            m_textCanvas->addWidget(m_textWidget);
            
            /* Move the text widget one line lower for better look. */
            m_textWidget.move(0, 1);
        }
    }

    return;
}

void IconTextPlugin::inactive(void)
{
    /* Nothing to do. */
    return;
}

void IconTextPlugin::registerWebInterface(AsyncWebServer& srv, const String& baseUri)
{
    IconTextPlugin* plugin = this;

    m_urlIcon = baseUri + "/bitmap";
    m_callbackWebHandlerIcon = &srv.on(m_urlIcon.c_str(), staticWebReqHandler);

    LOG_INFO("[%s] Register: %s", getName(), m_urlIcon.c_str());
    
    m_urlText = baseUri + "/text";
    m_callbackWebHandlerText = &srv.on(m_urlText.c_str(), staticWebReqHandler);

    LOG_INFO("[%s] Register: %s", getName(), m_urlText.c_str());

    m_instances.append(plugin);

    return;
}

void IconTextPlugin::unregisterWebInterface(AsyncWebServer& srv)
{
    if (false == m_instances.find(this))
    {
        LOG_WARNING("Couldn't find %s in own list.", this->getName());
    }
    else
    {
        m_instances.removeSelected();
    }

    LOG_INFO("[%s] Unregister: %s", m_urlIcon);

    if (false == srv.removeHandler(m_callbackWebHandlerIcon))
    {
        LOG_WARNING("Couldn't remove %s handler.", this->getName());
    }

    m_callbackWebHandlerIcon = NULL;

    LOG_INFO("[%s] Unregister: %s", m_urlText);

    if (false == srv.removeHandler(m_callbackWebHandlerText))
    {
        LOG_WARNING("Couldn't remove %s handler.", this->getName());
    }

    m_callbackWebHandlerText = NULL;

    return;
}

void IconTextPlugin::update(IGfx& gfx)
{
    gfx.fillScreen(ColorDef::convert888To565(ColorDef::BLACK));
    
    if (NULL != m_iconCanvas)
    {
        m_iconCanvas->update(gfx);
    }
    
    if (NULL != m_textCanvas)
    {
        m_textCanvas->update(gfx);
    }

    return;
}

void IconTextPlugin::setText(const String& formatText)
{
    m_textWidget.setFormatStr(formatText);
    return;
}

void IconTextPlugin::setBitmap(const uint16_t* bitmap, uint16_t width, uint16_t height)
{
    uint16_t*       buffer      = NULL;
    const uint16_t* oldBuffer   = NULL;
    uint16_t        oldWidth    = 0u;
    uint16_t        oldHeight   = 0u;

    if ((NULL == bitmap) ||
        (ICON_WIDTH < width) ||
        (ICON_HEIGHT < height))
    {
        return;
    }

    /* Store current bitmap buffer */
    oldBuffer = m_bitmapWidget.get(oldWidth, oldHeight);

    /* Get new bitmap buffer */
    buffer = new uint16_t[width * height];

    if (NULL != buffer)
    {
        /* Release old bitmap buffer */
        delete[] oldBuffer;

        /* Copy new bitmap */
        memcpy(buffer, bitmap, width * height * sizeof(uint16_t));

        m_bitmapWidget.set(buffer, width, height);
    }

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void IconTextPlugin::staticWebReqHandler(AsyncWebServerRequest *request)
{
    if (false == m_instances.selectFirstElement())
    {
        LOG_WARNING("Couldn't handle web req. for %s.", request->url().c_str());
    }
    else
    {
        IconTextPlugin**    elem    = m_instances.current();
        IconTextPlugin*     plugin  = NULL;

        while((NULL != elem) && (NULL == plugin))
        {
            if (((*elem)->m_urlIcon == request->url()) ||
                ((*elem)->m_urlText == request->url()))
            {
                plugin = *elem;
            }
            else
            {
                if (false == m_instances.next())
                {
                    elem = NULL;
                }
                else
                {
                    elem = m_instances.current();
                }
            }
        }

        if (NULL != plugin)
        {
            if (plugin->m_urlIcon == request->url())
            {
                plugin->webReqHandlerIcon(request);
            }
            else if (plugin->m_urlText == request->url())
            {
                plugin->webReqHandlerText(request);
            }
            else
            {
                /* Should never happen. */
                ;
            }
        }
    }

    return;
}

void IconTextPlugin::webReqHandlerText(AsyncWebServerRequest *request)
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

void IconTextPlugin::webReqHandlerIcon(AsyncWebServerRequest *request)
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
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
        errorObj["msg"]     = "HTTP method not supported.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        uint16_t    width   = 0u;
        uint16_t    height  = 0u;

        /* "width" argument missing? */
        if (false == request->hasArg("width"))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Width is missing.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        /* "height" argument missing? */
        else if (false == request->hasArg("height"))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Height is missing.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        /* "data" argument missing? */
        else if (false == request->hasArg("data"))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Data is missing.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        /* Invalid width? */
        else if (false == Util::strToUInt16(request->arg("width"), width))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Invalid width.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        /* Invalid height? */
        else if (false == Util::strToUInt16(request->arg("height"), height))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
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

            setBitmap(bitmap, width, height);

            delete bitmap;

            (void)jsonDoc.createNestedObject("data");

            /* Prepare response */
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

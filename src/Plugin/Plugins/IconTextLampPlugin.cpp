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
 * @brief  Icon, text and lamp plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IconTextLampPlugin.h"
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
DLinkedList<IconTextLampPlugin*>    IconTextLampPlugin::m_instances;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void IconTextLampPlugin::active(IGfx& gfx)
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
        m_textCanvas = new Canvas(gfx.width() - ICON_WIDTH, gfx.height() - 2, ICON_WIDTH, 0);

        if (NULL != m_textCanvas)
        {
            m_textCanvas->addWidget(m_textWidget);
        }
    }

    if (NULL == m_lampCanvas)
    {
        m_lampCanvas = new Canvas(gfx.width() - ICON_WIDTH, 1, ICON_WIDTH, gfx.height() - 1);
        
        if (NULL != m_lampCanvas)
        {
            uint8_t index = 0u;

            for(index = 0u; index < MAX_LAMPS; ++index)
            {
                /* One space at the begin, two spaces between the lamps. */
                int16_t x = (LampWidget::WIDTH + 2) * index + 1;

                m_lampCanvas->addWidget(m_lampWidgets[index]);
                m_lampWidgets[index].move(x, 0);
            }
        }
    }

    return;
}

void IconTextLampPlugin::inactive(void)
{
    /* Nothing to do. */
    return;
}

void IconTextLampPlugin::registerWebInterface(AsyncWebServer& srv, const String& baseUri)
{
    IconTextLampPlugin* plugin = this;

    m_urlIcon = baseUri + "/bitmap";
    m_callbackWebHandlerIcon = &srv.on(m_urlIcon.c_str(), staticWebReqHandler);

    LOG_INFO("[%s] Register: %s", getName(), m_urlIcon.c_str());
    
    m_urlText = baseUri + "/text";
    m_callbackWebHandlerText = &srv.on(m_urlText.c_str(), staticWebReqHandler);

    LOG_INFO("[%s] Register: %s", getName(), m_urlText.c_str());

    m_urlLamp = baseUri + "/lamp/*";
    m_callbackWebHandlerLamp = &srv.on(m_urlLamp.c_str(), staticWebReqHandler);

    LOG_INFO("[%s] Register: %s", getName(), m_urlLamp.c_str());

    m_instances.append(plugin);

    return;
}

void IconTextLampPlugin::unregisterWebInterface(AsyncWebServer& srv)
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

    LOG_INFO("[%s] Unregister: %s", m_urlLamp);

    if (false == srv.removeHandler(m_callbackWebHandlerLamp))
    {
        LOG_WARNING("Couldn't remove %s handler.", this->getName());
    }

    m_callbackWebHandlerLamp = NULL;

    return;
}

void IconTextLampPlugin::update(IGfx& gfx)
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

    if (NULL != m_lampCanvas)
    {
        m_lampCanvas->update(gfx);
    }

    return;
}

void IconTextLampPlugin::setText(const String& formatText)
{
    m_textWidget.setFormatStr(formatText);
    return;
}

void IconTextLampPlugin::setBitmap(const uint16_t* bitmap, uint16_t width, uint16_t height)
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

void IconTextLampPlugin::setLamp(uint8_t lampId, bool state)
{
    if (MAX_LAMPS > lampId)
    {
        m_lampWidgets[lampId].setOnState(state);
    }

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void IconTextLampPlugin::staticWebReqHandler(AsyncWebServerRequest *request)
{
    if (false == m_instances.selectFirstElement())
    {
        LOG_WARNING("Couldn't handle web req. for %s.", request->url().c_str());
    }
    else
    {
        IconTextLampPlugin**    elem    = m_instances.current();
        IconTextLampPlugin*     plugin  = NULL;
        String                  urlLamp;

        while((NULL != elem) && (NULL == plugin))
        {
            urlLamp = (*elem)->m_urlLamp.substring(0, (*elem)->m_urlLamp.length() - 2); // Remove the '*' at the end

            if (((*elem)->m_urlIcon == request->url()) ||
                ((*elem)->m_urlText == request->url()) ||
                (true == request->url().startsWith(urlLamp)))
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
            else if (true == request->url().startsWith(urlLamp))
            {
                plugin->webReqHandlerLamp(request);
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

void IconTextLampPlugin::webReqHandlerText(AsyncWebServerRequest *request)
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

void IconTextLampPlugin::webReqHandlerIcon(AsyncWebServerRequest *request)
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

void IconTextLampPlugin::webReqHandlerLamp(AsyncWebServerRequest *request)
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
        uint32_t    indexBeginLampId    = m_urlLamp.length() - 1;
        uint32_t    indexEndLampId      = request->url().indexOf("/", indexBeginLampId);
        String      slotIdStr           = request->url().substring(indexBeginLampId, indexEndLampId);
        uint8_t     lampId              = MAX_LAMPS;
        bool        status              = Util::strToUInt8(slotIdStr, lampId);

        /* Lamp id invalid? */
        if ((false == status) ||
            (MAX_LAMPS <= lampId))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
            errorObj["msg"]     = "Lamp id not supported.";
            httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else if ((false == request->hasArg("set")) ||
                 ((request->arg("set") != "off") &&
                  (request->arg("set") != "on")))
        {
            JsonObject errorObj = jsonDoc.createNestedObject("error");

            /* Prepare response */
            jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
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

            setLamp(lampId, lampState);

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

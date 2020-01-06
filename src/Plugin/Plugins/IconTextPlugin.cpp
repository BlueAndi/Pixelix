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
#include <SPIFFS.h>

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

/* Initialize upload path. */
const char*                     IconTextPlugin::UPLOAD_PATH = "/tmp";

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
    m_callbackWebHandlerIcon = &srv.on(m_urlIcon.c_str(), HTTP_ANY, staticWebReqHandler, staticUploadHandler);

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
        LOG_WARNING("Couldn't find %s in own list.", getName());
    }
    else
    {
        m_instances.removeSelected();
    }

    LOG_INFO("[%s] Unregister: %s", getName(), m_urlIcon.c_str());

    if (false == srv.removeHandler(m_callbackWebHandlerIcon))
    {
        LOG_WARNING("Couldn't remove %s handler.", getName());
    }

    m_callbackWebHandlerIcon = NULL;

    LOG_INFO("[%s] Unregister: %s", getName(), m_urlText.c_str());

    if (false == srv.removeHandler(m_callbackWebHandlerText))
    {
        LOG_WARNING("Couldn't remove %s handler.", getName());
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

void IconTextPlugin::staticUploadHandler(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final)
{
    if (false == m_instances.selectFirstElement())
    {
        LOG_WARNING("Couldn't handle upload req. for %s.", request->url().c_str());
    }
    else
    {
        IconTextPlugin**    elem    = m_instances.current();
        IconTextPlugin*     plugin  = NULL;

        while((NULL != elem) && (NULL == plugin))
        {
            if ((*elem)->m_urlIcon == request->url())
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
                plugin->iconUploadHandler(request, filename, index, data, len, final);
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
    /* Upload failed? */
    else if (true == m_isUploadError)
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
        errorObj["msg"]     = "Upload failed.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    /* Load bitmap file. */
    else if (false == m_bitmapWidget.load(getFileName()))
    {
        JsonObject errorObj = jsonDoc.createNestedObject("error");

        /* Prepare response */
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_NOT_FOUND);
        errorObj["msg"]     = "Incompatible file format.";
        httpStatusCode      = HttpStatus::STATUS_CODE_NOT_FOUND;
    }
    else
    {
        /* Prepare response */
        (void)jsonDoc.createNestedObject("data");
        jsonDoc["status"]   = static_cast<uint8_t>(RestApi::STATUS_CODE_OK);
        httpStatusCode      = HttpStatus::STATUS_CODE_OK;
    }

    serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

void IconTextPlugin::iconUploadHandler(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final)
{
    UTIL_NOT_USED(filename);

    /* Begin of upload? */
    if (0 == index)
    {
        LOG_INFO("Upload of %s (%d bytes) starts.", filename.c_str(), request->contentLength());
        m_isUploadError = false;

        if (false == filename.endsWith(".bmp"))
        {
            LOG_ERROR("File is not a bitmap file.");
            m_isUploadError = true;
        }
        else
        {
            /* All uploaded bitmaps shall be in a dedicated folder.
             * This folder may not be created yet.
             */
            if (false == SPIFFS.exists(UPLOAD_PATH))
            {
                if (false == SPIFFS.mkdir(UPLOAD_PATH))
                {
                    LOG_ERROR("Couldn't create directory: %s", UPLOAD_PATH);
                    m_isUploadError = true;
                }
            }

            /* Create a new file and overwrite a existing one. */
            m_fd = SPIFFS.open(getFileName(), "w");

            if (false == m_fd)
            {
                LOG_ERROR("Couldn't create file: %s", getFileName().c_str());
                m_isUploadError = true;
            }
        }
    }

    if (false == m_isUploadError)
    {
        /* If file is open, write data to it. */
        if (true == m_fd)
        {
            if (len != m_fd.write(data, len))
            {
                LOG_ERROR("Less data written, upload aborted.");
                m_isUploadError = true;

                m_fd.close();
            }
        }

        /* Upload finished? */
        if (true == final)
        {
            LOG_INFO("Upload of %s finished.", filename.c_str());

            m_fd.close();
        }
    }

    return;
}

String IconTextPlugin::getFileName(void)
{
    String filename = UPLOAD_PATH;

    filename += "/";
    filename += getSlotId();
    filename += ".bmp";

    return filename;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

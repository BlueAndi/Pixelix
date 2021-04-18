/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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
#include "FileSystem.h"

#include <Logging.h>
#include <ArduinoJson.h>

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

/* Initialize plugin topic. */
const char* IconTextLampPlugin::TOPIC_TEXT  = "/text";

/* Initialize plugin topic. */
const char* IconTextLampPlugin::TOPIC_LAMPS = "/lamps";

/* Initialize plugin topic. */
const char* IconTextLampPlugin::TOPIC_LAMP  = "/lamp";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void IconTextLampPlugin::getTopics(JsonArray& topics) const
{
    uint8_t lampId = 0U;

    topics.add(TOPIC_TEXT);
    topics.add(TOPIC_LAMPS);

    for(lampId = 0U; lampId < MAX_LAMPS; ++lampId)
    {
        topics.add(String(TOPIC_LAMP) + "/" + lampId);
    }
}

bool IconTextLampPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_TEXT))
    {
        String  formattedText   = getText();

        value["text"] = formattedText;

        isSuccessful = true;
    }
    else if (0U != topic.equals(TOPIC_LAMPS))
    {
        JsonArray   lampArray   = value.createNestedArray("lamps");
        uint8_t     lampId      = 0U;

        for(lampId = 0U; lampId < MAX_LAMPS; ++lampId)
        {
            bool        lampOnState = getLamp(lampId);
            JsonObject  lampObj     = lampArray.createNestedObject();

            lampObj["id"]       = lampId;
            lampObj["state"]    = (false == lampOnState) ? String("off") : String("on");
        }

        isSuccessful = true;
    }
    else if (0U != topic.startsWith(String(TOPIC_LAMP) + "/"))
    {
        uint32_t    indexBeginLampId    = topic.lastIndexOf("/") + 1U;
        String      lampIdStr           = topic.substring(indexBeginLampId);
        uint8_t     lampId              = MAX_LAMPS;
        bool        status              = Util::strToUInt8(lampIdStr, lampId);

        if ((true == status) &&
            (MAX_LAMPS > lampId))
        {
            bool    lampState       = getLamp(lampId);
            String  lampStateStr    = (false == lampState) ? "off" : "on";

            value["id"]     = lampId;
            value["state"]  = lampStateStr;

            isSuccessful = true;
        }
    }
    else
    {
        ;
    }

    return isSuccessful;
}

bool IconTextLampPlugin::setTopic(const String& topic, const JsonObject& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_TEXT))
    {
        String  text;

        if (false == value["show"].isNull())
        {
            text = value["show"].as<String>();
            isSuccessful = true;
        }

        if (true == isSuccessful)
        {
            setText(text);
        }
    }
    else if (0U != topic.startsWith(String(TOPIC_LAMP) + "/"))
    {
        uint32_t    indexBeginLampId    = topic.lastIndexOf("/") + 1U;
        String      lampIdStr           = topic.substring(indexBeginLampId);
        uint8_t     lampId              = MAX_LAMPS;
        bool        status              = Util::strToUInt8(lampIdStr, lampId);

        if ((true == status) &&
            (MAX_LAMPS > lampId) &&
            (false == value["set"].isNull()))
        {
            String state = value["set"].as<String>();

            if (state == "off")
            {
                setLamp(lampId, false);
                isSuccessful = true;
            }
            else if (state == "on")
            {
                setLamp(lampId, true);
                isSuccessful = true;
            }
            else
            {
                ;
            }
        }
    }
    else
    {
        ;
    }

    return isSuccessful;
}

void IconTextLampPlugin::active(IGfx& gfx)
{
    lock();

    if (nullptr == m_iconCanvas)
    {
        m_iconCanvas = new Canvas(ICON_WIDTH, ICON_HEIGHT, 0, 0);

        if (nullptr != m_iconCanvas)
        {
            (void)m_iconCanvas->addWidget(m_bitmapWidget);

            /* If there is already a icon in the filesystem, load it. */
            (void)m_bitmapWidget.load(FILESYSTEM, getFileName());
        }
    }

    if (nullptr == m_textCanvas)
    {
        m_textCanvas = new Canvas(gfx.getWidth() - ICON_WIDTH, gfx.getHeight() - 2U, ICON_WIDTH, 0);

        if (nullptr != m_textCanvas)
        {
            (void)m_textCanvas->addWidget(m_textWidget);
        }
    }

    if (nullptr == m_lampCanvas)
    {
        m_lampCanvas = new Canvas(gfx.getWidth() - ICON_WIDTH, 1U, ICON_WIDTH, gfx.getHeight() - 1);

        if (nullptr != m_lampCanvas)
        {
            uint8_t index = 0U;

            for(index = 0U; index < MAX_LAMPS; ++index)
            {
                /* One space at the begin, two spaces between the lamps. */
                int16_t x = (LampWidget::DEFAULT_WIDTH + 2) * index + 1;

                (void)m_lampCanvas->addWidget(m_lampWidgets[index]);
                m_lampWidgets[index].move(x, 0);
            }
        }
    }

    unlock();

    return;
}

void IconTextLampPlugin::inactive()
{
    /* Nothing to do. */
    return;
}

void IconTextLampPlugin::registerWebInterface(AsyncWebServer& srv, const String& baseUri)
{
    m_urlIcon = baseUri + "/bitmap";
    m_callbackWebHandlerIcon = &srv.on( m_urlIcon.c_str(),
                                        HTTP_ANY,
                                        [this](AsyncWebServerRequest *request)
                                        {
                                            this->webReqHandlerIcon(request);
                                        },
                                        [this](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final)
                                        {
                                            this->iconUploadHandler(request, filename, index, data, len, final);
                                        });

    LOG_INFO("[%s] Register: %s", getName(), m_urlIcon.c_str());

    return;
}

void IconTextLampPlugin::unregisterWebInterface(AsyncWebServer& srv)
{
    LOG_INFO("[%s] Unregister: %s", getName(), m_urlIcon.c_str());

    if (false == srv.removeHandler(m_callbackWebHandlerIcon))
    {
        LOG_WARNING("Couldn't remove %s handler.", this->getName());
    }

    m_callbackWebHandlerIcon = nullptr;

    return;
}

void IconTextLampPlugin::update(IGfx& gfx)
{
    lock();

    gfx.fillScreen(ColorDef::BLACK);

    if (nullptr != m_iconCanvas)
    {
        m_iconCanvas->update(gfx);
    }

    if (nullptr != m_textCanvas)
    {
        m_textCanvas->update(gfx);
    }

    if (nullptr != m_lampCanvas)
    {
        m_lampCanvas->update(gfx);
    }

    unlock();

    return;
}

String IconTextLampPlugin::getText() const
{
    String formattedText;

    lock();
    formattedText = m_textWidget.getFormatStr();
    unlock();

    return formattedText;
}

void IconTextLampPlugin::setText(const String& formatText)
{
    unlock();
    m_textWidget.setFormatStr(formatText);
    unlock();

    return;
}

void IconTextLampPlugin::setBitmap(const Color* bitmap, uint16_t width, uint16_t height)
{
    if ((nullptr != bitmap) &&
        (ICON_WIDTH >= width) &&
        (ICON_HEIGHT >= height))
    {
        lock();
        m_bitmapWidget.set(bitmap, width, height);
        unlock();
    }

    return;
}

bool IconTextLampPlugin::loadBitmap(const String& filename)
{
    bool status = false;

    lock();
    status = m_bitmapWidget.load(FILESYSTEM, filename);
    unlock();

    return status;
}

bool IconTextLampPlugin::getLamp(uint8_t lampId) const
{
    bool lampState = false;

    if (MAX_LAMPS > lampId)
    {
        lock();
        lampState = m_lampWidgets[lampId].getOnState();
        unlock();
    }

    return lampState;
}

void IconTextLampPlugin::setLamp(uint8_t lampId, bool state)
{
    if (MAX_LAMPS > lampId)
    {
        lock();
        m_lampWidgets[lampId].setOnState(state);
        unlock();
    }

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void IconTextLampPlugin::webReqHandlerIcon(AsyncWebServerRequest *request)
{
    String              content;
    const size_t        JSON_DOC_SIZE   = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;

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
    else if (false == loadBitmap(getFileName()))
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

    if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document has less memory available.");
    }
    else
    {
        LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
    }

    (void)serializeJsonPretty(jsonDoc, content);
    request->send(httpStatusCode, "application/json", content);

    return;
}

void IconTextLampPlugin::iconUploadHandler(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final)
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
            /* Create a new file and overwrite a existing one. */
            m_fd = FILESYSTEM.open(getFileName(), "w");

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

String IconTextLampPlugin::getFileName()
{
    return generateFullPath(".bmp");
}

void IconTextLampPlugin::lock() const
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreTakeRecursive(m_xMutex, portMAX_DELAY);
    }

    return;
}

void IconTextLampPlugin::unlock() const
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreGiveRecursive(m_xMutex);
    }

    return;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @brief  REST service
 * @author Niklas Kümmel
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "RestService.h"

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

bool RestService::start()
{
    return true;
}

void RestService::stop()
{
}

void RestService::process()
{
    if (true == m_mutex.take(portMAX_DELAY))
    {

        Cmd cmd;

        if (true == m_cmdQueue.receive(&cmd, 0U))
        {
            if (true == m_client.begin(cmd.url))
            {
                switch (cmd.id)
                {
                case CMD_ID_GET:
                    if (false == m_client.GET(cmd.restId))
                    {
                        Msg msg;

                        msg.restId = cmd.restId;
                        msg.isMsg  = false;
                        msg.rsp    = nullptr;

                        if (true == this->m_taskProxy.send(msg))
                        {
                            LOG_ERROR("Msg could not be sent to Msg-Queue");
                        }

                        m_mutex.give();
                    }
                    break;

                case CMD_ID_POST:
                    if (false == m_client.POST(cmd.restId, cmd.u.data.data, cmd.u.data.size))
                    {
                        Msg msg;

                        msg.restId = cmd.restId;
                        msg.isMsg  = false;
                        msg.rsp    = nullptr;

                        if (true == this->m_taskProxy.send(msg))
                        {
                            LOG_ERROR("Msg could not be sent to Msg-Queue");
                        }

                        m_mutex.give();
                    }
                    break;

                default:
                    break;
                };
            }
        }
        else
        {
            m_mutex.give();
        }
    }
}

int RestService::registerPlugin()
{
    return restIdCounter++;
}

void RestService::setFilter(const int restId, DynamicJsonDocument* filter)
{
    if (filters.find(restId) == filters.end())
    {
        filters[restId] = filter;
    }
}

void RestService::deleteFilter(const int restId)
{
    filters.erase(restId);
}

bool RestService::get(const int restId, const String& url)
{
    Cmd cmd;

    memset(&cmd, 0, sizeof(cmd));
    cmd.id     = CMD_ID_GET;
    cmd.restId = restId;
    cmd.url    = url;

    return m_cmdQueue.sendToBack(cmd, portMAX_DELAY);
}

bool RestService::post(const int restId, const String& url, const uint8_t* payload, size_t size)
{
    Cmd cmd;

    memset(&cmd, 0, sizeof(cmd));
    cmd.id          = CMD_ID_POST;
    cmd.restId      = restId;
    cmd.url         = url;
    cmd.u.data.data = payload;
    cmd.u.data.size = size;

    return m_cmdQueue.sendToBack(cmd, portMAX_DELAY);
}

bool RestService::post(const int restId, const String& url, const String& payload)
{
    Cmd cmd;

    memset(&cmd, 0, sizeof(cmd));
    cmd.id          = CMD_ID_POST;
    cmd.restId      = restId;
    cmd.url         = url;
    cmd.u.data.data = reinterpret_cast<const uint8_t*>(payload.c_str());
    cmd.u.data.size = payload.length();

    return m_cmdQueue.sendToBack(cmd, portMAX_DELAY);
}

void RestService::handleAsyncWebResponse(const int restId, const HttpResponse& rsp)
{
    const size_t         JSON_DOC_SIZE    = 4096U;
    DynamicJsonDocument* jsonDoc          = new (std::nothrow) DynamicJsonDocument(JSON_DOC_SIZE);
    bool                 isError          = false;
    bool                 isSuccessfulSent = false;

    if (HttpStatus::STATUS_CODE_OK == rsp.getStatusCode())
    {
        if (nullptr != jsonDoc)
        {
            size_t      payloadSize = 0U;
            const void* vPayload    = rsp.getPayload(payloadSize);
            const char* payload     = static_cast<const char*>(vPayload);

            if (filters.find(restId) != filters.end())
            {
                DynamicJsonDocument* const filter = filters[restId];
                if (true == filter->overflowed())
                {
                    LOG_ERROR("Less memory for filter available.");
                    isError = true;
                }
                else if ((nullptr == payload) ||
                         (0U == payloadSize))
                {
                    LOG_ERROR("No payload.");
                    isError = true;
                }
                else
                {
                    DeserializationError error = deserializeJson(*jsonDoc, payload, payloadSize, DeserializationOption::Filter(filter->as<JsonVariantConst>()));

                    if (DeserializationError::Ok != error.code())
                    {
                        LOG_WARNING("JSON parse error: %s", error.c_str());
                        isError = true;
                    }
                }
            }
            else if ((nullptr == payload) ||
                     (0U == payloadSize))
            {
                LOG_ERROR("No payload.");
                isError = true;
            }
            else
            {
                DeserializationError error = deserializeJson(*jsonDoc, payload, payloadSize);

                if (DeserializationError::Ok != error.code())
                {
                    LOG_WARNING("JSON parse error: %s", error.c_str());
                    isError = true;
                }
            }
        }
        else
        {
            LOG_ERROR("Not enough memory available to store DynamicJsonDocument");
            isError = true;
        }
    }
    else
    {
        isError = true;
    }
    if (true == isError)
    {
        Msg msg;

        msg.restId       = restId;
        msg.isMsg        = false;
        msg.rsp          = nullptr;
        isSuccessfulSent = this->m_taskProxy.send(msg);
    }
    else
    {
        Msg msg;

        msg.restId       = restId;
        msg.isMsg        = true;
        msg.rsp          = jsonDoc;

        isSuccessfulSent = this->m_taskProxy.send(msg);
    }
    if (false == isSuccessfulSent)
    {
        LOG_ERROR("Msg could not be sent to Msg-Queue");
        delete jsonDoc;
        jsonDoc = nullptr;
    }
    m_mutex.give();
}

bool RestService::getResponse(const int restId, bool& isValidRsp, DynamicJsonDocument* payload)
{
    bool             isSuccessful = false;
    std::vector<Msg> buffer;

    do
    {
        Msg msg;

        if (true == m_taskProxy.receive(msg))
        {
            if (restId == msg.restId)
            {
                isValidRsp   = msg.isMsg;
                payload      = msg.rsp;
                isSuccessful = true;
                break;
            }
            else
            {
                buffer.push_back(msg);
            }
        }
        else
        {
            break;
        }
    }
    while (true);

    if (0 != buffer.size())
    {
        for (int i = 0; i < buffer.size(); i++)
        {
            Msg msg = buffer.back();
            buffer.pop_back();

            if (false == m_taskProxy.sendToFront(msg))
            {
                LOG_ERROR("Msg could not be sent to Msg-Queue");
            }
        }
    }

    return isSuccessful;
}
/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

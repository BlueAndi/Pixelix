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
 * @file   RequestHandler.h
 * @brief  Makapix request handler
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <WString.h>
#include <SimpleTimer.hpp>
#include "MakapixTypes.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Makapix request handler class.
 */
class RequestHandler
{
public:

    /**
     * Constructs the request handler.
     */
    RequestHandler() :
        m_onResponseCallback(nullptr),
        m_playerKey(),
        m_mqttInstance(0U),
        m_requestTimer(),
        m_lastRequestId(0U)
    {
    }

    /**
     * Destroys the request handler.
     */
    ~RequestHandler()
    {
    }

    /**
     * Initialize the request handler.
     *
     * @param[in] onResponseCallback   Callback function which is called on response.
     */
    void init(MakapixOnResponseCallback onResponseCallback);

    /**
     * Configure the request handler.
     * Any pending request will be aborted.
     *
     * @param[in] playerKey     Player key.
     * @param[in] mqttInstance  MQTT instance index.
     */
    void configure(const String& playerKey, uint8_t mqttInstance);

    /**
     * Process the request handler.
     */
    void process();

    /**
     * Send request to the server.
     * It will abort any pending request.
     *
     * @param[in] channelName   The channel name.
     * @param[in] userHandle    The user handle is only required for channel CHANNEL_BY_USER.
     * @param[in] sortOrder     Sort order.
     * @param[in] page          Page for pagination.
     * @param[in] limit         Number of items per page.
     *
     * @return If request is successful sent, it will return true otherwise false.
     */
    bool request(const char* channelName, const char* userHandle, const char* sortOrder, uint32_t page, uint32_t limit);

    /**
     * Check if a request is pending.
     *
     * @return If a request is pending, it will return true otherwise false.
     */
    bool isPending() const
    {
        return m_requestTimer.isTimerRunning();
    }

    /**
     * Abort pending request.
     */
    void abort();

private:

    /**
     * Request timeout in milliseconds.
     */
    static const uint32_t     REQUEST_TIMEOUT = SIMPLE_TIMER_SECONDS(30U);

    MakapixOnResponseCallback m_onResponseCallback; /**< Callback function which is called on response. */
    String                    m_playerKey;          /**< Player key. */
    uint8_t                   m_mqttInstance;       /**< MQTT instance index. */
    SimpleTimer               m_requestTimer;       /**< Timer used to observe a request. */
    uint32_t                  m_lastRequestId;      /**< The last request id used to generate the next id. */

    /**
     * The MQTT callback is registered by subscription and will be called on change by
     * the MQTT service.
     *
     * @param[in] topic     Topic
     * @param[in] payload   Topic payload
     * @param[in] size      Topic payload size in byte
     */
    void mqttTopicCallback(const String& topic, const uint8_t* payload, size_t size);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* REQUEST_HANDLER_H */

/** @} */

/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   RestService.h
 * @brief  REST service
 * @author Niklas Kümmel (niklas-kuemmel@web.de)
 *
 * @addtogroup REST_SERVICE
 *
 * @{
 */

#ifndef REST_SERVICE_H
#define REST_SERVICE_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <IService.hpp>
#include <HttpService.h>
#include <Mutex.hpp>
#include <ArduinoJson.h>
#include <map>
#include <Logging.h>
#include <vector>
#include <utility>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The REST service handles outgoing REST-API calls and their responses.
 */
class RestService : public IService
{
public:

    /**
     * Prototype of a preprocessing callback for a successful response.
     */
    typedef std::function<bool(const char*, size_t, DynamicJsonDocument&)> PreProcessCallback;

    /**
     * Get the REST service instance.
     *
     * @return REST service instance
     */
    static RestService& getInstance()
    {
        static RestService instance; /* idiom */

        return instance;
    }

    /**
     * Start the service.
     *
     * @return If successful started, it will return true otherwise false.
     */
    bool start() final;

    /**
     * Stop the service.
     */
    void stop() final;

    /**
     * Process the service.
     */
    void process() final;

    /**
     * Send GET request to host.
     *
     * @param[in] url                 URL
     * @param[in] preProcessCallback  PreProcessCallback which will be called by the RestService to filter the received date.
     *
     * @return If request is successful sent, it will return a valid restId otherwise it will return INVALID_REST_ID.
     */
    uint32_t get(const String& url, PreProcessCallback preProcessCallback);

    /**
     * Send POST request to host.
     *
     * @param[in] url                 URL
     * @param[in] preProcessCallback  PreProcessCallback which will be called by the RestService to filter the received date.
     * @param[in] payload             Payload, which must be kept alive until response is available!
     * @param[in] size                Payload size in byte
     *
     * @return If request is successful sent, it will return a valid restId otherwise it will return INVALID_REST_ID.
     */
    uint32_t post(const String& url, PreProcessCallback preProcessCallback, const uint8_t* payload = nullptr, size_t size = 0U);

    /**
     * Send POST request to host.
     *
     * @param[in] url                 URL
     * @param[in] payload             Payload, which must be kept alive until response is available!
     * @param[in] preProcessCallback  PreProcessCallback which will be called by the RestService to filter the received date.
     *
     * @return If request is successful sent, it will return a valid restId otherwise it will return INVALID_REST_ID.
     */
    uint32_t post(const String& url, const String& payload, PreProcessCallback preProcessCallback);

    /**
     * Get response to a previously started request.
     *
     * @param[in]  restId      Unique Id to identify plugin
     * @param[out] isValidRsp  Does Response have a payload
     * @param[out] payload     Content of the Response. The output variable is move-assigned.
     *
     * @return If a response is available, it will return true otherwise false.
     */
    bool getResponse(uint32_t restId, bool& isValidRsp, DynamicJsonDocument& payload);

    /**
     * Aborts a pending request. If there is already a response in the response queue it will be deleted as well.
     *
     * @param[in] restId Unique Id to identify plugin
     */
    void abortRequest(uint32_t restId);

    /**
     *  Used to indicate that HTTP request could not be started.
     */
    static constexpr uint32_t INVALID_REST_ID = 0U;

private:

    /**
     * Response that will be queued in m_responseQueue. Contains validity and data of a response.
     */
    struct Response
    {
        uint32_t            restId;      /**< Used to identify plugin in RestService. */
        bool                isRsp;       /**< true: successful response, false: request failed */
        DynamicJsonDocument jsonDocData; /**< Content of the response. Only valid if isRsp == true. */

        /**
         * Constructs a response.
         */
        Response() :
            restId(INVALID_REST_ID),
            isRsp(false),
            jsonDocData(4096U)
        {
        }

        /**
         * Constructs a response.
         *
         * @param[in] dataSize Size of the DynamicJsonDocument which holds the data.
         */
        explicit Response(size_t dataSize) :
            restId(INVALID_REST_ID),
            isRsp(false),
            jsonDocData(dataSize)
        {
        }
    };

    /**
     * Request ids are used to identify what the user requests.
     */
    enum RequestId
    {
        REQUEST_ID_GET = 0, /**< GET request */
        REQUEST_ID_POST     /**< POST request */
    };

    /**
     * A combination of a request and its corresponding data.
     */
    struct Request
    {
        RequestId          id;                 /**< The request id identifies the kind of request. */
        uint32_t           restId;             /**< Used to identify plugin in RestService. */
        PreProcessCallback preProcessCallback; /**< Individual callback called when response arrives. */
        String             url;                /**< URL to be called. */

        /**
         * Data parameters, only valid for REQUEST_ID_POST.
         */
        struct
        {
            const uint8_t* data; /**< Request specific data. Struct does not have ownership! */
            size_t         size; /**< Request specific data size in byte. */
        } data;

        /**
         * Constructs a request.
         */
        Request() :
            id(),
            restId(INVALID_REST_ID),
            preProcessCallback(nullptr),
            url(),
            data{ nullptr, 0U }
        {
        }

        /**
         * Constructs a request by copying an existing one.
         *
         * @param[in] other The response which to copy.
         */
        Request(const Request& other)            = default;

        /**
         * Assign a request.
         *
         * @param[in] other Request, which to assign.
         *
         * @return The request.
         */
        Request& operator=(const Request& other) = default;

        /**
         * Constructs a request by moving from an existing one.
         *
         * @param[in] other The request to move from.
         */
        Request(Request&& other) noexcept
            :
            id(other.id),
            restId(other.restId),
            preProcessCallback(other.preProcessCallback),
            url(std::move(other.url)),
            data{ other.data.data, other.data.size }
        {
            other.data.data = nullptr;
            other.data.size = 0U;
        }

        /**
         * Move a request.
         *
         * @param[in] other The request to move.
         *
         * @return The request.
         */
        Request& operator=(Request&& other) noexcept
        {
            if (this != &other)
            {
                id                 = other.id;
                restId             = other.restId;
                preProcessCallback = std::move(other.preProcessCallback);
                url                = std::move(other.url);
                data               = other.data;

                other.data.data    = nullptr;
                other.data.size    = 0U;
            }
            return *this;
        }
    };

    /** Request Queue */
    typedef std::vector<Request> RequestQueue;

    /** Response Queue */
    typedef std::vector<Response> ResponseQueue;

    RequestQueue                  m_requestQueue;             /**< Stores requests for sequential execution. */
    ResponseQueue                 m_responseQueue;            /**< Saves responses to outgoing requests. */
    bool                          m_isRunning;                /**< Signals the status of the service. True means it is running, false means it is stopped. */
    uint32_t                      m_restIdCounter;            /**< Used to generate restIds. */
    uint32_t                      m_activeRestId;             /**< Saves the restId of the in-flight request. */
    HttpJobId                     m_activeHttpJobId;          /**< Saves the HTTP service job id of the in-flight request. */
    PreProcessCallback            m_activePreProcessCallback; /**< Saves the callback sent by a request until the response arrives. */
    Mutex                         m_mutex;                    /**< Mutex to protect against concurrent access. */

    /**
     * Constructs the service instance.
     */
    RestService() :
        IService(),
        m_requestQueue(),
        m_responseQueue(),
        m_isRunning(false),
        m_restIdCounter(INVALID_REST_ID),
        m_activeRestId(INVALID_REST_ID),
        m_activeHttpJobId(INVALID_HTTP_JOB_ID),
        m_activePreProcessCallback(),
        m_mutex()
    {
    }

    /**
     * Destroys the service instance.
     */
    ~RestService()
    {
        /* Never called. */
    }

    /* An instance shall not be copied. */
    RestService(const RestService& service);
    RestService& operator=(const RestService& service);

    /**
     * Handles HTTP responses from the HTTP service.
     *
     * @param[in] httpRsp HTTP response.
     */
    void handleHttpResponse(const HttpRsp& httpRsp);

    /**
     * Generates a valid restId.
     *
     * @return A valid restId.
     */
    uint32_t getRestId();

    /**
     * Checks if the service is waiting for a response.
     *
     * @return True if waiting for a response, false otherwise.
     */
    inline bool isWaitingForResponse() const
    {
        return (INVALID_REST_ID != m_activeRestId) && (INVALID_HTTP_JOB_ID != m_activeHttpJobId);
    }
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* REST_SERVICE_H */

/** @} */
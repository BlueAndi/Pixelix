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
#include <PsramJsonDocument.hpp>
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
     * Result of a preprocessing callback.
     *
     * It tells the REST service whether the response was handled, whether it
     * is worth another attempt with a larger document, or whether the payload
     * is unusable.
     */
    enum class PreProcessResult
    {
        OK = 0,     /**< The result is stored in the document. */
        NEEDS_MORE, /**< The document is too small. Retry with a larger one. */
        FAILED      /**< The payload is not usable. Do not retry. */
    };

    /**
     * Prototype of a preprocessing callback for a successful response.
     *
     * The REST service provides a document, which is sized by a heuristic
     * based on the payload size. The heuristic can be wrong, because the
     * parsed representation of a structure heavy payload needs more memory
     * than its JSON text. Therefore a callback, which runs out of document
     * memory, shall return PreProcessResult::NEEDS_MORE instead of
     * PreProcessResult::FAILED. The REST service will then call it again with
     * a larger document, up to DOC_PARSE_ATTEMPTS times.
     *
     * Note, the callback may be called more than once for the same response.
     * It must not have any side effect outside of the given document.
     */
    typedef std::function<PreProcessResult(const char*, size_t, PsramJsonDocument&)> PreProcessCallback;

    /**
     * Map a deserialization error to a preprocessing result.
     *
     * A document which ran out of memory is worth another attempt with a
     * larger one, therefore it is mapped to PreProcessResult::NEEDS_MORE.
     * Any other error is final and gets logged.
     *
     * @param[in] error Deserialization error
     *
     * @return Preprocessing result
     */
    static PreProcessResult toPreProcessResult(const DeserializationError& error);

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
     * @param[in] payload             Payload, which is copied by the underlying HTTP service.
     * @param[in] size                Payload size in byte
     *
     * @return If request is successful sent, it will return a valid restId otherwise it will return INVALID_REST_ID.
     */
    uint32_t post(const String& url, PreProcessCallback preProcessCallback, const uint8_t* payload = nullptr, size_t size = 0U);

    /**
     * Send POST request to host.
     *
     * @param[in] url                 URL
     * @param[in] payload             Payload, which is copied by the underlying HTTP service.
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
    bool getResponse(uint32_t restId, bool& isValidRsp, PsramJsonDocument& payload);

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
        uint32_t          restId;      /**< Used to identify plugin in RestService. */
        bool              isRsp;       /**< true: successful response, false: request failed */
        PsramJsonDocument jsonDocData; /**< Content of the response. Only valid if isRsp == true. */

        /**
         * Constructs a response.
         */
        Response() :
            restId(INVALID_REST_ID),
            isRsp(false),
            jsonDocData(0U)
        {
        }

        /**
         * Constructs a response.
         *
         * @param[in] dataSize Size of the PsramJsonDocument which holds the data.
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
            preProcessCallback(std::move(other.preProcessCallback)),
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

    /**
     * Slot size in byte of a 32 bit target, which the payload size based
     * document sizing is calibrated for.
     */
    static const size_t JSON_SLOT_SIZE_32_BIT = 16U;

    /**
     * Ratio between the slot size of the build platform and the one of a
     * 32 bit target. It is 1 on the target itself and 2 on a 64 bit host,
     * where every slot costs twice as much.
     */
    static const size_t DOC_SIZE_SLOT_RATIO   = (JSON_OBJECT_SIZE(1U) + JSON_SLOT_SIZE_32_BIT - 1U) / JSON_SLOT_SIZE_32_BIT;

    /**
     * Lower limit for the response document size in byte. It covers small
     * payloads, whose parsed representation is a multiple of their text.
     */
    static const size_t DOC_SIZE_MIN          = 512U;

    /**
     * Upper limit for the response document size in byte. It stops a runaway
     * allocation if a callback keeps asking for more.
     *
     * A target without PSRAM has to share its internal RAM with the WiFi and
     * TLS buffers, therefore the budget is much smaller there. Note, the limit
     * never undercuts the payload size, see getMaxDocSize().
     */
#if defined(BOARD_HAS_PSRAM) || defined(NATIVE)
    static const size_t DOC_SIZE_MAX = 128U * 1024U;
#else
    static const size_t DOC_SIZE_MAX = 16U * 1024U;
#endif /* defined(BOARD_HAS_PSRAM) || defined(NATIVE) */

    /**
     * Max. number of attempts to preprocess a response. Every attempt doubles
     * the document size, therefore the document grows up to
     * 2^(DOC_PARSE_ATTEMPTS - 1) times the initial size.
     */
    static const uint8_t DOC_PARSE_ATTEMPTS = 4U;

    RequestQueue         m_requestQueue;             /**< Stores requests for sequential execution. */
    ResponseQueue        m_responseQueue;            /**< Saves responses to outgoing requests. */
    bool                 m_isRunning;                /**< Signals the status of the service. True means it is running, false means it is stopped. */
    uint32_t             m_restIdCounter;            /**< Used to generate restIds. */
    uint32_t             m_activeRestId;             /**< Saves the restId of the in-flight request. */
    HttpJobId            m_activeHttpJobId;          /**< Saves the HTTP service job id of the in-flight request. */
    PreProcessCallback   m_activePreProcessCallback; /**< Saves the callback sent by a request until the response arrives. */
    Mutex                m_mutex;                    /**< Mutex to protect against concurrent access. */

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
     * Determine the size of the first response document for a given payload.
     *
     * @param[in] payloadSize Payload size in byte
     *
     * @return Document size in byte
     */
    static size_t getInitialDocSize(size_t payloadSize);

    /**
     * Determine the upper limit of the response document for a given payload.
     *
     * @param[in] payloadSize Payload size in byte
     *
     * @return Max. document size in byte
     */
    static size_t getMaxDocSize(size_t payloadSize);

    /**
     * Preprocess a response payload, growing the document on demand.
     *
     * @param[in]  payload     Response payload
     * @param[in]  payloadSize Payload size in byte
     * @param[out] jsonDoc     Document which receives the result
     *
     * @return If successful it will return true otherwise false.
     */
    bool preProcessResponse(const char* payload, size_t payloadSize, PsramJsonDocument& jsonDoc);

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
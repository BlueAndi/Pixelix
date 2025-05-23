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
#include <AsyncHttpClient.h>
#include <TaskProxy.hpp>
#include <Mutex.hpp>
#include <ArduinoJson.h>
#include <map>
#include <HttpStatus.h>
#include <Logging.h>
#include <vector>
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
     * callback prototype.
     */

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
     * Get restId for plugin
     *
     * @return Unique restId
     */
    int registerPlugin();

    /**
     * Set filter, only one per plugin
     *
     * @param[in] restId Unique Id to identify plugin
     * @param[in] filter Filter in JSON format, can be null
     */
    void setFilter(const int restId, DynamicJsonDocument* filter);

    /**
     * Delete filter if one exists
     *
     * @param[in] restId Unqiue Id to identify plugin
     */
    void deleteFilter(const int restId);

    /**
     * Send GET request to host
     *
     * @param[in] restId  Unique Id to identify plugin
     * @param[in] url     URL
     *
     * @return If request is successful sent, it will return true otherwise false.
     */
    bool get(const int restId, const String& url);

    /**
     * Send POST request to host
     *
     * @param[in] restId   Unique Id to identify plugin
     * @param[in] url      URL
     * @param[in] payload  Payload, which must be kept alive until response is available!
     * @param[in] size     Payload size in byte
     *
     * @return If request is successful sent, it will return true otherwise false.
     */
    bool post(const int restId, const String& url, const uint8_t* payload = nullptr, size_t size = 0U);

    /**
     * Send POST request to host.
     * @param[in] restId   Unique Id to identify plugin
     * @param[in] url      URL
     * @param[in] payload  Payload, which must be kept alive until response is available!
     *
     * @return If request is successful sent, it will return true otherwise false.
     */
    bool post(const int restId, const String& url, const String& payload);

    /**
     * Get Response
     *
     * @param[in]  restId      Unique Id to identify plugin
     * @param[out] isValidRsp  Does Response have a payload
     * @param[out] payload     Content of the Response
     *
     * @return If a response is available, it will return true otherwise false
     */

    bool getResponse(const int restId, bool& isValidRsp, DynamicJsonDocument* payload);

private:

    /**
     *  Max. number of commands which can be queued. Must be increased when new user of RestService is added
     */
    static const size_t CMD_QUEUE_SIZE = 9U;

    struct Msg
    {
        int                  restId; /**< Used to identify plugin in RestService */
        bool                 isMsg;  /**< true: successful Response, false: request failed*/
        DynamicJsonDocument* rsp;    /**< Response, only valid if isMsg == true */

        /**
         * Constructs a message.
         */
        Msg() :
            isMsg(false),
            rsp(nullptr)
        {
        }
    };

    enum CmdId
    {
        CMD_ID_GET = 0, /**< GET request */
        CMD_ID_POST     /**< POST request */
    };

    /**
     * A command is a combination of request and its corresponding data.
     */
    struct Cmd
    {
        CmdId  id;     /**< The command id identifies the kind of request. */
        int    restId; /**< Used to identify plugin in RestService */
        String url;    /**< URL */

        /**
         * The union contains the event id specific parameters.
         * Note not every command id must have parameters.
         */
        union
        {
            /**
             * Data parameters, only valid for CMD_ID_POST.
             */
            struct
            {
                const uint8_t* data; /**< Command specific data. */
                size_t         size; /**< Command specific data size in byte. */
            } data;

        } u;
    };

    int                    restIdCounter; /**< Used to generate restIds, Increases with each registered plugin */
    AsyncHttpClient        m_client;      /**< Asynchronous HTTP client. */
    Queue<Cmd>             m_cmdQueue;    /**< Command queue */
    TaskProxy<Msg, 9U, 0U> m_taskProxy;   /**< Task proxy used to decouple server responses, which happen in a different task context.*/
    Mutex                  m_mutex;       /**< Used to protect against concurrent access */

    /**
     * Saves filters
     * key: restId of plugin
     * value: filter
     */
    std::map<int, DynamicJsonDocument*> filters;

    /**
     * Constructs the service instance.
     */
    RestService() :
        IService(),
        restIdCounter(0),
        filters(),
        m_client(),
        m_taskProxy(),
        m_cmdQueue(),
        m_mutex()
    {

        (void)m_cmdQueue.create(CMD_QUEUE_SIZE);
        (void)m_mutex.create();

        /* Note: All registered callbacks are running in a different task context!
         *       Therefore it is not allowed to access a member here directly.
         *       The processing must be deferred via task proxy.
         */
        m_client.regOnResponse(
            [this](const int restId, const HttpResponse& rsp) {
                handleAsyncWebResponse(restId, rsp);
            });

        m_client.regOnError(
            [this](const int restId) {
                Msg msg;

                msg.restId = restId;
                msg.isMsg  = false;
                msg.rsp    = nullptr;

                if (false == this->m_taskProxy.send(msg))
                {
                    LOG_ERROR("Msg could not be sent to Msg-Queue");
                }

                this->m_mutex.give();
            });
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
     * Handle asynchronous web response from the server.
     * This will be called in LwIP context! Don't modify any member here directly!
     *
     * @param[in] rsp     Web response as JSON document
     * @param[in] restId  Unique Id to identify plugin
     */
    void handleAsyncWebResponse(const int restId, const HttpResponse& rsp);
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* REST_SERVICE_H */

/** @} */
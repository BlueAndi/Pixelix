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
 * @file   HttpResponseSink.h
 * @brief  HTTP response sink stream
 * @author Andreas Merkle (web@blue-andi.de)
 *
 * @addtogroup HTTP_SERVICE
 *
 * @{
 */

#ifndef HTTP_RESPONSE_SINK_H
#define HTTP_RESPONSE_SINK_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Stream.h>
#include "HttpServiceTypes.h"

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
 * Stream sink used by HTTPClient::writeToStream() to forward decoded payload.
 */
class HttpResponseSink : public Stream
{
public:

    /**
     * Constructs the HTTP response sink.
     *
     * @param[in] handler   Optional handler to process streaming chunks.
     * @param[in] response  Response buffer used if no handler is provided.
     */
    HttpResponseSink(IHttpResponseHandler* handler, WorkerResponse& response);

    /**
     * Destroys the HTTP response sink.
     */
    ~HttpResponseSink();

    /**
     * Write a single byte.
     *
     * @param[in] data  Byte to write.
     *
     * @return Number of bytes written.
     */
    size_t write(uint8_t data) final;

    /**
     * Write a byte buffer.
     *
     * @param[in] buffer    Data buffer.
     * @param[in] size      Buffer size in byte.
     *
     * @return Number of bytes written.
     */
    size_t write(const uint8_t* buffer, size_t size) final;

    /**
     * Return number of bytes available for reading.
     *
     * @return Always 0, this sink is write-only.
     */
    int available() final;

    /**
     * Read one byte.
     *
     * @return Always -1, this sink is write-only.
     */
    int read() final;

    /**
     * Peek one byte.
     *
     * @return Always -1, this sink is write-only.
     */
    int peek() final;

    /**
     * Flush sink.
     */
    void flush() final;

    /**
     * Finalize the stream and notify handler about end of data.
     */
    void finalize();

private:

    IHttpResponseHandler* m_handler;  /**< Optional response handler. */
    WorkerResponse&       m_response; /**< Response buffer for payload accumulation. */
    uint32_t              m_index;    /**< Chunk index. */

    /**
     * Disable default constructor.
     */
    HttpResponseSink()                                        = delete;

    /**
     * Disable copy constructor.
     *
     * @param[in] sink  Sink to copy.
     */
    HttpResponseSink(const HttpResponseSink& sink)            = delete;

    /**
     * Disable assignment operator.
     *
     * @param[in] sink  Sink to assign.
     *
     * @return Assigned sink.
     */
    HttpResponseSink& operator=(const HttpResponseSink& sink) = delete;
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* HTTP_RESPONSE_SINK_H */

/** @} */

/* MIT License
 *
 * Copyright (c) 2019 - 2023 Andreas Merkle <web@blue-andi.de>
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
 * @brief  HTTP status
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup web
 *
 * @{
 */

#ifndef HTTP_STATUS
#define HTTP_STATUS

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

/** HTTP status */
namespace HttpStatus
{

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This type defines supported HTTP response status codes according to RFC7231.
 */
enum StatusCode
{
    STATUS_CODE_CONTINUE = 100,                         /**< Continue */
    STATUS_CODE_SWITCHING_PROTOCOLS = 101,              /**< Switching Protocols */
    STATUS_CODE_PROCESSING = 102,                       /**< Processing */
    STATUS_CODE_OK = 200,                               /**< Ok */
    STATUS_CODE_CREATED = 201,                          /**< Created */
    STATUS_CODE_ACCEPTED = 202,                         /**< Accepted */
    STATUS_CODE_NON_AUTHORITATIVE_INFORMATION = 203,    /**< Non-Authoritative Information */
    STATUS_CODE_NO_CONTENT = 204,                       /**< No Content */
    STATUS_CODE_RESET_CONTENT = 205,                    /**< Reset Content */
    STATUS_CODE_PARTIAL_CONTENT = 206,                  /**< Partial Content */
    STATUS_CODE_MULTI_STATUS = 207,                     /**< Multi-Status */
    STATUS_CODE_ALREADY_REPORTED = 208,                 /**< Already Reported */
    STATUS_CODE_IM_USED = 226,                          /**< IM Used */
    STATUS_CODE_MULTIPLE_CHOICES = 300,                 /**< Multiple Choices */
    STATUS_CODE_MOVED_PERMANENTLY = 301,                /**< Moved Permantently */
    STATUS_CODE_FOUND = 302,                            /**< Found */
    STATUS_CODE_SEE_OTHER = 303,                        /**< See Other */
    STATUS_CODE_NOT_MODIFIED = 304,                     /**< Not Modified */
    STATUS_CODE_USE_PROXY = 305,                        /**< Use Proxy */
    STATUS_CODE_TEMPORARY_REDIRECT = 307,               /**< Temporary Redirect */
    STATUS_CODE_PERMANENT_REDIRECT = 308,               /**< Permanent Redirect */
    STATUS_CODE_BAD_REQUEST = 400,                      /**< Bad Request */
    STATUS_CODE_UNAUTHORIZED = 401,                     /**< Unauthorized */
    STATUS_CODE_PAYMENT_REQUIRED = 402,                 /**< Payment Required */
    STATUS_CODE_FORBIDDEN = 403,                        /**< Forbidden */
    STATUS_CODE_NOT_FOUND = 404,                        /**< Not Found */
    STATUS_CODE_METHOD_NOT_ALLOWED = 405,               /**< Method Not Allowed */
    STATUS_CODE_NOT_ACCEPTABLE = 406,                   /**< Not Acceptable */
    STATUS_CODE_PROXY_AUTHENTICATION_REQUIRED = 407,    /**< Proxy Authentication Required */
    STATUS_CODE_REQUEST_TIMEOUT = 408,                  /**< Request Timeout */
    STATUS_CODE_CONFLICT = 409,                         /**< Conflict */
    STATUS_CODE_GONE = 410,                             /**< Gone */
    STATUS_CODE_LENGTH_REQUIRED = 411,                  /**< Length Required */
    STATUS_CODE_PRECONDITION_FAILED = 412,              /**< Precondition Failed */
    STATUS_CODE_PAYLOAD_TOO_LARGE = 413,                /**< Payload Too Large */
    STATUS_CODE_URI_TOO_LONG = 414,                     /**< URI Too Long */
    STATUS_CODE_UNSUPPORTED_MEDIA_TYPE = 415,           /**< Unsupported Media Type */
    STATUS_CODE_RANGE_NOT_SATISFIABLE = 416,            /**< Range Not Satisfiable */
    STATUS_CODE_EXPECTATION_FAILED = 417,               /**< Expectation Failed */
    STATUS_CODE_MISDIRECTED_REQUEST = 421,              /**< Misdirected Request */
    STATUS_CODE_UNPROCESSABLE_ENTITY = 422,             /**< Unprocessable Entity */
    STATUS_CODE_LOCKED = 423,                           /**< Locked */
    STATUS_CODE_FAILED_DEPENDENCY = 424,                /**< Failed Dependency */
    STATUS_CODE_UPGRADE_REQUIRED = 426,                 /**< Upgrade Required */
    STATUS_CODE_PRECONDITION_REQUIRED = 428,            /**< Precondition Required */
    STATUS_CODE_TOO_MANY_REQUESTS = 429,                /**< Too Many Requests */
    STATUS_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,  /**< Request Header Fields Too Large */
    STATUS_CODE_INTERNAL_SERVER_ERROR = 500,            /**< Internal Server Error */
    STATUS_CODE_NOT_IMPLEMENTED = 501,                  /**< Not Implemented */
    STATUS_CODE_BAD_GATEWAY = 502,                      /**< Bad Gateway */
    STATUS_CODE_SERVICE_UNAVAILABLE = 503,              /**< Service Unavailable */
    STATUS_CODE_GATEWAY_TIMEOUT = 504,                  /**< Gateway Timeout */
    STATUS_CODE_HTTP_VERSION_NOT_SUPPORTED = 505,       /**< Http Version Not Supported */
    STATUS_CODE_VARIANT_ALSO_NEGOTIATES = 506,          /**< Variant Also Negotiates */
    STATUS_CODE_INSUFFICIENT_STORAGE = 507,             /**< Insufficient Storage */
    STATUS_CODE_LOOP_DETECTED = 508,                    /**< Loop Detected */
    STATUS_CODE_NOT_EXTENDED = 510,                     /**< Not Extended */
    STATUS_CODE_NETWORK_AUTHENTICATION_REQUIRED = 511   /**< Network Authentication Required */
};


/******************************************************************************
 * Functions
 *****************************************************************************/

}

#endif  /* HTTP_STATUS */

/** @} */
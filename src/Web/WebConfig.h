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
 * @brief  Webserver configuration
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup web
 *
 * @{
 */

#ifndef WEBCONFIG_H
#define WEBCONFIG_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

/** Webserver configuration constants. */
namespace WebConfig
{

/******************************************************************************
 * Constants
 *****************************************************************************/

/** Web server port */
static const uint32_t   WEBSERVER_PORT          = 80U;

/** Project title, used by the web pages. */
static const char       PROJECT_TITLE[]         = "PIXELIX";

/** Websocket protocol */
static const char       WEBSOCKET_PROTOCOL[]    = "ws";

/** Websocket port */
static const uint32_t   WEBSOCKET_PORT          = 80U;

/** Websocket path */
static const char       WEBSOCKET_PATH[]        = "/ws";

/** Arduino OTA port */
static const uint32_t   ARDUINO_OTA_PORT        = 3232U;

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

}

#endif  /* WEBCONFIG_H */

/** @} */
/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
@brief  Route
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
This module provides web request routing.

*******************************************************************************/
/** @defgroup route Route
 * This module provides a route class, which can be used to bind a web page to
 * a URI. This will be registered by the web server and automatically called,
 * if a client requests it.
 *
 * @{
 */

#ifndef __ROUTE_H__
#define __ROUTE_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <WebServer.h>
#include <stdint.h>
#include "IWebPage.hpp"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This class routes the request to the right web page.
 */
class Route
{
public:

    /**
     * Creates a route for request.
     * 
     * @param[in] srv           Web server, used to retrieve request arguments.
     * @param[in] webPage       The web page, which shall be shown on request.
     * @param[in] authHandler   Optional authentication handler, used for web page access.
     */
    Route(WebServer& srv, IWebPage& webPage, IAuthHandler* authHandler) :
        m_srv(&srv),
        m_webPage(&webPage),
        m_authHandler(authHandler)
    {
    }

    /***
     * Destroys the route.
     */
    virtual ~Route()
    {
    }

protected:

    /**
     * The static callback only knows its slot, but doesn't has
     * access to the non-static method. This method will handle
     * this by knowing the slot.
     */
    static void staticInvoke(uint8_t slot);

private:

    WebServer*      m_srv;          /**< Web server to get request arguments or headers */
    IWebPage*       m_webPage;      /**< Web page to show by request */
    IAuthHandler*   m_authHandler;  /**< Authentication handler, used for web page access */

    Route();
    Route(const Route& route);
    Route& operator=(const Route& route);
};

/**
 * This class provides a static callback, used to be registered by the
 * web server. To avoid automatically create dedicated static callbacks,
 * a slot in a array must be given. The slot represents the index in this
 * array, which is necessary to retrieve the right non-static callback
 * method later.
 */
template < uint8_t slot >
class StaticCallback : public Route
{
public:

    /**
     * Creates a static callback.
     * 
     * @param[in] srv           Web server, used to retrieve request arguments.
     * @param[in] webPage       The web page, which shall be shown on request.
     * @param[in] authHandler   Optional authentication handler, used for web page access.
     */
    StaticCallback(WebServer& srv, const char* path, IWebPage& webPage, IAuthHandler* authHandler) :
        Route(srv, webPage, authHandler)
    {
        srv.on(path, handler);
    }

    /**
     * Destroys the static callback.
     */
    ~StaticCallback()
    {
    }

private:

    StaticCallback();
    StaticCallback(const StaticCallback& cb);
    StaticCallback& operator=(const StaticCallback& cb);

    /**
     * Web server callback, which will be called for a client request.
     */
    static void handler(void)
    {
        staticInvoke(slot);
        return;
    }

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __ROUTE_H__ */

/** @} */
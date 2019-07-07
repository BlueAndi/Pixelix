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
@see Route.h

*******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Route.h"
#include "MyWebServer.hpp"
#include "IndexPage.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Get number of array elements. */
#define ARRAY_NUM(__arr)    (sizeof(__arr) / sizeof((__arr)[0]))

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/**
 * Handle webserver authentication. If the client is not authenticated, an
 * authenticatio process will be performed.
 */
class AuthHandler : public IAuthHandler
{
public:

    /**
     * Constructs the authentication handler.
     */
    AuthHandler() : 
        IAuthHandler()
    {
    }

    /**
     * Destroys the authentication handler.
     */
    ~AuthHandler()
    {
    }

    /**
     * This method will be called to authenticate the client.
     * A web page can request this on demand.
     * 
     * @param[in] srv   Web server
     * 
     * @return Authentication result
     * @retval false    Authentication failed
     * @retval true     Authentication successful
     */
    bool authenticate(WebServer& srv)
    {
        bool status = false;

        /* If there is no authentication with the client, it will be requested. */
        if (false == srv.authenticate(WebConfig::WEB_LOGIN_USER, WebConfig::WEB_LOGIN_PASSWORD))
        {
            const String authFailResponse = "Authentication failed!";

            /* Use encrypted communication for authentication request to avoid
            * that the credentials can be read by everyone.
            */
            srv.requestAuthentication(DIGEST_AUTH, NULL, authFailResponse);
            
            status = false;
        }

        return status;
    }

private:

};

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/**
 * General web authentication handler.
 */
static AuthHandler          gAuthHandler;

/**
 * Route "/" request to index.html
 */
static StaticCallback<0u>   gRouteToIndexPage(  MyWebServer::srv,
                                                "/",
                                                IndexPage::getInstance(),
                                                &gAuthHandler);

/**
 * All registered routes.
 * Please be aware to insert the right route at the right slot.
 */
static Route*               gRoutes[] =
{
    &gRouteToIndexPage
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

void Route::staticInvoke(uint8_t slot)
{
    if (ARRAY_NUM(gRoutes) > slot)
    {
        Route*  route = gRoutes[slot];

        route->m_webPage->show(*(route->m_srv), route->m_authHandler);
    }

    return;
}

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

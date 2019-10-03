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
 * @brief  Web pages
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Pages.h"
#include "Html.h"
#include "WebConfig.h"

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

static bool authenticate(WebServer& srv);
static void errorNotFound(void);
static void indexPage(void);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/**
 * Web server
 */
static WebServer*   gWebServer  = NULL;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

void Pages::init(WebServer& srv)
{
    gWebServer = &srv;

    gWebServer->onNotFound(errorNotFound);
    gWebServer->on("/", HTTP_GET, indexPage);

    return;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * This function will be called to authenticate the client.
 * A web page can request this on demand.
 * 
 * @param[in] srv   Web server
 * 
 * @return Authentication result
 * @retval false    Authentication failed
 * @retval true     Authentication successful
 */
static bool authenticate(WebServer& srv)
{
    bool status = true;

    /* If there is no authentication with the client, it will be requested. */
    if (false == srv.authenticate(WebConfig::WEB_LOGIN_USER, WebConfig::WEB_LOGIN_PASSWORD))
    {
        const String authFailResponse = "Authentication failed!";

        /* Use basic authentication, although no secure transport layer (https) is used
         * right now. This just for simplicity.
         * 
         * Digest Authentication communicates credentials in an encrypted form by applying
         * a hash function to the the username, the password, a server supplied nonce value,
         * the HTTP method, and the requested URI.
         * 
         * TODO Use DIGEST_AUTH
         */
        srv.requestAuthentication(BASIC_AUTH, NULL, authFailResponse);
        
        status = false;
    }

    return status;
}

/**
 * Error web page used in case a requested path was not found.
 */
static void errorNotFound(void)
{
    String page;

    if (NULL == gWebServer)
    {
        return;
    }

    page  = Html::htmlHead(WebConfig::PROJECT_TITLE);
    page += Html::heading(WebConfig::PROJECT_TITLE, 1);
    page += Html::heading("Error", 2);
    page += Html::paragraph("Requested path not found.");
    page += Html::htmlTail();

    gWebServer->send(Html::STATUS_CODE_NOT_FOUND, "text/html", page);

    return;
}

/**
 * Index page on root path ("/").
 */
static void indexPage(void)
{
    String page;

    /* If authentication fails, a error page is automatically shown. */
    if (false == authenticate(*gWebServer))
    {
        return;
    }
    
    page  = Html::htmlHead(WebConfig::PROJECT_TITLE);
    page += Html::heading(WebConfig::PROJECT_TITLE, 1);
    page += Html::paragraph("Root directory.");
    page += Html::htmlTail();

    gWebServer->send(Html::STATUS_CODE_OK, "text/html", page);

    return;
}
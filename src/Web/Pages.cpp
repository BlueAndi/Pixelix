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
#include "Settings.h"

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
static void savedPage(void);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Web server */
static WebServer*       gWebServer                  = NULL;

/** Name of the input field for wifi SSID. */
static const char*      FORM_INPUT_NAME_SSID        = "ssid";

/** Name of the input field for wifi passphrase. */
static const char*      FORM_INPUT_NAME_PASSPHRASE  = "passphrase";

/** Min. wifi SSID length */
static const uint8_t    MIN_SSID_LENGTH             = 0u;

/** Max. wifi SSID length */
static const uint8_t    MAX_SSID_LENGTH             = 32u;

/** Min. wifi passphrase length */
static const uint8_t    MIN_PASSPHRASE_LENGTH       = 8u;

/** Max. wifi passphrase length */
static const uint8_t    MAX_PASSPHRASE_LENGTH       = 64u;

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
    gWebServer->on("/", HTTP_POST, savedPage);

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

    Settings::getInstance().open(true);
    
    page  = Html::htmlHead(WebConfig::PROJECT_TITLE);
    page += Html::heading(WebConfig::PROJECT_TITLE, 1);
    page += Html::heading("Wifi Settings", 2);
    page += Html::form(
        String("SSID:") +
        Html::nextLine() +
        Html::inputText(FORM_INPUT_NAME_SSID, Settings::getInstance().getWifiSSID(), MAX_SSID_LENGTH, MIN_SSID_LENGTH, MAX_SSID_LENGTH) +
        Html::nextLine() +
        "Passphrase" +
        Html::nextLine() +
        Html::inputText(FORM_INPUT_NAME_PASSPHRASE, Settings::getInstance().getWifiPassphrase(), MAX_PASSPHRASE_LENGTH, MIN_PASSPHRASE_LENGTH, MAX_PASSPHRASE_LENGTH) +
        Html::nextLine(),
        "#"
    );
    page += Html::htmlTail();

    Settings::getInstance().close();

    gWebServer->send(Html::STATUS_CODE_OK, "text/html", page);

    return;
}

/**
 * Page on root path ("/"), which is shown if the user submits
 * settings.
 */
static void savedPage(void)
{
    String  page;
    bool    isError     = false;
    String  errorMsg    = "Error: ";
    String  ssid;
    String  passphrase;

    /* If authentication fails, a error page is automatically shown. */
    if (false == authenticate(*gWebServer))
    {
        return;
    }

    /* Check for the necessary arguments. */
    if (false == gWebServer->hasArg(FORM_INPUT_NAME_SSID))
    {
        isError = true;
        errorMsg += "SSID missing.\r\n";
    }

    if (false == gWebServer->hasArg(FORM_INPUT_NAME_PASSPHRASE))
    {
        isError = true;
        errorMsg += "Passphrase missing.\r\n";
    }

    /* Arguments are available */
    if (false == isError)
    {
        ssid        = gWebServer->arg(FORM_INPUT_NAME_SSID);
        passphrase  = gWebServer->arg(FORM_INPUT_NAME_PASSPHRASE);

        /* Check arguments min. and max. lengths */
        if (MIN_SSID_LENGTH > ssid.length())
        {
            isError = true;
            errorMsg += "SSID too short";
        }
        else if (MAX_SSID_LENGTH < ssid.length())
        {
            isError = true;
            errorMsg += "SSID too long";
        }

        if (MIN_PASSPHRASE_LENGTH > passphrase.length())
        {
            isError = true;
            errorMsg += "Passphrase too short";
        }
        else if (MAX_PASSPHRASE_LENGTH < passphrase.length())
        {
            isError = true;
            errorMsg += "Passphrase too long";
        }

        /* Arguments are valid, store them. */
        if (false == isError)
        {
            Settings::getInstance().open(false);
            Settings::getInstance().setWifiSSID(ssid);
            Settings::getInstance().setWifiPassphrase(passphrase);
            Settings::getInstance().close();
        }
    }

    page  = Html::htmlHead(WebConfig::PROJECT_TITLE);
    page += Html::heading(WebConfig::PROJECT_TITLE, 1);
    if (true == isError)
    {
        page += Html::paragraph(errorMsg);
    }
    else
    {
        page += Html::paragraph("Settings saved.");
    }
    page += Html::paragraph(
        Html::hyperlink("/", "Back.")
    );
    page += Html::htmlTail();

    gWebServer->send(Html::STATUS_CODE_OK, "text/html", page);

    return;
}

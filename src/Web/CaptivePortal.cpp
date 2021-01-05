/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Captive portal web page
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "CaptivePortal.h"
#include "HttpStatus.h"
#include "WebConfig.h"
#include "Settings.h"

#include <SPIFFS.h>
#include <Util.h>
#include <Logging.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

static void reqRestart();

/**
 * Captive portal request handler.
 */
class CaptiveRequestHandler : public AsyncWebHandler
{
public:

    /**
     * Constructs the captive portal request handler.
     */
    CaptiveRequestHandler()
    {
    }

    /**
     * Destroys the captive portal request handler.
     */
    ~CaptiveRequestHandler()
    {
    }

    /**
     * Checks whether the request can be handled.
     *
     * @param[in] request   Web request
     *
     * @return If request can be handled, it will return true otherwise false.
     */
    bool canHandle(AsyncWebServerRequest* request) final
    {
        /* The captive portal handles every request. */
        return true;
    }

    /**
     * Handles the request.
     *
     * @param[in] request   Web request, which to handle.
     */
    void handleRequest(AsyncWebServerRequest* request) final
    {
        if (nullptr == request)
        {
            return;
        }

        /* Force authentication! */
        if (false == request->authenticate(WebConfig::WEB_LOGIN_USER, WebConfig::WEB_LOGIN_PASSWORD))
        {
            /* Request DIGEST authentication */
            request->requestAuthentication();
            return;
        }

        if (HTTP_POST == request->method())
        {
            if ((true == request->hasArg("ssid")) &&
                (true == request->hasArg("passphrase")))
            {
                Settings&   settings = Settings::getInstance();

                if (true == settings.open(false))
                {
                    const String& ssid              = request->arg("ssid");
                    const String& passphrase        = request->arg("passphrase");
                    KeyValueString& kvSSID          = settings.getWifiSSID();
                    KeyValueString& kvPassphrase    = settings.getWifiPassphrase();

                    kvSSID.setValue(ssid);
                    kvPassphrase.setValue(passphrase);

                    settings.close();

                    request->send(HttpStatus::STATUS_CODE_OK, "plain/text", "Ok.");
                }
                else
                {
                    request->send(HttpStatus::STATUS_CODE_OK, "plain/text", "Failed.");
                }
            }
            else if ((true == request->hasArg("restart")) &&
                    (0 != request->arg("restart").equals("now")))
            {
                /* Restart after client is disconnected. */
                request->onDisconnect(reqRestart);
                request->send(HttpStatus::STATUS_CODE_OK, "plain/text", "Restarting ...");
            }
            else
            {
                request->send(HttpStatus::STATUS_CODE_OK, "plain/text", "Request invalid.");
            }
        }
        else if (HTTP_GET == request->method())
        {
            request->send(SPIFFS, "/cp/captivePortal.html", "text/html", false, captivePortalPageProcessor);
        }
        else
        {
            request->send(HttpStatus::STATUS_CODE_BAD_REQUEST, "plain/text", "Error");
        }
    }

    /**
     * Non-trivial handler.
     * This is important to control the HTTP body parsing. In case of a non-trivial
     * handler, the webserver will parse the body and provides encoded parameters to
     * the request handler.
     */
    bool isRequestHandlerTrivial() final
    {
        return false;
    }

private:

    /**
     * Processor for index page template.
     * It is responsible for the data binding.
     *
     * @param[in] var   Name of variable in the template
     */
    static String captivePortalPageProcessor(const String& var)
    {
        String  result = var;

        if (var == "SSID")
        {
            if (true == Settings::getInstance().open(true))
            {
                result = Settings::getInstance().getWifiSSID().getValue();
                Settings::getInstance().close();
            }
        }
        else if (var == "PASSPHRASE")
        {
            if (true == Settings::getInstance().open(true))
            {
                result = Settings::getInstance().getWifiPassphrase().getValue();
                Settings::getInstance().close();
            }
        }
        else
        {
            ;
        }

        return result;
    }

};

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/**
 * Captive portal requst handler.
 */
static CaptiveRequestHandler    gCaptivePortalReqHandler;

/**
 * Flag to request a restart.
 */
static bool                     gIsRestartRequested         = false;

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

void CaptivePortal::init(AsyncWebServer& srv)
{
    /* Serve files with static content with enabled cache control.
     * The client may cache files from filesytem for 1 hour.
     */
    (void)srv.serveStatic("/favicon.png", SPIFFS, "/favicon.png", "max-age=3600");
    (void)srv.serveStatic("/images/", SPIFFS, "/images/", "max-age=3600");
    (void)srv.serveStatic("/js/", SPIFFS, "/js/", "max-age=3600");
    (void)srv.serveStatic("/style/", SPIFFS, "/style/", "max-age=3600");

    /* Add the captive portal request handler at last, because it will handle everything. */
    (void)srv.addHandler(&gCaptivePortalReqHandler).setFilter(ON_AP_FILTER);

    return;
}

bool CaptivePortal::isRestartRequested()
{
    return gIsRestartRequested;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Request restart.
 */
static void reqRestart()
{
    gIsRestartRequested = true;
}

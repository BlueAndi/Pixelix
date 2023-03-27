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
 * @brief  Captive portal request handler
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "CaptivePortalHandler.h"
#include "HttpStatus.h"
#include "FileSystem.h"

#include <SettingsService.h>

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

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void CaptivePortalHandler::handleRequest(AsyncWebServerRequest* request)
{
    if (nullptr == request)
    {
        return;
    }

    if (HTTP_POST == request->method())
    {
        if ((true == request->hasArg("ssid")) &&
            (true == request->hasArg("passphrase")))
        {
            SettingsService&    settings    = SettingsService::getInstance();

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
            if (nullptr == m_resetReqHandler)
            {
                request->send(HttpStatus::STATUS_CODE_INTERNAL_SERVER_ERROR, "plain/text", "Internal error.");
            }
            else
            {
                /* Restart after client is disconnected. */
                request->onDisconnect(m_resetReqHandler);
                request->send(HttpStatus::STATUS_CODE_OK, "plain/text", "Restarting ...");
            }
        }
        else
        {
            request->send(HttpStatus::STATUS_CODE_OK, "plain/text", "Request invalid.");
        }
    }
    else if (HTTP_GET == request->method())
    {
        request->send(FILESYSTEM, "/cp/captivePortal.html", "text/html", false, captivePortalPageProcessor);
    }
    else
    {
        request->send(HttpStatus::STATUS_CODE_BAD_REQUEST, "plain/text", "Error");
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

String CaptivePortalHandler::captivePortalPageProcessor(const String& var)
{
    String  result = var;

    if (var == "SSID")
    {
        SettingsService&    settings    = SettingsService::getInstance();

        if (true == settings.open(true))
        {
            result = settings.getWifiSSID().getValue();
            settings.close();
        }
    }
    else if (var == "PASSPHRASE")
    {
        SettingsService&    settings    = SettingsService::getInstance();

        if (true == settings.open(true))
        {
            result = settings.getWifiPassphrase().getValue();
            settings.close();
        }
    }
    else if (var == "MAC_ADDR")
    {
        result = WiFi.macAddress();
    }
    else
    {
        ;
    }

    return result;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

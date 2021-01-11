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
#include "CaptivePortalHandler.h"

#include <SPIFFS.h>

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

static void reqRestart();

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/**
 * Captive portal requst handler.
 */
static CaptivePortalHandler     gCaptivePortalReqHandler(reqRestart);

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
     * The client may cache files from filesystem for 1 hour.
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

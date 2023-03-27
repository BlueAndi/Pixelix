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
 * @brief  Captive portal web page
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "CaptivePortal.h"
#include "HttpStatus.h"
#include "CaptivePortalHandler.h"
#include "FileSystem.h"

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
    /* Don't use authentication, because this has a bad influence on some
     * mobile devices with special connectivity checkers.
     * Not nice from a security point of view, but because the captive portal
     * is only active in WiFi access point mode and this must be started
     * manually, we keep one eye closed here.
     */

    /* Serve files with static content with enabled cache control.
     * The client may cache files from filesystem for 1 hour.
     */
    (void)srv.serveStatic("/favicon.png", FILESYSTEM, "/favicon.png", "max-age=3600");
    (void)srv.serveStatic("/images/", FILESYSTEM, "/images/", "max-age=3600");
    (void)srv.serveStatic("/js/", FILESYSTEM, "/js/", "max-age=3600");
    (void)srv.serveStatic("/style/", FILESYSTEM, "/style/", "max-age=3600");

    /* The about dialog is the only additional page, which shall be accessible. */
    (void)srv.serveStatic("/about.html", FILESYSTEM, "/about.html");

    /* Add the captive portal request handler at last, because it will handle everything else. */
    (void)srv.addHandler(&gCaptivePortalReqHandler)
        .setFilter(ON_AP_FILTER);
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

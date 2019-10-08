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
#include "Version.h"

#include <WiFi.h>
#include <Esp.h>
#include <SPIFFS.h>

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
 * One menu item.
 */
typedef struct
{
    const char* name;   /**< Menu item name */
    const char* href;   /**< Menu item hyperlink */

} MenuItem;

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static String getProjectTitle(void);
static void addHeader(String& body);
static void addTopNav(String& body, uint8_t active);
static void addFooter(String& body);
static void errorNotFound(AsyncWebServerRequest* request);
static void indexPage(AsyncWebServerRequest* request);
static void networkPage(AsyncWebServerRequest* request);
static void settingsPage(AsyncWebServerRequest* request);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Web server */
static AsyncWebServer*  gWebServer                  = NULL;

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

/** Top navigation menu items */
static MenuItem         gTopNavItems[] =
{
    { "Home",       "/"         },
    { "Network",    "/network"  },
    { "Settings",   "/settings" }
};

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

void Pages::init(AsyncWebServer& srv)
{
    gWebServer = &srv;

    gWebServer->onNotFound(errorNotFound);
    gWebServer->on("/", HTTP_GET, indexPage);
    gWebServer->on("/network", HTTP_GET, networkPage);
    gWebServer->on("/settings", HTTP_GET, settingsPage);
    gWebServer->on("/settings", HTTP_POST, settingsPage);

    /* Serve files from filesystem */
    gWebServer->serveStatic("/data/style.css", SPIFFS, "/style.css");
    gWebServer->serveStatic("/data/util.js", SPIFFS, "/utils.js");

    return;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Get project title in color form.
 * 
 * @return Project title in color form (HTML).
 */
static String getProjectTitle(void)
{
    String      title;
    uint8_t     projectTitleLen = strlen(WebConfig::PROJECT_TITLE);
    uint8_t     index           = 0;
    uint8_t     colorIndex  = 0;
    const char* colors[]    =
    {
        "#FF0000",
        "#FFFF00",
        "#00FF00",
        "#00FFFF",
        "#0000FF",
        "#FF00FF"
    };

    title += ".:";
    for(index = 0; index < projectTitleLen; ++index)
    {
        title += "<span style=\"color:";
        title += colors[colorIndex];
        title += "\">";
        title += WebConfig::PROJECT_TITLE[index];
        title += "</span>";

        ++colorIndex;
        if (ARRAY_NUM(colors) <= colorIndex)
        {
            colorIndex = 0;
        }
    }
    title += ":.";

    return title;
}

/**
 * Add HTML page header to body.
 * 
 * @param[inout] body   Body
 */
static void addHeader(String& body)
{
    body += "<div class=\"header\">\r\n";
    body += "\t<h1>";
    body += getProjectTitle();
    body += "</h1>\r\n";
    body += "</div>\r\n";

    return;
}

/**
 * Add HTML top navigation to body.
 * 
 * @param[inout]    body   Body
 * @param[in]       active Active menu item [0; ...]
 */
static void addTopNav(String& body, uint8_t active)
{
    uint8_t index = 0;

    body += "<div class=\"topnav\">\r\n";

    for(index = 0; index < ARRAY_NUM(gTopNavItems); ++index)
    {
        body += "\t<a class=\"";
        if (active == index)
        {
            body += "active";
        }
        body += "\" href=\"";
        body += gTopNavItems[index].href;
        body += "\">";
        body += gTopNavItems[index].name;
        body += "</a>\r\n";
    }

    body += "</div>\r\n";

    return;
}

/**
 * Add HTML footer to body.
 * 
 * @param[inout]    body   Body
 */
static void addFooter(String& body)
{
    body += "<div class=\"footer\">\r\n";
    body += "\t<hr />\r\n";
    body += "\t(C) 2019 by Andreas Merkle (web@blue-andi.de)\r\n";
    body += "</div>\r\n";

    return;
}

/**
 * Error web page used in case a requested path was not found.
 * 
 * @param[in] request   HTTP request
 */
static void errorNotFound(AsyncWebServerRequest* request)
{
    String      body;
    Html::Page  page(WebConfig::PROJECT_TITLE);

    if (NULL == request)
    {
        return;
    }

    addHeader(body);
    body += Html::heading("Error", 2);
    body += Html::paragraph("Requested path not found.");
    addFooter(body);

    page.setBody(body);

    request->send(Html::STATUS_CODE_NOT_FOUND, "text/html", page.toString());

    return;
}

/**
 * Index page on root path ("/").
 * 
 * @param[in] request   HTTP request
 */
static void indexPage(AsyncWebServerRequest* request)
{
    String      body;
    Html::Page  page(WebConfig::PROJECT_TITLE);
    String      ssid;
    String      passphrase;

    if (NULL == request)
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

    addHeader(body);
    addTopNav(body, 0);
    body += "<div class=\"main\">\r\n";
    body += "\t<h2>Home</h2>\r\n";
    body += "\t<p>Welcome!</p>\r\n";

    body += "\t<h3>Software</h3>\r\n";
    body += "\t<table>";

    body += "\t\t<tr>\r\n";
    body += "\t\t\t<td><strong>Version:</strong></td>\r\n";
    body += "\t\t\t<td>";
    body += Version::SOFTWARE;
    body += "</td>\r\n";
    body += "\t\t</tr>\r\n";

    body += "\t\t<tr>\r\n";
    body += "\t\t\t<td><strong>ESP SDK Version:</strong></td>\r\n";
    body += "\t\t\t<td>";
    body += ESP.getSdkVersion();
    body += "</td>\r\n";
    body += "\t\t</tr>\r\n";

    body += "\t\t<tr>\r\n";
    body += "\t\t\t<td><strong>Heap size:</strong></td>\r\n";
    body += "\t\t\t<td>";
    body += ESP.getHeapSize();
    body += " byte</td>\r\n";
    body += "\t\t</tr>\r\n";

    body += "\t\t<tr>\r\n";
    body += "\t\t\t<td><strong>Available heap size:</strong></td>\r\n";
    body += "\t\t\t<td>";
    body += ESP.getFreeHeap();
    body += " byte</td>\r\n";
    body += "\t\t</tr>\r\n";

    body += "\t</table>";

    body += "\t<h3>Hardware</h3>\r\n";
    body += "\t<table>";

    body += "\t\t<tr>\r\n";
    body += "\t\t\t<td><strong>ESP chip rev.:</strong></td>\r\n";
    body += "\t\t\t<td>";
    body += ESP.getChipRevision();
    body += "</td>\r\n";
    body += "\t\t</tr>\r\n";
    
    body += "\t\t<tr>\r\n";
    body += "\t\t\t<td><strong>ESP cpu freq.:</strong></td>\r\n";
    body += "\t\t\t<td>";
    body += ESP.getCpuFreqMHz();
    body += " MHz</td>\r\n";
    body += "\t\t</tr>\r\n";

    body += "\t</table>";
    body += "</div>\r\n";
    addFooter(body);

    page.setBody(body);

    request->send(Html::STATUS_CODE_OK, "text/html", page.toString());

    return;
}

/**
 * Network page, shows all information regarding the network.
 * 
 * @param[in] request   HTTP request
 */
static void networkPage(AsyncWebServerRequest* request)
{
    String      body;
    Html::Page  page(WebConfig::PROJECT_TITLE);
    String      ssid;

    if (NULL == request)
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

    if (true == Settings::getInstance().open(true))
    {
        ssid = Settings::getInstance().getWifiSSID();
        Settings::getInstance().close();
    }

    addHeader(body);
    addTopNav(body, 1);
    body += "<div class=\"main\">\r\n";
    body += "\t<h2>Network</h2>\r\n";
    body += "\t<table>";

    body += "\t\t<tr>\r\n";
    body += "\t\t\t<td><strong>SSID:</strong></td>\r\n";
    body += "\t\t\t<td>";
    body += ssid;
    body += "</td>\r\n";
    body += "\t\t</tr>\r\n";

    body += "\t\t<tr>\r\n";
    body += "\t\t\t<td><strong>RSSI:</strong></td>\r\n";
    body += "\t\t\t<td>";
    body += WiFi.RSSI();
    body += " dBm</td>\r\n";
    body += "\t\t</tr>\r\n";

    body += "\t\t<tr>\r\n";
    body += "\t\t\t<td><strong>Hostname:</strong></td>\r\n";
    body += "\t\t\t<td>";
    body += WiFi.getHostname();
    body += "</td>\r\n";
    body += "\t\t</tr>\r\n";

    body += "\t\t<tr>\r\n";
    body += "\t\t\t<td><strong>IPv4:</strong></td>\r\n";
    body += "\t\t\t<td>";
    body += WiFi.localIP().toString();
    body += "</td>\r\n";
    body += "\t\t</tr>\r\n";

    body += "\t</table>";
    body += "</div>\r\n";
    addFooter(body);

    page.setBody(body);

    request->send(Html::STATUS_CODE_OK, "text/html", page.toString());

    return;
}

/**
 * Settings page to show and store settings.
 * 
 * @param[in] request   HTTP request
 */
static void settingsPage(AsyncWebServerRequest* request)
{
    String      body;
    Html::Page  page(WebConfig::PROJECT_TITLE);
    bool        isError     = false;
    String      errorMsg;
    String      ssid;
    String      passphrase;
    const char* style       =
    ".error {"                          \
        "padding: 20px;"                \
        "background-color: #f44336;"    \
        "color: white;"                 \
    "}"                                 \
    ".success {"                        \
        "padding: 20px;"                \
        "background-color: #4CAF50;"    \
        "color: white;"                 \
    "}"                                 \
    ".closebtn {"                       \
        "margin-left: 15px;"            \
        "color: white;"                 \
        "font-weight: bold;"            \
        "float: right;"                 \
        "font-size: 22px;"              \
        "line-height: 20px;"            \
        "cursor: pointer;"              \
        "transition: 0.3s;"             \
    "}"                                 \
    ".closebtn:hover {"                 \
        "color: black;"                 \
    "}";

    if (NULL == request)
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

    /* Store settings? */
    if (0 < request->args())
    {
        /* Check for the necessary arguments. */
        if (false == request->hasArg(FORM_INPUT_NAME_SSID))
        {
            isError = true;
            errorMsg += "<p>SSID missing.</p>\r\n";
        }

        if (false == request->hasArg(FORM_INPUT_NAME_PASSPHRASE))
        {
            isError = true;
            errorMsg += "<p>Passphrase missing.</p>\r\n";
        }

        /* Arguments are available */
        if (false == isError)
        {
            ssid        = request->arg(FORM_INPUT_NAME_SSID);
            passphrase  = request->arg(FORM_INPUT_NAME_PASSPHRASE);

            /* Check arguments min. and max. lengths */
            if (MIN_SSID_LENGTH > ssid.length())
            {
                isError = true;
                errorMsg += "<p>SSID too short.</p>";
            }
            else if (MAX_SSID_LENGTH < ssid.length())
            {
                isError = true;
                errorMsg += "<p>SSID too long.</p>";
            }

            if (MIN_PASSPHRASE_LENGTH > passphrase.length())
            {
                isError = true;
                errorMsg += "<p>Passphrase too short.</p>";
            }
            else if (MAX_PASSPHRASE_LENGTH < passphrase.length())
            {
                isError = true;
                errorMsg += "<p>Passphrase too long.</p>";
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
    }

    Settings::getInstance().open(true);
    ssid        = Settings::getInstance().getWifiSSID();
    passphrase  = Settings::getInstance().getWifiPassphrase();
    Settings::getInstance().close();

    addHeader(body);
    addTopNav(body, 2);
    body += "<div class=\"main\">\r\n";
    body += "\t<h2>Wifi Settings</h2>";

    if (0 < request->args())
    {
        if (false == isError)
        {
            body += "\t<div class=\"success\">";
            body += "\t\t<span class=\"closebtn\" onclick=\"this.parentElement.style.display='none';\">&times;</span>";
            body += "\t\t<strong>Info!</strong> Settings successful stored.";
            body += "\t</div>";
        }
        else
        {
            body += "\t<div class=\"error\">";
            body += "\t\t<span class=\"closebtn\" onclick=\"this.parentElement.style.display='none';\">&times;</span>";
            body += "\t\t<strong>Error!</strong> ";
            body += errorMsg;
            body += "\t</div>";
        }
    }

    body += Html::form(
        String("SSID:") +
        Html::nextLine() +
        Html::inputText(FORM_INPUT_NAME_SSID, ssid, MAX_SSID_LENGTH, MIN_SSID_LENGTH, MAX_SSID_LENGTH) +
        Html::nextLine() +
        "Passphrase:" +
        Html::nextLine() +
        Html::inputText(FORM_INPUT_NAME_PASSPHRASE, passphrase, MAX_PASSPHRASE_LENGTH, MIN_PASSPHRASE_LENGTH, MAX_PASSPHRASE_LENGTH) +
        Html::nextLine(),
        "#"
    );

    body += "</div>\r\n";
    addFooter(body);

    page.setBody(body);
    page.setStyle(style);

    request->send(Html::STATUS_CODE_OK, "text/html", page.toString());

    return;
}

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

static bool authenticate(WebServer& srv);
static String getProjectTitle(void);
static void addHeader(String& body);
static void addTopNav(String& body, uint8_t active);
static void addFooter(String& body);
static void errorNotFound(void);
static void indexPage(void);
static void networkPage(void);
static void settingsPage(void);

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

/** CSS used for every page. */
static const char*      WEB_PAGE_STYLE              =
    "body {" \
        "color: white;" \
        "background-color: #202020;" \
    "}" \
    "h2 {" \
        "color: #ff0000;" \
    "}" \
    "h3 {" \
        "color: #ffff00;" \
    "}" \
    "table {" \
        "border-collapse: collapse;" \
    "}" \
    "tr:nth-child(even) {" \
        "background-color: #333;" \
    "}" \
    "td {" \
        "border: 1px solid white;" \
        "padding: 2px" \
    "}" \
    ".header {" \
        "padding: 10px 16px;" \
    "}" \
    ".topnav {" \
        "background-color: #333;" \
        "overflow: hidden;" \
    "}" \
    ".topnav a {" \
        "float: left;" \
        "color: #f2f2f2;" \
        "text-align: center;" \
        "padding: 14px 16px;" \
        "text-decoration: none;" \
        "font-size: 17px;" \
    "}" \
    ".topnav a:hover {" \
        "background-color: #ddd;" \
        "color: black;" \
    "}" \
    ".topnav a.active {" \
        "background-color: #4CAF50;" \
        "color: white;" \
    "}" \
    ".main {" \
        "padding: 16px;" \
    "}" \
    ".footer {" \
        "padding: 10px 16px;" \
        "text-align: center;" \
    "}";

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

void Pages::init(WebServer& srv)
{
    gWebServer = &srv;

    gWebServer->onNotFound(errorNotFound);
    gWebServer->on("/", HTTP_GET, indexPage);
    gWebServer->on("/network", HTTP_GET, networkPage);
    gWebServer->on("/settings", HTTP_GET, settingsPage);
    gWebServer->on("/settings", HTTP_POST, settingsPage);

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
 */
static void errorNotFound(void)
{
    String      body;
    Html::Page  page(WebConfig::PROJECT_TITLE);

    if (NULL == gWebServer)
    {
        return;
    }

    addHeader(body);
    body += Html::heading("Error", 2);
    body += Html::paragraph("Requested path not found.");
    addFooter(body);

    page.setBody(body);
    page.setStyle(WEB_PAGE_STYLE);

    gWebServer->send(Html::STATUS_CODE_NOT_FOUND, "text/html", page.toString());

    return;
}

/**
 * Index page on root path ("/").
 */
static void indexPage(void)
{
    String      body;
    Html::Page  page(WebConfig::PROJECT_TITLE);
    String      ssid;
    String      passphrase;

    /* If authentication fails, a error page is automatically shown. */
    if (false == authenticate(*gWebServer))
    {
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
    page.setStyle(WEB_PAGE_STYLE);

    gWebServer->send(Html::STATUS_CODE_OK, "text/html", page.toString());

    return;
}

/**
 * Network page, shows all information regarding the network.
 */
static void networkPage(void)
{
    String      body;
    Html::Page  page(WebConfig::PROJECT_TITLE);
    String      ssid;

    /* If authentication fails, a error page is automatically shown. */
    if (false == authenticate(*gWebServer))
    {
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
    page.setStyle(WEB_PAGE_STYLE);

    gWebServer->send(Html::STATUS_CODE_OK, "text/html", page.toString());

    return;
}

/**
 * Settings page to show and store settings.
 */
static void settingsPage(void)
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

    /* If authentication fails, a error page is automatically shown. */
    if (false == authenticate(*gWebServer))
    {
        return;
    }

    /* Store settings? */
    if (0 < gWebServer->args())
    {
        /* Check for the necessary arguments. */
        if (false == gWebServer->hasArg(FORM_INPUT_NAME_SSID))
        {
            isError = true;
            errorMsg += "<p>SSID missing.</p>\r\n";
        }

        if (false == gWebServer->hasArg(FORM_INPUT_NAME_PASSPHRASE))
        {
            isError = true;
            errorMsg += "<p>Passphrase missing.</p>\r\n";
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

    if (0 < gWebServer->args())
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
        "Passphrase" +
        Html::nextLine() +
        Html::inputText(FORM_INPUT_NAME_PASSPHRASE, passphrase, MAX_PASSPHRASE_LENGTH, MIN_PASSPHRASE_LENGTH, MAX_PASSPHRASE_LENGTH) +
        Html::nextLine(),
        "#"
    );

    body += "</div>\r\n";
    addFooter(body);

    page.setBody(body);
    page.setStyle(String(WEB_PAGE_STYLE) + style);

    gWebServer->send(Html::STATUS_CODE_OK, "text/html", page.toString());

    return;
}

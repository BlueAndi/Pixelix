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
#include "HttpStatus.h"
#include "WebConfig.h"
#include "Settings.h"
#include "Version.h"
#include "UpdateMgr.h"
#include "LedMatrix.h"
#include "DisplayMgr.h"

#include <WiFi.h>
#include <Esp.h>
#include <SPIFFS.h>
#include <Update.h>
#include <Logging.h>

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

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static bool isValidHostname(const String& hostname);
static bool isValidSSID(const String& ssid);
static String getColoredText(const String& text);

static String commonPageProcessor(const String& var);

static void errorPage(AsyncWebServerRequest* request);
static String errorPageProcessor(const String& var);

static void indexPage(AsyncWebServerRequest* request);
static String indexPageProcessor(const String& var);

static void networkPage(AsyncWebServerRequest* request);
static String networkPageProcessor(const String& var);

static void configPage(AsyncWebServerRequest* request);
static String configPageProcessor(const String& var);

static void addSettingsData(String& dst, const char* title, const char* name, const char* value, uint8_t size, uint8_t minLen, uint8_t maxLen);
static void settingsPage(AsyncWebServerRequest* request);
static String settingsPageProcessor(const String& var);

static void devPage(AsyncWebServerRequest* request);
static String devPageProcessor(const String& var);

static void updatePage(AsyncWebServerRequest* request);
static String updatePageProcessor(const String& var);

static void uploadPage(AsyncWebServerRequest* request);
static void uploadHandler(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);

static void displayPage(AsyncWebServerRequest* request);
static String displayPageProcessor(const String& var);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Name of the input field for wifi SSID. */
static const char*      FORM_INPUT_NAME_WIFI_SSID           = "wifi_ssid";

/** Name of the input field for wifi passphrase. */
static const char*      FORM_INPUT_NAME_WIFI_PASSPHRASE     = "wifi_passphrase";

/** Name of the input field for wifi access point SSID. */
static const char*      FORM_INPUT_NAME_WIFI_AP_SSID        = "wifi_ap_ssid";

/** Name of the input field for wifi access point passphrase. */
static const char*      FORM_INPUT_NAME_WIFI_AP_PASSPHRASE  = "wifi_ap_passphrase";

/** Name of the input field for hostname. */
static const char*      FORM_INPUT_NAME_HOSTNAME            = "hostname";

/** Min. wifi SSID length. Section 7.3.2.1 of the 802.11-2007 specification. */
static const uint8_t    MIN_SSID_LENGTH             = 0u;

/** Max. wifi SSID length. Section 7.3.2.1 of the 802.11-2007 specification. */
static const uint8_t    MAX_SSID_LENGTH             = 32u;

/** Min. wifi passphrase length */
static const uint8_t    MIN_PASSPHRASE_LENGTH       = 8u;

/** Max. wifi passphrase length */
static const uint8_t    MAX_PASSPHRASE_LENGTH       = 64u;

/** Min. hostname length (RFC1034 1 - 63) */
static const uint8_t    MIN_HOSTNAME_LENGTH         = 1u;

/** Max. hostname length (RFC1034 1 - 63) */
static const uint8_t    MAX_HOSTNAME_LENGTH         = 63u;

/** Firmware binary filename, used for update. */
static const char*      FIRMWARE_FILENAME           = "firmware.bin";

/** Filesystem binary filename, used for update. */
static const char*      FILESYSTEM_FILENAME         = "spiffs.bin";

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
    srv.onNotFound(errorPage);
    srv.on("/", HTTP_GET, indexPage);
    srv.on("/dev", HTTP_GET, devPage);
    srv.on("/display", HTTP_GET | HTTP_POST, displayPage);
    srv.on("/network", HTTP_GET, networkPage);
    srv.on("/config", HTTP_GET | HTTP_POST, configPage);
    srv.on("/settings", HTTP_GET | HTTP_POST, settingsPage);
    srv.on("/update", HTTP_GET, updatePage);
    srv.on("/upload", HTTP_POST, uploadPage, uploadHandler);

    /* Serve files from filesystem */
    srv.serveStatic("/data/style.css", SPIFFS, "/style.css");
    srv.serveStatic("/data/util.js", SPIFFS, "/util.js");

    return;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Check the given hostname and returns whether it is valid or not.
 * Validation is according to RFC952.
 * 
 * @param[in] hostname  Hostname which to validate
 * 
 * @return Is valid (true) or not (false).
 */
static bool isValidHostname(const String& hostname)
{
    bool isValid = true;

    if ((MIN_HOSTNAME_LENGTH > hostname.length()) ||
        (MAX_HOSTNAME_LENGTH < hostname.length()))
    {
        isValid = false;
    }
    else
    {
        uint8_t index = 0;

        while((true == isValid) && (index < hostname.length()))
        {
            if (('0' <= hostname[index]) &&
                ('9' >= hostname[index]))
            {
                /* No digit at the begin */
                if (0 == index)
                {
                    isValid = false;
                }
            }
            else if (('A' <= hostname[index]) &&
                     ('Z' >= hostname[index]))
            {
                /* Ok */
                ;
            }
            else if (('a' <= hostname[index]) &&
                     ('z' >= hostname[index]))
            {
                /* Ok */
                ;
            }
            else if ('-' == hostname[index])
            {
                /* No - at the begin */
                if (0 == index)
                {
                    isValid = false;
                }
            }
            else
            {
                isValid = false;
            }

            ++index;
        }
    }

    return isValid;
}

/**
 * Check the given wifi SSID and returns whether it is valid or not.
 * Validation is according to Section 7.3.2.1 of the 802.11-2007 specification.
 * 
 * @param[in] hostname  SSID which to validate
 * 
 * @return Is valid (true) or not (false).
 */
static bool isValidSSID(const String& ssid)
{
    bool isValid = true;

    if ((MIN_SSID_LENGTH > ssid.length()) ||
        (MAX_SSID_LENGTH < ssid.length()))
    {
        isValid = false;
    }

    return isValid;
}

/**
 * Get text in color format (HTML).
 * 
 * @param[in] text  Text
 * 
 * @return Text in color format (HTML).
 */
static String getColoredText(const String& text)
{
    String      result;
    uint8_t     index       = 0;
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

    for(index = 0; index < text.length(); ++index)
    {
        result += "<span style=\"color:";
        result += colors[colorIndex];
        result += "\">";
        result += text[index];
        result += "</span>";

        ++colorIndex;
        if (ARRAY_NUM(colors) <= colorIndex)
        {
            colorIndex = 0;
        }
    }

    return result;
}

/**
 * Processor for page template, containing the common part, which is available
 * in every page. It is responsible for the data binding.
 * 
 * @param[in] var   Name of variable in the template
 */
static String commonPageProcessor(const String& var)
{
    String  result;

    if (var == "PAGE_TITLE")
    {
        result = WebConfig::PROJECT_TITLE;
    }
    else if (var == "HEADER")
    {
        result += "<h1>";
        result += ".:";
        result += getColoredText(WebConfig::PROJECT_TITLE);
        result += ":.";
        result += "</h1>\r\n";
    }
    else
    {
        ;
    }

    return result;
}

/**
 * Error web page used in case a requested path was not found.
 * 
 * @param[in] request   HTTP request
 */
static void errorPage(AsyncWebServerRequest* request)
{
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

    request->send(SPIFFS, "/error.html", "text/html", false, errorPageProcessor);

    return;
}

/**
 * Processor for error page template.
 * It is responsible for the data binding.
 * 
 * @param[in] var   Name of variable in the template
 */
static String errorPageProcessor(const String& var)
{
    return commonPageProcessor(var);
}

/**
 * Index page on root path ("/").
 * 
 * @param[in] request   HTTP request
 */
static void indexPage(AsyncWebServerRequest* request)
{
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

    request->send(SPIFFS, "/index.html", "text/html", false, indexPageProcessor);

    return;
}

/**
 * Processor for index page template.
 * It is responsible for the data binding.
 * 
 * @param[in] var   Name of variable in the template
 */
static String indexPageProcessor(const String& var)
{
    String  result;

    if (var == "VERSION")
    {
        result = Version::SOFTWARE;
    }
    else if (var == "ESP_SDK_VERSION")
    {
        result = ESP.getSdkVersion();
    }
    else if (var == "HEAP_SIZE")
    {
        result = ESP.getHeapSize();
    }
    else if (var == "AVAILABLE_HEAP_SIZE")
    {
        result = ESP.getFreeHeap();
    }
    else if (var == "ESP_CHIP_REV")
    {
        result = ESP.getChipRevision();
    }
    else if (var == "ESP_CPU_FREQ")
    {
        result = ESP.getCpuFreqMHz();
    }
    else
    {
        result = commonPageProcessor(var);
    }

    return result;
}

/**
 * Network page, shows all information regarding the network.
 * 
 * @param[in] request   HTTP request
 */
static void networkPage(AsyncWebServerRequest* request)
{
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

    request->send(SPIFFS, "/network.html", "text/html", false, networkPageProcessor);

    return;
}

/**
 * Processor for network page template.
 * It is responsible for the data binding.
 * 
 * @param[in] var   Name of variable in the template
 */
static String networkPageProcessor(const String& var)
{
    String  result;

    if (var == "SSID")
    {
        if (true == Settings::getInstance().open(true))
        {
            result = Settings::getInstance().getWifiSSID();
            Settings::getInstance().close();
        }
    }
    else if (var == "RSSI")
    {
        result = WiFi.RSSI();
    }
    else if (var == "HOSTNAME")
    {
        const char* hostname = NULL;

        if (WIFI_MODE_AP == WiFi.getMode())
        {
            hostname = WiFi.softAPgetHostname();
        }
        else
        {
            hostname = WiFi.getHostname();
        }

        if (NULL != hostname)
        {
            result = hostname;
        }
    }
    else if (var == "IPV4")
    {
        if (WIFI_MODE_AP == WiFi.getMode())
        {
            result = WiFi.softAPIP().toString();
        }
        else
        {
            result = WiFi.localIP().toString();
        }
    }
    else
    {
        result = commonPageProcessor(var);
    }

    return result;
}

/**
 * Configuration page, where to configure the display.
 * 
 * @param[in] request   HTTP request
 */
static void configPage(AsyncWebServerRequest* request)
{
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

    /* Store configuration? */
    if ((HTTP_POST == request->method()) &&
        (0 < request->args()))
    {
        String jsonRsp;

        if (false == request->hasArg("matrixType"))
        {
            jsonRsp = "{ \"status\": 1, \"error\": \"Matrix type is missing.\" }";
        }
        else if (false == Settings::getInstance().open(false))
        {
            jsonRsp = "{ \"status\": 1, \"error\": \"Internal error.\" }";
        }
        else
        {
            int32_t i32MatrixType = request->arg("matrixType").toInt();

            if ((0 > i32MatrixType) ||
                (UINT8_MAX < i32MatrixType))
            {
                jsonRsp = "{ \"status\": 1, \"error\": \"Invalid matrix type.\" }";
            }
            else
            {
                uint8_t matrixType = static_cast<uint8_t>(i32MatrixType);

                Settings::getInstance().setMatrixType(matrixType);
                LedMatrix::getInstance().setType(matrixType);

                jsonRsp = "{ \"status\": 0, \"info\": \"Successful stored and applied.\" }";
            }

            Settings::getInstance().close();
        }

        request->send(HttpStatus::STATUS_CODE_OK, "application/json", jsonRsp);
    }
    else if (HTTP_GET == request->method())
    {
        request->send(SPIFFS, "/configuration.html", "text/html", false, configPageProcessor);
    }
    else
    {
        request->send(HttpStatus::STATUS_CODE_BAD_REQ, "plain/text", "Error");
    }

    return;
}

/**
 * Processor for configuration page template.
 * It is responsible for the data binding.
 * 
 * @param[in] var   Name of variable in the template
 */
static String configPageProcessor(const String& var)
{
    String  result;

    if (var == "MATRIX_TYPE")
    {
        if (false == Settings::getInstance().open(true))
        {
            result += 0u;
        }
        else
        {
            uint8_t matrixType = Settings::getInstance().getMatrixType();
            Settings::getInstance().close();

            result += matrixType;
        }
    }
    else
    {
        result = commonPageProcessor(var);
    }

    return result;
}

/**
 * Settings page to show and store settings.
 * 
 * @param[in] request   HTTP request
 */
static void settingsPage(AsyncWebServerRequest* request)
{
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
    if ((HTTP_POST == request->method()) &&
        (0 < request->args()))
    {
        bool    isError = false;
        String  jsonRsp;

        if (true == request->hasArg(FORM_INPUT_NAME_WIFI_SSID))
        {
            String  wifiSSID = request->arg(FORM_INPUT_NAME_WIFI_SSID);

            if (false == isValidSSID(wifiSSID))
            {
                isError = true;
                jsonRsp = "{ \"status\": 1, \"error\": \"Invalid SSID.\" }";
            }
            else
            {
                if (false == Settings::getInstance().open(false))
                {
                    LOG_WARNING("Couldn't open settings.");

                    isError = true;
                    jsonRsp = "{ \"status\": 1, \"error\": \"Internal error.\" }";
                }
                else
                {
                    Settings::getInstance().setWifiSSID(wifiSSID);
                    Settings::getInstance().close();
                }
            }
        }
        else if (true == request->hasArg(FORM_INPUT_NAME_WIFI_PASSPHRASE))
        {
            String  wifiPassphrase = request->arg(FORM_INPUT_NAME_WIFI_PASSPHRASE);

            /* Check arguments min. and max. lengths */
            if (MIN_PASSPHRASE_LENGTH > wifiPassphrase.length())
            {
                isError = true;
                jsonRsp = "{ \"status\": 1, \"error\": \"Passphrase too short.\" }";
            }
            else if (MAX_PASSPHRASE_LENGTH < wifiPassphrase.length())
            {
                isError = true;
                jsonRsp = "{ \"status\": 1, \"error\": \"Passphrase too long.\" }";
            }
            else
            {
                if (false == Settings::getInstance().open(false))
                {
                    LOG_WARNING("Couldn't open settings.");

                    isError = true;
                    jsonRsp = "{ \"status\": 1, \"error\": \"Internal error.\" }";
                }
                else
                {
                    Settings::getInstance().setWifiPassphrase(wifiPassphrase);
                    Settings::getInstance().close();
                }
            }
        }
        else if (true == request->hasArg(FORM_INPUT_NAME_WIFI_AP_SSID))
        {
            String  wifiApSSID = request->arg(FORM_INPUT_NAME_WIFI_AP_SSID);

            if (false == isValidSSID(wifiApSSID))
            {
                isError = true;
                jsonRsp = "{ \"status\": 1, \"error\": \"Invalid SSID.\" }";
            }
            else
            {
                if (false == Settings::getInstance().open(false))
                {
                    LOG_WARNING("Couldn't open settings.");

                    isError = true;
                    jsonRsp = "{ \"status\": 1, \"error\": \"Internal error.\" }";
                }
                else
                {
                    Settings::getInstance().setWifiApSSID(wifiApSSID);
                    Settings::getInstance().close();
                }
            }
        }
        else if (true == request->hasArg(FORM_INPUT_NAME_WIFI_AP_PASSPHRASE))
        {
            String  wifiApPassphrase = request->arg(FORM_INPUT_NAME_WIFI_AP_PASSPHRASE);

            /* Check arguments min. and max. lengths */
            if (MIN_PASSPHRASE_LENGTH > wifiApPassphrase.length())
            {
                isError = true;
                jsonRsp = "{ \"status\": 1, \"error\": \"Passphrase too short.\" }";
            }
            else if (MAX_PASSPHRASE_LENGTH < wifiApPassphrase.length())
            {
                isError = true;
                jsonRsp = "{ \"status\": 1, \"error\": \"Passphrase too long.\" }";
            }
            else
            {
                if (false == Settings::getInstance().open(false))
                {
                    LOG_WARNING("Couldn't open settings.");

                    isError = true;
                    jsonRsp = "{ \"status\": 1, \"error\": \"Internal error.\" }";
                }
                else
                {
                    Settings::getInstance().setWifiApPassphrase(wifiApPassphrase);
                    Settings::getInstance().close();
                }
            }
        }
        else if (true == request->hasArg(FORM_INPUT_NAME_HOSTNAME))
        {
            String  hostname = request->arg(FORM_INPUT_NAME_HOSTNAME);

            if (false == isValidHostname(hostname))
            {
                isError = true;
                jsonRsp = "{ \"status\": 1, \"error\": \"Hostname is invalid.\" }";
            }
            else
            {
                if (false == Settings::getInstance().open(false))
                {
                    LOG_WARNING("Couldn't open settings.");

                    isError = true;
                    jsonRsp = "{ \"status\": 1, \"error\": \"Internal error.\" }";
                }
                else
                {
                    Settings::getInstance().setHostname(hostname);
                    Settings::getInstance().close();
                }
            }
        }
        else
        {
            isError = true;
            jsonRsp = "{ \"status\": 1, \"error\": \"Unknown argument.\" }";
        }

        if (false == isError)
        {
            jsonRsp = "{ \"status\": 0, \"info\": \"Successful stored.\" }";
        }

        request->send(HttpStatus::STATUS_CODE_OK, "application/json", jsonRsp);
    }
    else if (HTTP_GET == request->method())
    {
        request->send(SPIFFS, "/settings.html", "text/html", false, settingsPageProcessor);
    }
    else
    {
        request->send(HttpStatus::STATUS_CODE_BAD_REQ, "plain/text", "Error");
    }
    
    return;
}

/**
 * Add settings data to the destination string in JSON format.
 * The structure depends on the javascript code in settings.html.
 * 
 * @param[in] dst       Destination string
 * @param[in] title     Settings title
 * @param[in] name      Input field name
 * @param[in] value     Input field value
 * @param[in] size      Input field size in characters
 * @param[in] minLen    Minimum length of input field value
 * @param[in] maxLen    Maximum length of input field value
 */
static void addSettingsData(String& dst, const char* title, const char* name, const char* value, uint8_t size, uint8_t minLen, uint8_t maxLen)
{
    dst += "{";
    dst += "title: \"";
    dst += title;
    dst += "\", ";
    dst += "input: {";
    dst += "name: \"";
    dst += name;
    dst += "\", ";
    dst += "value: \"";
    dst += value;
    dst += "\", ";
    dst += "size: ";
    dst += size;
    dst += ", ";
    dst += "minlength: ";
    dst += minLen;
    dst += ", ";
    dst += "maxlength: ";
    dst += maxLen;
    dst += "} ";
    dst += "}";
    return;
}

/**
 * Processor for settings page template.
 * It is responsible for the data binding.
 * 
 * @param[in] var   Name of variable in the template
 */
static String settingsPageProcessor(const String& var)
{
    String  result;

    if (var == "DATA")
    {
        String  wifiSSID;
        String  wifiPassphrase;
        String  wifiApSSID;
        String  wifiApPassphrase;
        String  hostname;

        Settings::getInstance().open(true);
        wifiSSID            = Settings::getInstance().getWifiSSID();
        wifiPassphrase      = Settings::getInstance().getWifiPassphrase();
        wifiApSSID          = Settings::getInstance().getWifiApSSID();
        wifiApPassphrase    = Settings::getInstance().getWifiApPassphrase();
        hostname            = Settings::getInstance().getHostname();
        Settings::getInstance().close();

        addSettingsData(result, "Wifi SSID", FORM_INPUT_NAME_WIFI_SSID, wifiSSID.c_str(), MAX_SSID_LENGTH, MIN_SSID_LENGTH, MAX_SSID_LENGTH);
        result += ", ";
        addSettingsData(result, "Wifi Passphrase", FORM_INPUT_NAME_WIFI_PASSPHRASE, wifiPassphrase.c_str(), MAX_PASSPHRASE_LENGTH, MIN_PASSPHRASE_LENGTH, MAX_PASSPHRASE_LENGTH);
        result += ", ";
        addSettingsData(result, "Wifi AP SSID", FORM_INPUT_NAME_WIFI_AP_SSID, wifiApSSID.c_str(), MAX_SSID_LENGTH, MIN_SSID_LENGTH, MAX_SSID_LENGTH);
        result += ", ";
        addSettingsData(result, "Wifi AP Passphrase", FORM_INPUT_NAME_WIFI_AP_PASSPHRASE, wifiApPassphrase.c_str(), MAX_PASSPHRASE_LENGTH, MIN_PASSPHRASE_LENGTH, MAX_PASSPHRASE_LENGTH);
        result += ", ";
        addSettingsData(result, "Hostname", FORM_INPUT_NAME_HOSTNAME, hostname.c_str(), MAX_HOSTNAME_LENGTH, MIN_HOSTNAME_LENGTH, MAX_HOSTNAME_LENGTH);
    }
    else
    {
        result = commonPageProcessor(var);
    }

    return result;
}

/**
 * Page with stuff for development and debug purposes.
 * 
 * @param[in] request   HTTP request
 */
static void devPage(AsyncWebServerRequest* request)
{
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

    request->send(SPIFFS, "/dev.html", "text/html", false, devPageProcessor);

    return;
}

/**
 * Processor for dev page template.
 * It is responsible for the data binding.
 * 
 * @param[in] var   Name of variable in the template
 */
static String devPageProcessor(const String& var)
{
    String  result;

    if (var == "WS_PROTOCOL")
    {
        result = WebConfig::WEBSOCKET_PROTOCOL;
    }
    else if (var == "WS_HOSTNAME")
    {
        if (WIFI_MODE_AP == WiFi.getMode())
        {
            result = WiFi.softAPgetHostname();
        }
        else
        {
            result = WiFi.getHostname();
        }
    }
    else if (var == "WS_PORT")
    {
        result = WebConfig::WEBSOCKET_PORT;
    }
    else if (var == "WS_ENDPOINT")
    {
        result = WebConfig::WEBSOCKET_PATH;
    }
    else
    {
        result = commonPageProcessor(var);
    }

    return result;
}

/**
 * Page for software update.
 * 
 * @param[in] request   HTTP request
 */
static void updatePage(AsyncWebServerRequest* request)
{
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

    request->send(SPIFFS, "/update.html", "text/html", false, updatePageProcessor);

    return;
}

/**
 * Processor for update page template.
 * It is responsible for the data binding.
 * 
 * @param[in] var   Name of variable in the template
 */
static String updatePageProcessor(const String& var)
{
    String  result;

    if (var == "FIRMWARE_FILENAME")
    {
        result = FIRMWARE_FILENAME;
    }
    else if (var == "FILESYSTEM_FILENAME")
    {
        result = FILESYSTEM_FILENAME;
    }
    else
    {
        result = commonPageProcessor(var);
    }

    return result;
}

/**
 * Page for upload result.
 * 
 * @param[in] request   HTTP request
 */
static void uploadPage(AsyncWebServerRequest* request)
{
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

    request->send(HttpStatus::STATUS_CODE_OK, "text/plain", "Ok");

    return;
}

/**
 * File upload handler.
 * 
 * @param[in] request   HTTP request.
 * @param[in] filename  Name of the uploaded file.
 * @param[in] index     Current file offset.
 * @param[in] data      Next data part of file, starting at offset.
 * @param[in] len       Data part size in byte.
 * @param[in] final     Is final packet or not.
 */
static void uploadHandler(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final)
{
    bool            isError     = false;
    static uint32_t progress    = 0u;

    /* Begin of upload? */
    if (0 == index)
    {
        /* Upload firmware or filesystem? */
        int cmd = (filename == FILESYSTEM_FILENAME) ? U_SPIFFS : U_FLASH;

        LOG_INFO("Upload of %s (%d bytes) starts.", filename.c_str(), request->contentLength());

        /* TODO request->contentLength() contains 200 bytes more, than which will be written.
         * How to calculate it correct?
         */
        if (false == Update.begin(request->contentLength(), cmd))
        {
            LOG_ERROR("Upload failed: %s", Update.errorString());
            isError = true;
        }

        progress = 0u;
    }

    if (true == Update.isRunning())
    {
        if(len != Update.write(data, len))
        {
            LOG_ERROR("Upload failed: %s", Update.errorString());
            isError = true;
        }
        else
        {
            uint32_t progressNext = (Update.progress() * 100) / Update.size();

            /* Don't spam the console and output only if something changed. */
            if (progress != progressNext)
            {
                LOG_INFO("Upload progress: %u %%", progressNext);
                progress = progressNext;
            }
        }

        /* Upload finished? */
        if (true == final)
        {
            if (false == Update.end(true))
            {
                LOG_ERROR("Upload failed: %s", Update.errorString());
                isError = true;
            }
            else
            {
                LOG_INFO("Upload of %s finished.", filename.c_str());

                /* Request a restart */
                UpdateMgr::getInstance().reqRestart();
            }
        }
    }

    if (true == isError)
    {
        Update.abort();
        request->send(HttpStatus::STATUS_CODE_BAD_REQ, "plain/text", "Error");
    }

    return;
}

/**
 * Display page, showing current display content.
 * 
 * @param[in] request   HTTP request
 */
static void displayPage(AsyncWebServerRequest* request)
{
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

    /* Store configuration? */
    if ((HTTP_POST == request->method()) &&
        (0 < request->args()))
    {
        String jsonRsp;

        if (false == request->hasArg("get"))
        {
            jsonRsp = "{ \"data\": [] }";
        }
        else if (request->arg("get") != "data")
        {
            jsonRsp = "{ \"data\": [] }";
        }
        else
        {
            uint16_t        index   = 0;
            const uint32_t* fb      = NULL;
            size_t          fbSize  = 0;
            
            DisplayMgr::getInstance().getFBCopy(fb, fbSize);

            jsonRsp = "{ \"data\": [ ";

            for(index = 0; index < fbSize; ++index)
            {
                if (0 < index)
                {
                    jsonRsp += ", ";
                }

                jsonRsp += fb[index];
            }

            jsonRsp += " ] }";
        }

        request->send(HttpStatus::STATUS_CODE_OK, "application/json", jsonRsp);
    }
    else if (HTTP_GET == request->method())
    {
        request->send(SPIFFS, "/display.html", "text/html", false, displayPageProcessor);
    }
    else
    {
        request->send(HttpStatus::STATUS_CODE_BAD_REQ, "plain/text", "Error");
    }

    return;
}

/**
 * Processor for display page template.
 * It is responsible for the data binding.
 * 
 * @param[in] var   Name of variable in the template
 */
static String displayPageProcessor(const String& var)
{
    return commonPageProcessor(var);
}

/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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
#include "RestApi.h"

#include <WiFi.h>
#include <Esp.h>
#include <SPIFFS.h>
#include <Update.h>
#include <Logging.h>
#include <Util.h>
#include <SPIFFSEditor.h>

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

static bool isValidHostname(const String& hostname);
static bool isValidSSID(const String& ssid);
static String getColoredText(const String& text);

static String commonPageProcessor(const String& var);

static String errorPageProcessor(const String& var);

static void indexPage(AsyncWebServerRequest* request);
static String indexPageProcessor(const String& var);

static void networkPage(AsyncWebServerRequest* request);
static String networkPageProcessor(const String& var);

static void addSettingsData(String& dst, const char* title, const char* name, const char* value, uint8_t size, uint8_t minLen, uint8_t maxLen);
static void settingsPage(AsyncWebServerRequest* request);
static String settingsPageProcessor(const String& var);

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
static const uint8_t    MIN_SSID_LENGTH             = 0U;

/** Max. wifi SSID length. Section 7.3.2.1 of the 802.11-2007 specification. */
static const uint8_t    MAX_SSID_LENGTH             = 32U;

/** Min. wifi passphrase length */
static const uint8_t    MIN_PASSPHRASE_LENGTH       = 8U;

/** Max. wifi passphrase length */
static const uint8_t    MAX_PASSPHRASE_LENGTH       = 64U;

/** Min. hostname length (RFC1034 1 - 63) */
static const uint8_t    MIN_HOSTNAME_LENGTH         = 1U;

/** Max. hostname length (RFC1034 1 - 63) */
static const uint8_t    MAX_HOSTNAME_LENGTH         = 63U;

/** Firmware binary filename, used for update. */
static const char*      FIRMWARE_FILENAME           = "firmware.bin";

/** Filesystem binary filename, used for update. */
static const char*      FILESYSTEM_FILENAME         = "spiffs.bin";

/** Flag used to signal any kind of file upload error. */
static bool             gIsUploadError               = false;

/** The SPIFFS editor instance. */
static SPIFFSEditor     gSPIFFSEditor(SPIFFS);

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
    AsyncStaticWebHandler*  handler = nullptr;

    (void)srv.on("/", HTTP_GET, indexPage);
    (void)srv.on("/display", HTTP_GET, displayPage);
    (void)srv.on("/network", HTTP_GET, networkPage);
    (void)srv.on("/settings", HTTP_GET | HTTP_POST, settingsPage);
    (void)srv.on("/update", HTTP_GET, updatePage);
    (void)srv.on("/upload", HTTP_POST, uploadPage, uploadHandler);

    /* Serve files with static content with enabled cache control.
     * The client may cache files from filesytem for 1 hour.
     */
    handler = &srv.serveStatic("/data/style.css", SPIFFS, "/style.css");
    handler->setCacheControl("max-age=3600");

    handler = &srv.serveStatic("/data/ws.js", SPIFFS, "/ws.js");
    handler->setCacheControl("max-age=3600");

    /* Add SPIFFS file editor to "/edit" */
    (void)srv.addHandler(&gSPIFFSEditor);

    return;
}

/**
 * Error web page used in case a requested path was not found.
 *
 * @param[in] request   HTTP request
 */
void Pages::error(AsyncWebServerRequest* request)
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

    request->send(SPIFFS, "/error.html", "text/html", false, errorPageProcessor);

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
        if (UTIL_ARRAY_NUM(colors) <= colorIndex)
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
    else if (var == "FS_SIZE")
    {
        result = SPIFFS.totalBytes();
    }
    else if (var == "USED_FS_SIZE")
    {
        result = SPIFFS.usedBytes();
    }
    else if (var == "ESP_CHIP_REV")
    {
        result = ESP.getChipRevision();
    }
    else if (var == "ESP_CHIP_ID")
    {
        uint64_t    chipId      = ESP.getEfuseMac();
        uint32_t    highPart    = (chipId >> 32U) & 0x0000ffffU;
        uint32_t    lowPart     = (chipId >>  0U) & 0xffffffffU;
        char        chipIdStr[13];

        snprintf(chipIdStr, UTIL_ARRAY_NUM(chipIdStr), "%04X%08X", highPart, lowPart);

        result = chipIdStr;
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
        const char* hostname = nullptr;

        if (WIFI_MODE_AP == WiFi.getMode())
        {
            hostname = WiFi.softAPgetHostname();
        }
        else
        {
            hostname = WiFi.getHostname();
        }

        if (nullptr != hostname)
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
    else if (var == "MAC_ADDR")
    {
        result = WiFi.macAddress();
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
 * Page for software update.
 *
 * @param[in] request   HTTP request
 */
static void updatePage(AsyncWebServerRequest* request)
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

    if (true == gIsUploadError)
    {
        request->send(HttpStatus::STATUS_CODE_BAD_REQ, "text/plain", "Error");
    }
    else
    {
        request->send(HttpStatus::STATUS_CODE_OK, "text/plain", "Ok");
    }

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
    /* Begin of upload? */
    if (0 == index)
    {
        AsyncWebHeader* header = request->getHeader("X-File-Size");
        uint32_t        fileSize    = UPDATE_SIZE_UNKNOWN;

        /* Upload firmware or filesystem? */
        int cmd = (filename == FILESYSTEM_FILENAME) ? U_SPIFFS : U_FLASH;

        /* File size available? */
        if (nullptr != header)
        {
            /* If conversion fails, it will contain UPDATE_SIZE_UNKNOWN. */
            (void)Util::strToUInt32(header->value(), fileSize);
        }

        if (UPDATE_SIZE_UNKNOWN == fileSize)
        {
            LOG_INFO("Upload of %s (unknown size) starts.", filename.c_str());
        }
        else
        {
            LOG_INFO("Upload of %s (%u byte) starts.", filename.c_str(), fileSize);
        }

        gIsUploadError = false;

        /* Update filesystem? */
        if (U_SPIFFS == cmd)
        {
            /* Close filesystem before continue.
             * Note, this needs a restart after update is finished.
             */
            SPIFFS.end();
        }

        /* We will get only the content length of the whole request, but not the size
         * of the file itself. Therefore we must set UPDATE_SIZE_UNKNOWN.
         */
        if (false == Update.begin(fileSize, cmd))
        {
            LOG_ERROR("Upload failed: %s", Update.errorString());
            gIsUploadError = true;

            /* Mount filesystem again. */
            if (false == SPIFFS.begin())
            {
                LOG_FATAL("Couldn't mount filesystem.");
            }

            /* Inform client about abort.*/
            request->send(HttpStatus::STATUS_CODE_ENTITY_TOO_LARGE, "text/plain", "Upload aborted.");
        }
        /* Update is now running. */
        else
        {
            /* Use UpdateMgr to show the user the update status.
             * Note, the display manager will be completey stopped during this,
             * to avoid artifacts on the display, because of long writes to flash.
             */
            UpdateMgr::getInstance().beginProgress();
        }
    }

    if (true == Update.isRunning())
    {
        if (false == gIsUploadError)
        {
            if(len != Update.write(data, len))
            {
                LOG_ERROR("Upload failed: %s", Update.errorString());
                gIsUploadError = true;
            }
            else
            {
                uint32_t progress = (Update.progress() * 100) / Update.size();

                UpdateMgr::getInstance().updateProgress(progress);
            }

            /* Upload finished? */
            if (true == final)
            {
                /* Finish update now. */
                if (false == Update.end(true))
                {
                    LOG_ERROR("Upload failed: %s", Update.errorString());
                    gIsUploadError = true;
                }
                /* Update was successful! */
                else
                {
                    LOG_INFO("Upload of %s finished.", filename.c_str());

                    /* Filesystem is not mounted here, because we will restart in the next seconds. */

                    /* Ensure that the user see 100% update status on the display. */
                    UpdateMgr::getInstance().updateProgress(100U);
                    UpdateMgr::getInstance().endProgress();

                    /* Request a restart */
                    UpdateMgr::getInstance().reqRestart();
                }
            }
        }
        else
        {
            /* Mount filesystem again, it may be unmounted in case of filesystem update. */
            if (false == SPIFFS.begin())
            {
                LOG_FATAL("Couldn't mount filesystem.");
            }

            /* Abort update */
            Update.abort();
            UpdateMgr::getInstance().endProgress();

            /* Inform client about abort.*/
            request->send(HttpStatus::STATUS_CODE_ENTITY_TOO_LARGE, "text/plain", "Upload aborted.");
        }
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

    request->send(SPIFFS, "/display.html", "text/html", false, displayPageProcessor);

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
    String  result;

    if (var == "WS_PROTOCOL")
    {
        result = WebConfig::WEBSOCKET_PROTOCOL;
    }
    else if (var == "WS_HOSTNAME")
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

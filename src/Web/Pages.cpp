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
#include "UpdateMgr.h"

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

static String getColoredText(const String& text);

static String commonPageProcessor(const String& var);

static void errorPage(AsyncWebServerRequest* request);
static String errorPageProcessor(const String& var);

static void indexPage(AsyncWebServerRequest* request);
static String indexPageProcessor(const String& var);

static void networkPage(AsyncWebServerRequest* request);
static String networkPageProcessor(const String& var);

static void settingsPage(AsyncWebServerRequest* request);
static String settingsPageProcessor(const String& var);

static void devPage(AsyncWebServerRequest* request);
static String devPageProcessor(const String& var);

static void updatePage(AsyncWebServerRequest* request);
static String updatePageProcessor(const String& var);

static void uploadPage(AsyncWebServerRequest* request);
static String uploadPageProcessor(const String& var);
static void uploadHandler(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

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

/** Firmware binary filename, used for update. */
static const char*      FIRMWARE_FILENAME           = "firmware.bin";

/** Filesystem binary filename, used for update. */
static const char*      FILESYSTEM_FILENAME         = "spiffs.bin";

/** Dialog flag, whether the dialog on the settings page shall be shown or not. */
static bool             gShowDialog = false;

/** Dialog title, used for settings page. */
static String           gDialogTitle;

/** Dialog text, used for settings page. */
static String           gDialogText;

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
    srv.on("/network", HTTP_GET, networkPage);
    srv.on("/settings", HTTP_GET, settingsPage);
    srv.on("/settings", HTTP_POST, settingsPage);
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
    if (0 < request->args())
    {
        String  ssid;
        String  passphrase;
        bool    isError     = false;

        gDialogText = "";

        /* Check for the necessary arguments. */
        if (false == request->hasArg(FORM_INPUT_NAME_SSID))
        {
            isError = true;
            gDialogText += "<p>SSID missing.</p>\r\n";
        }

        if (false == request->hasArg(FORM_INPUT_NAME_PASSPHRASE))
        {
            isError = true;
            gDialogText += "<p>Passphrase missing.</p>\r\n";
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
                gDialogText += "<p>SSID too short.</p>";
            }
            else if (MAX_SSID_LENGTH < ssid.length())
            {
                isError = true;
                gDialogText += "<p>SSID too long.</p>";
            }

            if (MIN_PASSPHRASE_LENGTH > passphrase.length())
            {
                isError = true;
                gDialogText += "<p>Passphrase too short.</p>";
            }
            else if (MAX_PASSPHRASE_LENGTH < passphrase.length())
            {
                isError = true;
                gDialogText += "<p>Passphrase too long.</p>";
            }

            /* Arguments are valid, store them. */
            if (false == isError)
            {
                Settings::getInstance().open(false);
                Settings::getInstance().setWifiSSID(ssid);
                Settings::getInstance().setWifiPassphrase(passphrase);
                Settings::getInstance().close();

                gDialogText = "<p>Successful saved.</p>";
            }
        }

        if (false == isError)
        {
            gDialogTitle = "Info";
        }
        else
        {
            gDialogTitle = "Error";
        }

        gShowDialog = true;
    }

    request->send(SPIFFS, "/settings.html", "text/html", false, settingsPageProcessor);

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

    if (var == "SSID")
    {
        if (true == Settings::getInstance().open(true))
        {
            result = Settings::getInstance().getWifiSSID();
            Settings::getInstance().close();
        }
    }
    else if (var == "SETTINGS")
    {
        String ssid;
        String passphrase;

        Settings::getInstance().open(true);
        ssid        = Settings::getInstance().getWifiSSID();
        passphrase  = Settings::getInstance().getWifiPassphrase();
        Settings::getInstance().close();

        result += "<p>SSID:<br />\r\n";
        result += Html::inputText(FORM_INPUT_NAME_SSID, ssid, MAX_SSID_LENGTH, MIN_SSID_LENGTH, MAX_SSID_LENGTH);
        result += "</p><br />\r\n";
        result += "<p>Passphrase:<br />\r\n";
        result += Html::inputText(FORM_INPUT_NAME_PASSPHRASE, passphrase, MAX_PASSPHRASE_LENGTH, MIN_PASSPHRASE_LENGTH, MAX_PASSPHRASE_LENGTH);
        result += "</p><br />\r\n";
        result += "<p><input type=\"submit\" value=\"Save\"></p>";
    }
    else if (var == "SHOW_DIALOG")
    {
        if (false == gShowDialog)
        {
            result = "false";
        }
        else
        {
            result = "true";
        }
    }
    else if (var == "DIALOG_TITLE")
    {
        result = gDialogTitle;
    }
    else if (var == "DIALOG")
    {
        result = gDialogText;
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

    request->send(SPIFFS, "/upload.html", "text/html", false, uploadPageProcessor);

    return;
}

/**
 * Processor for upload page template.
 * It is responsible for the data binding.
 * 
 * @param[in] var   Name of variable in the template
 */
static String uploadPageProcessor(const String& var)
{
    String  result;

    if (var == "UPLOAD_RESULT")
    {
        if (true == Update.hasError())
        {
            result = "Upload failed.";
        }
        else
        {
            result = "Upload successful.";
        }
    }
    else
    {
        result = commonPageProcessor(var);
    }

    return result;
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
        request->send(Html::STATUS_CODE_BAD_REQ, "plain/text", "Error");
    }

    return;
}
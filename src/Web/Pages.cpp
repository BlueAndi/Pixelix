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
#include "DisplayMgr.h"
#include "RestApi.h"
#include "PluginMgr.h"
#include "FileSystem.h"

#include <WiFi.h>
#include <Esp.h>
#include <Update.h>
#include <Logging.h>
#include <Util.h>
#include <ArduinoJson.h>
#include <lwip/init.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/**
 * This type defines a template keyword to function.
 */
struct TmplKeyWordFunc
{
    const char* keyword;        /**< Keyword */
    String      (*func)(void);  /**< Function to call */
};

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static String fitToSpiffs(const String& path, const String& filenNameWithoutExt, const String& fileNameExtension);

static String tmplPageProcessor(const String& var);

static void aboutPage(AsyncWebServerRequest* request);
static void debugPage(AsyncWebServerRequest* request);
static void displayPage(AsyncWebServerRequest* request);
static void editPage(AsyncWebServerRequest* request);
static void indexPage(AsyncWebServerRequest* request);
static void infoPage(AsyncWebServerRequest* request);
static void settingsPage(AsyncWebServerRequest* request);
static void updatePage(AsyncWebServerRequest* request);
static void uploadPage(AsyncWebServerRequest* request);
static void uploadHandler(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);

namespace tmpl
{
    static String getEspChipId();
    static String getEspType();
    static String getFlashChipMode();
    static String getHostname();
    static String getIPAddress();
    static String getRSSI();
    static String getSSID();
};

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Firmware binary filename, used for update. */
static const char*      FIRMWARE_FILENAME               = "firmware.bin";

/** Path to the plugin webpages. */
static const String     PLUGIN_PAGE_PATH                = "/plugins/";

/** Plugin webpage file extension. */
static const String     PLUGIN_PAGE_FILE_EXTENSION      = ".html";

/** SPIFFS limits the max. filename length, which includes the path as well. */
static const uint32_t   SPIFFS_FILENAME_LENGTH_LIMIT    = 32U;

/** Flag used to signal any kind of file upload error. */
static bool             gIsUploadError                  = false;

/**
 * List of all used template keywords and the function how to retrieve the information.
 * The list is alphabetic sorted in ascending order.
 */
static TmplKeyWordFunc  gTmplKeyWordToFunc[]            =
{
    "ARDUINO_IDF_BRANCH",   []() -> String { return CONFIG_ARDUINO_IDF_BRANCH; },
    "ESP_CHIP_ID",          tmpl::getEspChipId,
    "ESP_CHIP_REV",         []() -> String { return String(ESP.getChipRevision()); },
    "ESP_CPU_FREQ",         []() -> String { return String(ESP.getCpuFreqMHz()); },
    "ESP_SDK_VERSION",      []() -> String { return ESP.getSdkVersion(); },
    "ESP_TYPE",             tmpl::getEspType,
    "FILESYSTEM_FILENAME",  []() -> String { return FILESYSTEM_FILENAME; },
    "FIRMWARE_FILENAME",    []() -> String { return FIRMWARE_FILENAME; },
    "FLASH_CHIP_MODE",      tmpl::getFlashChipMode,
    "FLASH_CHIP_SIZE",      []() -> String { return String(ESP.getFlashChipSize() / (1024U * 1024U)); },
    "FLASH_CHIP_SPEED",     []() -> String { return String(ESP.getFlashChipSpeed() / (1000U * 1000U)); },
    "FS_SIZE",              []() -> String { return String(FILESYSTEM.totalBytes()); },
    "FS_SIZE_USED",         []() -> String { return String(FILESYSTEM.usedBytes()); },
    "HEAP_SIZE",            []() -> String { return String(ESP.getHeapSize()); },
    "HEAP_SIZE_AVAILABLE",  []() -> String { return String(ESP.getFreeHeap()); },
    "PSRAM_SIZE",           []() -> String { return String(ESP.getPsramSize()); },
    "PSRAM_SIZE_AVAILABLE", []() -> String { return String(ESP.getFreePsram()); },
    "HOSTNAME",             tmpl::getHostname,
    "IPV4",                 tmpl::getIPAddress,
    "LWIP_VERSION",         []() -> String { return LWIP_VERSION_STRING; },
    "MAC_ADDR",             []() -> String { return WiFi.macAddress(); },
    "RSSI",                 tmpl::getRSSI,
    "SSID",                 tmpl::getSSID,
    "SW_BRANCH",            []() -> String { return Version::SOFTWARE_BRANCH; },
    "SW_REVISION",          []() -> String { return Version::SOFTWARE_REV; },
    "SW_VERSION",           []() -> String { return Version::SOFTWARE_VER; },
    "WS_ENDPOINT",          []() -> String { return WebConfig::WEBSOCKET_PATH; },
    "WS_PORT",              []() -> String { return String(WebConfig::WEBSOCKET_PORT); },
    "WS_PROTOCOL",          []() -> String { return WebConfig::WEBSOCKET_PROTOCOL; }
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
    const char* pluginName = nullptr;
    String      webLoginUser;
    String      webLoginPassword;

    if (false == Settings::getInstance().open(true))
    {
        webLoginUser        = Settings::getInstance().getWebLoginUser().getDefault();
        webLoginPassword    = Settings::getInstance().getWebLoginPassword().getDefault();
    }
    else
    {
        webLoginUser        = Settings::getInstance().getWebLoginUser().getValue();
        webLoginPassword    = Settings::getInstance().getWebLoginPassword().getValue();

        Settings::getInstance().close();
    }

    (void)srv.on("/about.html", HTTP_GET, aboutPage)
        .setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());
    (void)srv.on("/debug.html", HTTP_GET, debugPage)
        .setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());
    (void)srv.on("/display.html", HTTP_GET, displayPage)
        .setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());
    (void)srv.on("/edit.html", HTTP_GET, editPage)
        .setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());
    (void)srv.on("/index.html", HTTP_GET, indexPage)
        .setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());
    (void)srv.on("/info.html", HTTP_GET, infoPage)
        .setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());
    (void)srv.on("/settings.html", HTTP_GET | HTTP_POST, settingsPage)
        .setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());
    (void)srv.on("/update.html", HTTP_GET, updatePage)
        .setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());
    (void)srv.on("/upload.html", HTTP_POST, uploadPage, uploadHandler)
        .setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());

    (void)srv.on("/", [](AsyncWebServerRequest* request) {
        if (nullptr != request)
        {
            request->redirect("/index.html");
        }
    });

    /* Serve files with static content with disabled cache control. */
    (void)srv.serveStatic("/configuration/", FILESYSTEM, "/configuration/")
        .setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());

    /* Serve files with static content with enabled cache control.
     * The client may cache files from filesystem for 1 hour.
     */
    (void)srv.serveStatic("/favicon.png", FILESYSTEM, "/favicon.png", "max-age=3600")
        .setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());
    (void)srv.serveStatic("/images/", FILESYSTEM, "/images/", "max-age=3600")
        .setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());
    (void)srv.serveStatic("/js/", FILESYSTEM, "/js/", "max-age=3600")
        .setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());
    (void)srv.serveStatic("/style/", FILESYSTEM, "/style/", "max-age=3600")
        .setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());

    /* Add one page per plugin. */
    pluginName = PluginMgr::getInstance().findFirst();
    while(nullptr != pluginName)
    {
        String uri = fitToSpiffs(PLUGIN_PAGE_PATH, String(pluginName), PLUGIN_PAGE_FILE_EXTENSION);

        (void)srv.on(   uri.c_str(),
                        HTTP_GET,
                        [uri](AsyncWebServerRequest* request)
                        {
                            if (nullptr == request)
                            {
                                return;
                            }

                            request->send(FILESYSTEM, uri, "text/html", false, tmplPageProcessor);

                        }).setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());;

        pluginName = PluginMgr::getInstance().findNext();
    }

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

    LOG_INFO("Invalid web request: %s", request->url().c_str());

    request->send(FILESYSTEM, "/error.html", "text/html", false, tmplPageProcessor);

    return;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * SPIFFS full filename (path + filename + extension) is limited to 32 characters.
 * This function reduces only the filename length and returns the full path.
 *
 * @param[in] path                  Path, e.g. "/mypath/".
 * @param[in] fileNameWithoutExt    Filename without extension, e.g. "myFile".
 * @param[in] fileNameExtension     Filename extension, e.g. ".html"
 *
 * @return Full path
 */
static String fitToSpiffs(const String& path, const String& filenNameWithoutExt, const String& fileNameExtension)
{
    String fileNameReduced = filenNameWithoutExt.substring(0, SPIFFS_FILENAME_LENGTH_LIMIT - path.length() - fileNameExtension.length() - 1U);

    return path + fileNameReduced + fileNameExtension;
}

/**
 * Processor for page template, containing the common part, which is available
 * in every page. It is responsible for the data binding.
 *
 * @param[in] var   Name of variable in the template
 */
static String tmplPageProcessor(const String& var)
{
    String  result;
    uint8_t index   = 0U;
    bool    isFound = false;

    while ((index < UTIL_ARRAY_NUM(gTmplKeyWordToFunc)) && (false == isFound))
    {
        if (var == gTmplKeyWordToFunc[index].keyword)
        {
            result  = gTmplKeyWordToFunc[index].func();
            isFound = true;
        }

        ++index;
    }

    if (false == isFound)
    {
        result = var;
    }

    return result;
}

/**
 * About page, showing the log output on demand.
 *
 * @param[in] request   HTTP request
 */
static void aboutPage(AsyncWebServerRequest* request)
{
    if (nullptr == request)
    {
        return;
    }

    request->send(FILESYSTEM, "/about.html", "text/html", false, tmplPageProcessor);

    return;
}

/**
 * Debug page, showing the log output on demand.
 *
 * @param[in] request   HTTP request
 */
static void debugPage(AsyncWebServerRequest* request)
{
    if (nullptr == request)
    {
        return;
    }

    request->send(FILESYSTEM, "/debug.html", "text/html", false, tmplPageProcessor);

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

    request->send(FILESYSTEM, "/display.html", "text/html", false, tmplPageProcessor);

    return;
}

/**
 * File edit page.
 *
 * @param[in] request   HTTP request
 */
static void editPage(AsyncWebServerRequest* request)
{
    if (nullptr == request)
    {
        return;
    }

    request->send(FILESYSTEM, "/edit.html", "text/html", false, tmplPageProcessor);

    return;
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

    request->send(FILESYSTEM, "/index.html", "text/html", false, tmplPageProcessor);

    return;
}

/**
 * Info page shows general informations.
 *
 * @param[in] request   HTTP request
 */
static void infoPage(AsyncWebServerRequest* request)
{
    if (nullptr == request)
    {
        return;
    }

    request->send(FILESYSTEM, "/info.html", "text/html", false, tmplPageProcessor);

    return;
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

    request->send(FILESYSTEM, "/settings.html", "text/html", false, tmplPageProcessor);

    return;
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

    request->send(FILESYSTEM, "/update.html", "text/html", false, tmplPageProcessor);

    return;
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

    if (true == gIsUploadError)
    {
        request->send(HttpStatus::STATUS_CODE_BAD_REQUEST, "text/plain", "Error");
    }
    else
    {
        request->send(HttpStatus::STATUS_CODE_OK, "text/plain", "Ok");
    }

    /* Trigger restart after the client has disconnected.
     * Do this in every case to ensure that if there was any error, the
     * device will be restarted as well.
     */
    request->onDisconnect(
        []()
        {
            UpdateMgr::getInstance().reqRestart();
        }
    );

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
        uint32_t fileSize = UPDATE_SIZE_UNKNOWN;

        /* If there is a pending upload, abort it. */
        if (true == Update.isRunning())
        {
            Update.abort();
            LOG_WARNING("Pending upload aborted.");
        }

        /* Upload firmware or filesystem? */
        int cmd = (filename == FILESYSTEM_FILENAME) ? U_SPIFFS : U_FLASH;

        if (U_FLASH == cmd)
        {
            AsyncWebHeader* headerXFileSizeFirmware = request->getHeader("X-File-Size-Firmware");

            /* Firmware file size available? */
            if (nullptr != headerXFileSizeFirmware)
            {
                /* If conversion fails, it will contain UPDATE_SIZE_UNKNOWN. */
                (void)Util::strToUInt32(headerXFileSizeFirmware->value(), fileSize);
            }
        }
        else if (U_SPIFFS == cmd)
        {
            AsyncWebHeader* headerXFileSizeFilesystem = request->getHeader("X-File-Size-Filesystem");

            /* Firmware file size available? */
            if (nullptr != headerXFileSizeFilesystem)
            {
                /* If conversion fails, it will contain UPDATE_SIZE_UNKNOWN. */
                (void)Util::strToUInt32(headerXFileSizeFilesystem->value(), fileSize);
            }
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
            /* Close filesystem before continue. */
            FILESYSTEM.end();
        }

        /* Start update */
        if (false == Update.begin(fileSize, cmd))
        {
            LOG_ERROR("Upload failed: %s", Update.errorString());
            gIsUploadError = true;

            /* Mount filesystem again, it may be unmounted in case of filesystem update.*/
            if (false == FILESYSTEM.begin())
            {
                LOG_FATAL("Couldn't mount filesystem.");
            }

            /* Inform client about abort.*/
            request->send(HttpStatus::STATUS_CODE_PAYLOAD_TOO_LARGE, "text/plain", "Upload aborted.");
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

                    /* Restart is requested in upload page handler, see uploadPage(). */
                }
            }
        }
        else
        {
            /* Mount filesystem again, it may be unmounted in case of filesystem update. */
            if (false == FILESYSTEM.begin())
            {
                LOG_FATAL("Couldn't mount filesystem.");
            }

            /* Abort update */
            Update.abort();
            UpdateMgr::getInstance().endProgress();

            /* Inform client about abort.*/
            request->send(HttpStatus::STATUS_CODE_PAYLOAD_TOO_LARGE, "text/plain", "Upload aborted.");
        }
    }

    return;
}

/**
 * Functions which are called for the corresponding template keyword.
 */
namespace tmpl
{
    /**
     * Get ESP chip id.
     *
     * @return ESP chip id
     */
    static String getEspChipId()
    {
        String      result;
        uint64_t    chipId      = ESP.getEfuseMac();
        uint32_t    highPart    = (chipId >> 32U) & 0x0000ffffU;
        uint32_t    lowPart     = (chipId >>  0U) & 0xffffffffU;
        char        chipIdStr[13];

        (void)snprintf(chipIdStr, UTIL_ARRAY_NUM(chipIdStr), "%04X%08X", highPart, lowPart);

        result = chipIdStr;

        return result;
    }

    /**
     * Get ESP type.
     *
     * @return ESP type
     */
    static String getEspType()
    {
        String result = CONFIG_IDF_TARGET;

        return result;
    }

    /**
     * Get flash chip mode.
     *
     * @return Flash chip mode.
     */
    static String getFlashChipMode()
    {
        String result;

        switch(ESP.getFlashChipMode())
        {
        case FM_QIO:
            result = "QUIO";
            break;

        case FM_QOUT:
            result = "QOUT";
            break;

        case FM_DIO:
            result = "DIO";
            break;

        case FM_DOUT:
            result = "DOUT";
            break;

        case FM_FAST_READ:
            result = "FAST_READ";
            break;

        case FM_SLOW_READ:
            result = "SLOW_READ";
            break;

        case FM_UNKNOWN:
            /* fallthrough */

        default:
            result = "UNKNOWN";
            break;
        }

        return result;
    }

    /**
     * Get hostname, depended on current WiFi mode.
     *
     * @return Hostname
     */
    static String getHostname()
    {
        String      result;
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

        return result;
    }

    /**
     * Get IP address, depended on WiFi mode.
     *
     * @return IPv4
     */
    static String getIPAddress()
    {
        String result;

        if (WIFI_MODE_AP == WiFi.getMode())
        {
            result = WiFi.softAPIP().toString();
        }
        else
        {
            result = WiFi.localIP().toString();
        }

        return result;
    }

    /**
     * Get wifi RSSI.
     *
     * @return WiFi station SSID
     */
    static String getRSSI()
    {
        String result;

        /* Only in station mode it makes sense to retrieve the RSSI.
         * Otherwise keep it -100 dbm.
         */
        if (WIFI_MODE_STA == WiFi.getMode())
        {
            result = WiFi.RSSI();
        }
        else
        {
            result = "-100";
        }

        return result;
    }

    /**
     * Get wifi station SSID.
     *
     * @return WiFi station SSID
     */
    static String getSSID()
    {
        String result;

        if (true == Settings::getInstance().open(true))
        {
            result = Settings::getInstance().getWifiSSID().getValue();
            Settings::getInstance().close();
        }

        return result;
    }
};

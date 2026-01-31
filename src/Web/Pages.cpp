/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   Pages.cpp
 * @brief  Web pages
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Pages.h"
#include "WebConfig.h"
#include "Version.h"
#include "DisplayMgr.h"
#include "RestApi.h"
#include "PluginList.h"
#include "Services.h"
#include "WiFiUtil.h"

#include <WiFi.h>
#include <Esp.h>
#include <Logging.h>
#include <MemUtil.h>
#include <Util.h>
#include <ArduinoJson.h>
#include <lwip/init.h>
#include <SettingsService.h>
#include <FileSystem.h>
#include <BitmapWidget.h>

#include <mbedtls/version.h>
#include <freertos/task.h>

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
    const char* keyword;  /**< Keyword */
    String (*func)(void); /**< Function to call */
};

/**
 * Single HTML page route.
 */
struct HtmlPageRoute
{
    const char*               page;               /**< Page in the filesystem. */
    WebRequestMethodComposite reqMethodComposite; /**< Request method composite */
};

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static String tmplPageProcessor(const String& var);
static void   htmlPage(AsyncWebServerRequest* request);

namespace tmpl
{
static String getEspChipId();
static String getFlashChipMode();
static String getHostname();
static String getIPAddress();
static String getImageFileExtensions();
}; /* namespace tmpl */

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Path to the plugin webpages. */
static const String PLUGIN_PAGE_PATH              = "/plugins/";

/** Path to the service webpages. */
static const String SERVICE_PAGE_PATH             = "/services/";

/**
 * List of all used template keywords and the function how to retrieve the information.
 * The list is alphabetic sorted in ascending order.
 */
static const TmplKeyWordFunc gTmplKeyWordToFunc[] = {
    { "DISPLAY_HEIGHT", []() -> String { return String(CONFIG_LED_MATRIX_HEIGHT); } },
    { "DISPLAY_WIDTH", []() -> String { return String(CONFIG_LED_MATRIX_WIDTH); } },
    { "ARDUINO_IDF_BRANCH", []() -> String { return CONFIG_ARDUINO_IDF_BRANCH; } },
    { "ESP_CHIP_ID", tmpl::getEspChipId },
    { "ESP_CHIP_REV", []() -> String { return String(ESP.getChipRevision()); } },
    { "ESP_CPU_FREQ", []() -> String { return String(ESP.getCpuFreqMHz()); } },
    { "ESP_SDK_VERSION", []() -> String { return ESP.getSdkVersion(); } },
    { "ESP_TYPE", []() -> String { return String(CONFIG_IDF_TARGET); } },
    { "FLASH_CHIP_MODE", tmpl::getFlashChipMode },
    { "FLASH_CHIP_SIZE", []() -> String { return String(ESP.getFlashChipSize() / (1024U * 1024U)); } },
    { "FLASH_CHIP_SPEED", []() -> String { return String(ESP.getFlashChipSpeed() / (1000U * 1000U)); } },
    { "FREERTOS_VERSION", []() -> String { return tskKERNEL_VERSION_NUMBER; } },
    { "FS_SIZE", []() -> String { return String(FILESYSTEM.totalBytes()); } },
    { "FS_SIZE_USED", []() -> String { return String(FILESYSTEM.usedBytes()); } },
    { "HEAP_SIZE", []() -> String { return String(MemUtil::getTotalHeapSize()); } },
    { "HEAP_SIZE_AVAILABLE", []() -> String { return String(MemUtil::getFreeHeapSize()); } },
    { "IMAGE_FILE_EXTENSIONS", []() -> String { return tmpl::getImageFileExtensions(); } },
    { "MBED_TLS_VERSION", []() -> String { return String(MBEDTLS_VERSION_STRING); } },
    { "PSRAM_SIZE", []() -> String { return String(ESP.getPsramSize()); } },
    { "PSRAM_SIZE_AVAILABLE", []() -> String { return String(ESP.getFreePsram()); } },
    { "HOSTNAME", tmpl::getHostname },
    { "IPV4", tmpl::getIPAddress },
    { "LWIP_VERSION", []() -> String { return LWIP_VERSION_STRING; } },
    { "MAC_ADDR", []() -> String { return WiFi.macAddress(); } },
    { "RSSI", []() -> String { return String(WiFi.RSSI()); } },
    { "SSID", []() -> String { return WiFi.SSID(); } },
    { "SW_BRANCH", []() -> String { return Version::getSoftwareBranchName(); } },
    { "SW_REVISION", []() -> String { return Version::getSoftwareRevision(); } },
    { "SW_VERSION", []() -> String { return Version::getSoftwareVersion(); } },
    { "TARGET", []() -> String { return Version::getTargetName(); } },
    { "WS_ENDPOINT", []() -> String { return WebConfig::WEBSOCKET_PATH; } },
    { "WS_PORT", []() -> String { return String(WebConfig::WEBSOCKET_PORT); } },
    { "WS_PROTOCOL", []() -> String { return WebConfig::WEBSOCKET_PROTOCOL; } }
};

/**
 * Standard HTML page routes.
 */
static const HtmlPageRoute gHtmlPageRoutes[] = {
    { "/about.html", HTTP_GET },
    { "/debug.html", HTTP_GET },
    { "/display.html", HTTP_GET },
    { "/edit.html", HTTP_GET },
    { "/icons.html", HTTP_GET },
    { "/index.html", HTTP_GET },
    { "/info.html", HTTP_GET },
    { "/settings.html", HTTP_GET | HTTP_POST },
    { "/update.html", HTTP_GET }
};

/**
 * Static routes to files with enabled cache.
 */
static const char* gStaticRoutesWithCache[] = {
    "/favicon.png",
    "/images/",
    "/js/",
    "/style/"
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
    uint8_t                    pluginTypeListLength = 0U;
    const PluginList::Element* pluginTypeList       = PluginList::getList(pluginTypeListLength);
    uint8_t                    serviceListLength    = 0U;
    const Services::Element*   serviceList          = Services::getList(serviceListLength);
    uint8_t                    idx                  = 0U;
    String                     webLoginUser;
    String                     webLoginPassword;
    SettingsService&           settings = SettingsService::getInstance();

    if (false == settings.open(true))
    {
        webLoginUser     = settings.getWebLoginUser().getDefault();
        webLoginPassword = settings.getWebLoginPassword().getDefault();
    }
    else
    {
        webLoginUser     = settings.getWebLoginUser().getValue();
        webLoginPassword = settings.getWebLoginPassword().getValue();

        settings.close();
    }

    /* Serve standard HTML pages. */
    while (UTIL_ARRAY_NUM(gHtmlPageRoutes) > idx)
    {
        const HtmlPageRoute& route = gHtmlPageRoutes[idx];

        (void)srv.on(route.page, route.reqMethodComposite, htmlPage)
            .setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());

        ++idx;
    }

    /* Redirect root folder access to index.html page. */
    (void)srv.on("/", [](AsyncWebServerRequest* request) {
        if (nullptr != request)
        {
            request->redirect("/index.html");
        }
    });

    /* Serve files with volatile content with disabled cache control. */
    (void)srv.serveStatic("/configuration/", FILESYSTEM, "/configuration/")
        .setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());

    /* Serve files with static content with enabled cache control.
     * The client may cache files from filesystem for 1 hour.
     */
    idx = 0U;
    while (UTIL_ARRAY_NUM(gStaticRoutesWithCache) > idx)
    {
        const char* route = gStaticRoutesWithCache[idx];

        (void)srv.serveStatic(route, FILESYSTEM, route, "max-age=3600")
            .setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());

        ++idx;
    }

    /* Add one page per plugin. */
    idx = 0U;
    while (pluginTypeListLength > idx)
    {
        const PluginList::Element* elem = &pluginTypeList[idx];
        String                     uri  = PLUGIN_PAGE_PATH + elem->name;

        (void)srv.on(uri.c_str(),
                     HTTP_GET,
                     [](AsyncWebServerRequest* request) {
                         if (nullptr == request)
                         {
                             return;
                         }

                         if (0U != request->url().endsWith(".html"))
                         {
                             request->send(FILESYSTEM, request->url(), "text/html", false, tmplPageProcessor);
                         }
                         else
                         {
                             request->send(FILESYSTEM, request->url());
                         }
                     })
            .setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());

        ++idx;
    }

    /* Add one page per service. */
    idx = 0U;
    while (serviceListLength > idx)
    {
        const Services::Element* elem = &serviceList[idx];
        String                   uri  = SERVICE_PAGE_PATH + elem->name;

        (void)srv.on(uri.c_str(),
                     HTTP_GET,
                     [](AsyncWebServerRequest* request) {
                         if (nullptr == request)
                         {
                             return;
                         }

                         if (0U != request->url().endsWith(".html"))
                         {
                             request->send(FILESYSTEM, request->url(), "text/html", false, tmplPageProcessor);
                         }
                         else
                         {
                             request->send(FILESYSTEM, request->url());
                         }
                     })
            .setAuthentication(webLoginUser.c_str(), webLoginPassword.c_str());

        ++idx;
    }
}

void Pages::error(AsyncWebServerRequest* request)
{
    if (nullptr == request)
    {
        return;
    }

    LOG_INFO("Invalid web request: %s", request->url().c_str());

    request->send(FILESYSTEM, "/error.html", "text/html", false, tmplPageProcessor);
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Processor for page template, containing the common part, which is available
 * in every page. It is responsible for the data binding.
 *
 * @param[in] var   Name of variable in the template
 *
 * @return The variable content.
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
 * Standard HTML page with template page processor applied.
 *
 * @param[in] request   HTTP request
 */
static void htmlPage(AsyncWebServerRequest* request)
{
    if (nullptr == request)
    {
        return;
    }

    request->send(FILESYSTEM, request->url(), "text/html", false, tmplPageProcessor);
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
    String chipId;

    /* Chip id is the same as the factory programmed wifi MAC address. */
    WiFiUtil::getChipId(chipId);

    return chipId;
}

/**
 * Get flash chip mode.
 *
 * @return Flash chip mode.
 */
static String getFlashChipMode()
{
    String result;

    switch (ESP.getFlashChipMode())
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
 * Get supported image file extensions for BitmapWidget.
 * The result is a comma separated list of extensions, each starting with a dot.
 *
 * @return Comma separated list of supported image file extensions.
 */
static String getImageFileExtensions()
{
    String  result;
    uint8_t idx;

    for (idx = 0; idx < BitmapWidget::IMAGE_FILE_EXTENSIONS_COUNT; ++idx)
    {
        if (0U < idx)
        {
            result += ",";
        }

        result += ".";
        result += BitmapWidget::IMAGE_FILE_EXTENSIONS[idx];
    }

    return result;
}

}; /* namespace tmpl */

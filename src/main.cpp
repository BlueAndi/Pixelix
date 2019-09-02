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
@brief  Main entry point
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
This module provides the main entry point. It setup the whole system and
has the main loop.

*******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include "Board.h"
#include "ButtonDrv.h"
#include "Settings.h"
#include "LedMatrix.h"

#include <WiFi.h>

#include <ArduinoOTA.h>
#include <SPIFFS.h>

#include "WebServer.h"
#include "WebConfig.h"
#include "Pages.h"
#include "RestApi.h"

#include "DisplayMgr.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Get number of array elements. */
#define ARRAY_NUM(__arr)    (sizeof(__arr) / sizeof((__arr)[0]))

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static void showBootInfo(void);
static void otaOnStart(void);
static void otaOnEnd(void);
static void otaOnProgress(unsigned int progress, unsigned int total);
static void otaOnError(ota_error_t error);

/******************************************************************************
 * Variables
 *****************************************************************************/

/** Current SW version */
static const char           SW_VERSION[]            = "Trunk";

/** Serial interface baudrate. */
static const uint32_t       SERIAL_BAUDRATE         = 115200u;

/** Access point SSID */
static const char           WIFI_AP_SSID[]          = "esp32-rgb-led-matrix";

/** Access point passphrase (min. 8 characters) */
static const char           WIFI_AP_PASSPHRASE[]    = "Luke, I am your father.";

/** If a fatal error happened, it shall be set true otherwise false. */
static bool                 gIsFatalError           = false;

/** Retry delay after a failed connection attempt in ms. */
static const uint32_t       RETRY_DELAY             = 30000u;

/** Over-the-air update password */
static const char           OTA_PASSWORD[]          = "maytheforcebewithyou";

/** Is over-the-air update started? */
static bool                 gisUpdateStarted        = false;

/** Web server */
static WebServer            gWebServer(WebConfig::WEBSERVER_PORT);

/** Standard wait time for showing a system message in ms */
static const uint32_t       SYS_MSG_WAIT_TIME_STD   = 2000u;

/** Short wait time for showing a system message in ms */
static const uint32_t       SYS_MSG_WAIT_TIME_SHORT = 250u;

/** The number of display pixels, showing the current OTA progress. */
static uint16_t             gOtaProgress            = 0u;

/******************************************************************************
 * External functions
 *****************************************************************************/

/**
 * Setup the system.
 */
void setup()
{
    bool        isAPMode    = false;
    DisplayMgr& dispMgr     = DisplayMgr::getInstance();
    uint8_t     index       = 0u;

    /* Initialize hardware */
    Board::init();

    /* Setup serial interface */
    Serial.begin(SERIAL_BAUDRATE);

    /* Initialize drivers */
    ButtonDrv::getInstance().init();

    /* Start LED matrix */
    LedMatrix::getInstance().begin();

    /* Show some interesting boot information */    
    showBootInfo();

    /* Initialize display layout */
    for(index = 0u; index < DisplayMgr::MAX_SLOTS; ++index)
    {
        dispMgr.setLayout(index, DisplayMgr::LAYOUT_ID_2);
    }

    /* Does the user request for setting up an wifi access point?
     * Because we just initialized the button driver, wait until
     * the button state has a reliable value.
     */
    while(false == ButtonDrv::getInstance().isUpdated())
    {
        /* Give other tasks a chance. */
        delay(1u);
    }

    if (ButtonDrv::STATE_PRESSED == ButtonDrv::getInstance().getState())
    {
        /* Setup wifi access point. */
        if (false == WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSPHRASE))
        {
            /* Fatal error */
            gIsFatalError = true;

            Serial.println("Setup wifi access point failed.");
            dispMgr.showSysMsg("Setup wifi access point failed.");
        }
        else
        {
            /* Show SSID on the LED marix */
            dispMgr.showSysMsg(String("SSID: ") + WIFI_AP_SSID);

            /* Show SSID on the serial interface */
            Serial.print("SSID: ");
            Serial.println(WIFI_AP_SSID);

            isAPMode = true;
        }
    }
    else
    {
        String  wifiSSID;
        String  wifiPassphrase;

        /* Are remote wifi network informations available? */
        if (true == Settings::getInstance().open(true))
        {
            wifiSSID        = Settings::getInstance().getWifiSSID();
            wifiPassphrase  = Settings::getInstance().getWifiPassphrase();

            Settings::getInstance().close();
        }

        if ((0 == wifiSSID.length()) ||
            (0 == wifiPassphrase.length()))
        {
            /* No remote wifi network informations available. */
            gIsFatalError = true;

            Serial.println("Remote wifi SSID/password missing.");
            Serial.println("Keep button pressed and reboot. Set SSID/password via webserer.");
            dispMgr.showSysMsg("Keep button pressed and reboot. Set SSID/password via webserer.");
        }
        else
        {
            wl_status_t status = WL_IDLE_STATUS;

            /* Remote wifi network informations are available, try to establish a connection. */
            while(WL_CONNECTED != status)
            {
                Serial.print("Connecting to ");
                Serial.println(wifiSSID);
                dispMgr.showSysMsg(String("Connecting to ") + wifiSSID);

                status = WiFi.begin(wifiSSID.c_str(), wifiPassphrase.c_str());

                if (WL_CONNECTED != status)
                {
                    Serial.println("Connection failed.");
                    dispMgr.showSysMsg("Connection failed.");

                    dispMgr.delay(RETRY_DELAY);
                }
                
                /* TODO How to determine whats wrong? SSID or password? */
            }

            /* Enable automatic reconnect in case the connection gets lost. */
            WiFi.setAutoReconnect(true);
        }
    }

    /* Continoue only if no error happened. */
    if (false == gIsFatalError)
    {
        /* Start webserver and register all web pages */
        gWebServer.begin();
        Pages::init(gWebServer);
        RestApi::init(gWebServer);

        Serial.print("Hostname: ");
        Serial.println(WiFi.getHostname());

        /* Access point active? */
        if (true == isAPMode)
        {
            IPAddress ipAddress = WiFi.softAPIP();

            /* Show the ip address on the LED matrix */
            dispMgr.showSysMsg(String(ipAddress));

            /* Show the ip address on the debug terminal */
            Serial.print("AP IP address: ");
            Serial.println(ipAddress);
        }

        /* Prepare over the air update. */
        ArduinoOTA.begin();
        ArduinoOTA.setPassword(OTA_PASSWORD);
        ArduinoOTA.onStart(otaOnStart);
        ArduinoOTA.onEnd(otaOnEnd);
        ArduinoOTA.onProgress(otaOnProgress);
        ArduinoOTA.onError(otaOnError);

        Serial.print("OTA hostname: ");
        Serial.println(ArduinoOTA.getHostname());
        Serial.print("Sketch size: ");
        Serial.print(ESP.getSketchSize());
        Serial.println(" bytes");
        Serial.print("Free size: ");
        Serial.print(ESP.getFreeSketchSpace());
        Serial.println(" bytes");

        /* Enable slots */
        dispMgr.enableSlots(true);
        dispMgr.startRotating(true);
    }

    return;
}

/**
 * Main loop, which is called periodically.
 */
void loop()
{
    /* If a fatal error happened, wait till manual reset. */
    if (true == gIsFatalError)
    {
        /* Wait till manual reset. */
        ;
    }
    /* Normal operation */
    else
    {
        ArduinoOTA.handle();

        /* As long as no update is running, do handle all other connections. */
        if (false == gisUpdateStarted)
        {
            gWebServer.handleClient();
        }

        DisplayMgr::getInstance().process();
    }

    return;
}

/******************************************************************************
 * Local functions
 *****************************************************************************/

/**
 * Show boot information on the serial interface.
 */
static void showBootInfo(void)
{
    /* Show information via serial interface */    
    Serial.println("Booting ...");
    
    Serial.print("SW version: ");
    Serial.println(SW_VERSION);
    DisplayMgr::getInstance().showSysMsg(SW_VERSION);

    Serial.print("ESP32 chip rev.: ");
    Serial.println(ESP.getChipRevision());

    Serial.print("ESP32 SDK version: ");
    Serial.println(ESP.getSdkVersion());

    /* User shall be able to read it on the display. But it shall be really a short delay. */
    DisplayMgr::getInstance().delay(SYS_MSG_WAIT_TIME_SHORT);

    return;
}

/**
 * On start of over-the-air update.
 */
static void otaOnStart(void)
{
    gisUpdateStarted = true;

    Serial.print("Start updating ");

    if (U_FLASH == ArduinoOTA.getCommand())
    {
        Serial.println("sketch.");
        DisplayMgr::getInstance().showSysMsg("Update sketch.");
    }
    else
    {
        Serial.println("filesystem.");
        DisplayMgr::getInstance().showSysMsg("Update filesystem.");

        /* Close filesystem before continue. 
         * Note, this needs a restart after update is finished.
         */
        SPIFFS.end();
    }

    /* Give the user a chance to read it. */
    DisplayMgr::getInstance().delay(SYS_MSG_WAIT_TIME_STD);

    /* Prepare to show the progress in the next steps. */
    LedMatrix::getInstance().clear();

    return;
}

/**
 * On end of over-the-air update.
 */
static void otaOnEnd(void)
{
    gisUpdateStarted = false;

    Serial.println("Update successful finished.");
    DisplayMgr::getInstance().showSysMsg("Update successful.");

    /* Give the user a chance to read it. */
    DisplayMgr::getInstance().delay(SYS_MSG_WAIT_TIME_STD);

    ESP.restart();

    return;
}

/**
 * On progress of over-the-air update.
 * 
 * @param[in] progress  Number of written bytes.
 * @param[in] total     Total size of the whole binary, which to update.
 */
static void otaOnProgress(unsigned int progress, unsigned int total)
{
    const uint32_t  PROGRESS_PERCENT    = (progress * 100u) / total;
    const uint32_t  PIXELS              = Board::LedMatrix::width * Board::LedMatrix::heigth;
    uint32_t        pixelProgress       = (PIXELS * PROGRESS_PERCENT) / 100u;
    uint32_t        delta               = pixelProgress - gOtaProgress;
    int16_t         y                   = gOtaProgress / Board::LedMatrix::width;
    int16_t         x                   = gOtaProgress % Board::LedMatrix::width;
    const uint16_t  COLOR               = 0xF800;   /* Red */

    Serial.printf("Progress: %u%%\r\n", PROGRESS_PERCENT);

    /* Fill the whole display.
     * The number of pixels equals 100% update progress.
     */
    while(0u < delta)
    {
        LedMatrix::getInstance().writePixel(x, y, COLOR);
        ++x;

        if (Board::LedMatrix::width <= x)
        {
            x = 0;
            ++y;
        }

        --delta;
    }

    LedMatrix::getInstance().show();

    gOtaProgress = pixelProgress;

    return;
}

/**
 * On error of over-the-air update.
 * 
 * @param[in] error Error information
 */
static void otaOnError(ota_error_t error)
{
    gisUpdateStarted    = false;
    gIsFatalError       = true;

    Serial.print("Update failed: ");  
    
    switch(error)
    {
    case OTA_AUTH_ERROR:
        Serial.println("OTA - Authentication error.");
        DisplayMgr::getInstance().showSysMsg("OTA - Authentication error.");
        break;

    case OTA_BEGIN_ERROR:
        Serial.println("OTA - Begin error.");
        DisplayMgr::getInstance().showSysMsg("OTA - Begin error.");
        break;

    case OTA_CONNECT_ERROR:
        Serial.println("OTA - Connect error.");
        DisplayMgr::getInstance().showSysMsg("OTA - Connect error.");
        break;

    case OTA_RECEIVE_ERROR:
        Serial.println("OTA - Receive error.");
        DisplayMgr::getInstance().showSysMsg("OTA - Receive error.");
        break;

    case OTA_END_ERROR:
        Serial.println("OTA - End error.");
        DisplayMgr::getInstance().showSysMsg("OTA - End error.");
        break;

    default:
        Serial.println("OTA - Unknown error.");
        DisplayMgr::getInstance().showSysMsg("OTA - Unknown error.");
        break;
    }

    /* Give the user a chance to read it. */
    DisplayMgr::getInstance().delay(SYS_MSG_WAIT_TIME_STD);

    ESP.restart();

    return;
}

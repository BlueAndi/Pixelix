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

/** FastLED RMT driver shall use only one channel to avoid wasting time and memory. */
#define FASTLED_RMT_MAX_CHANNELS    1
#include "FastLED.h"

#include <WiFi.h>
#include <WebServer.h>

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

static bool handleAuthentication();
static void handleRoot();

/******************************************************************************
 * Variables
 *****************************************************************************/

/** Serial interface baudrate. */
static const uint32_t   SERIAL_BAUDRATE     = 115200u;

/** Pixel representation of the LED matrix */
static CRGB             gLedMatrix[Board::LedMatrix::width * Board::LedMatrix::heigth];

/** Access point SSID */
static const char*      WIFI_AP_SSID        = "esp32-rgb-led-matrix";

/** Access point passphrase (min. 8 characters) */
static const char*      WIFI_AP_PASSPHRASE  = "Luke, I am your father.";

/** Port for HTTP */
static const uint32_t   WEBSERVER_PORT      = 80u;

/** HTTP server */
static WebServer        gWebServer(WEBSERVER_PORT);

/** Webserver login user */
static const char*      WEB_LOGIN_USER      = "luke";

/** Webserver login password */
static const char*      WEB_LOGIN_PASSWORD  = "skywalker";

/** If a fatal error happened, it shall be set true otherwise false. */
static bool             gIsFatalError       = false;

/** Retry delay after a failed connection attempt in ms. */
static const uint32_t   RETRY_DELAY         = 30000u;

/******************************************************************************
 * External functions
 *****************************************************************************/

/**
 * Setup the system.
 */
void setup()
{
    bool isAPMode = false;

    /* Initialize hardware */
    Board::init();

    /* Setup serial interface */
    Serial.begin(SERIAL_BAUDRATE);
    
    /* TODO */
    Serial.println("Booting ...");

    /* Initialize drivers */
    ButtonDrv::getInstance().init();

    /* Setup LED matrix and limit max. power. */
    FastLED.addLeds<NEOPIXEL, Board::Pin::ledMatrixDataOutPinNo>(gLedMatrix, ARRAY_NUM(gLedMatrix)).setCorrection(TypicalLEDStrip);
    FastLED.setMaxPowerInVoltsAndMilliamps(Board::LedMatrix::supplyVoltage, Board::LedMatrix::supplyCurrentMax);

    /* User request for setting up an wifi access point?
     * Because we just initialized the button driver, the delay
     * ensures that a reliable value can be read the first time.
     */
    delay(250u);
    if (ButtonDrv::STATE_PRESSED == ButtonDrv::getInstance().getState())
    {
        /* Setup wifi access point. */
        if (false == WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSPHRASE))
        {
            /* Fatal error */
            /* TODO */
            gIsFatalError = true;
        }
        else
        {
            /* Show SSID on the LED marix */
            /* TODO */

            /* Show SSID on the serial interface */
            /* TODO */
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
        wifiSSID        = Settings::getInstance().getWifiSSID();
        wifiPassphrase  = Settings::getInstance().getWifiPassphrase();

        if ((0 == wifiSSID.length()) ||
            (0 == wifiPassphrase.length()))
        {
            /* No remote wifi network informations available. */
            /* TODO Show info on LED matrix. */
            /* TODO Show info on serial interface. */
            gIsFatalError = true;
        }
        else
        {
            wl_status_t status = WL_IDLE_STATUS;

            /* Remote wifi network informations are available, try to establish a connection. */
            while(WL_CONNECTED != status)
            {
                /* TODO Show info on LED matrix. */
                /* TODO Show info on serial interface. */
                status = WiFi.begin(wifiSSID.c_str(), wifiPassphrase.c_str());

                if (WL_CONNECTED != status)
                {
                    delay(RETRY_DELAY);
                }
                
                /* TODO How to determine whats wrong? SSID or password? */
            }
        }
    }

    /* Continoue only if no error happened. */
    if (false == gIsFatalError)
    {
        /* Start webserver */
        gWebServer.begin();

        /* Access point active? */
        if (true == isAPMode)
        {
            IPAddress ipAddress = WiFi.softAPIP();

            gWebServer.on("/", handleRoot);

            /* Show the ip address on the LED matrix */
            /* TODO */

            /* Show the ip address on the debug terminal */
            /* TODO */
            Serial.print("AP IP address: ");
            Serial.println(ipAddress);
        }
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
    else
    /* Normal operation */
    {
        gWebServer.handleClient();

        /* TODO Handle unexpected disconnect from wifi network */
    }

    return;
}

/******************************************************************************
 * Local functions
 *****************************************************************************/

/**
 * Handle webserver authentication. If the client is not authenticated, an
 * authenticatio process will be performed.
 * 
 * @return If authentication is successful it returns true otherwise false.
 * @retval false    Authentication failed.
 * @retval true     Authentication successful.
 */
static bool handleAuthentication()
{
    bool status = true;

    /* If there is no authentication with the client, it will be requested. */
    if (false == gWebServer.authenticate(WEB_LOGIN_USER, WEB_LOGIN_PASSWORD))
    {
        const String authFailResponse = "Authentication failed!";

        /* Use encrypted communication for authentication request to avoid
         * that the credentials can be read by everyone.
         */
        gWebServer.requestAuthentication(DIGEST_AUTH, NULL, authFailResponse);
        
        status = false;
    }

    return status;
}

/**
 * Handle webserver "/" access.
 */
static void handleRoot()
{
    /* Perform authentication */
    if (false == handleAuthentication())
    {
        /* Authentication failed. */
        return;
    }

    gWebServer.send(200, "text/plain", "Root directory");

    return;
}

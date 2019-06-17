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
static const uint32_t   gSerialBaudrate     = 115200u;

/** Pixel representation of the LED matrix */
static CRGB             gLedMatrix[Board::LedMatrix::width * Board::LedMatrix::heigth];

/** Access point SSID */
static const char*      gAPSSID             = "esp32-rgb-led-matrix";

/** Access point passphrase (min. 8 characters) */
static const char*      gAPPassphrase       = "Luke, I am your father.";

/** Port for HTTP */
static const uint32_t   gWebServerPort      = 80u;

/** HTTP server */
static WebServer        gWebServer(gWebServerPort);

/** Webserver login user */
static const char*      gWebLoginUser       = "luke";

/** Webserver login password */
static const char*      gWebLoginPassword   = "skywalker";

/** If a fatal error happened, it shall be set true otherwise false. */
static bool             gIsFatalError       = false;

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
    Serial.begin(gSerialBaudrate);
    
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
        if (false == WiFi.softAP(gAPSSID, gAPPassphrase))
        {
            /* Fatal error */
            /* TODO */
            gIsFatalError = true;
        }
        else
        {
            /* Show SSID on the LED marix */
            /* TODO */

            /* Show SSID on the debug interface */
            /* TODO */
            Serial.print("SSID: ");
            Serial.println(gAPSSID);

            isAPMode = true;
        }
        
    }
    else
    {
        /* Connect to a wifi access point. */
        /* TODO */
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
    if (false == gWebServer.authenticate(gWebLoginUser, gWebLoginPassword))
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

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
 * @brief  System state: AP
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "APState.h"
#include <Arduino.h>
#include "SysMsg.h"
#include "MyWebServer.h"
#include "CaptivePortal.h"

#include "ErrorState.h"
#include "RestartState.h"

#include <WiFi.h>
#include <Logging.h>
#include <Util.h>
#include <SettingsService.h>

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

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/* Set a minimum of 8 digits for the passphrase. It shall not be lower than 8 digits! */
const uint8_t   APState::WIFI_AP_PASSPHRASE_MIN_LEN     = 8U;

/** Set access point local address */
const IPAddress APState::LOCAL_IP(192U, 168U, 4U, 1U);

/* Set access point gateway address */
const IPAddress APState::GATEWAY(192U, 168U, 4U, 1U);

/* Set access point subnet mask */
const IPAddress APState::SUBNET(255U, 255U, 255U, 0U);

/* Set DNS port */
const uint16_t  APState::DNS_PORT                       = 53U;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void APState::entry(StateMachine& sm)
{
    String              hostname;
    String              wifiApSSID;
    String              wifiApPassphrase;
    SettingsService&    settings            = SettingsService::getInstance();

    LOG_INFO("Setup access point.");

    /* Get necessary settings. */
    if (false == settings.open(true))
    {
        LOG_WARNING("Use default hostname.");
        hostname = settings.getHostname().getDefault();

        LOG_WARNING("Use default wifi AP SSID.");
        wifiApSSID = settings.getWifiApSSID().getDefault();

        LOG_WARNING("Use default wifi AP passphrase.");
        wifiApPassphrase = settings.getWifiApPassphrase().getDefault();
    }
    else
    {
        hostname            = settings.getHostname().getValue();
        wifiApSSID          = settings.getWifiApSSID().getValue();
        wifiApPassphrase    = settings.getWifiApPassphrase().getValue();

        settings.close();
    }

    /* Configure access point.
     * The DHCP server will automatically be started and uses the range x.x.x.1 - x.x.x.11
     */
    if (false == WiFi.softAPConfig(LOCAL_IP, GATEWAY, SUBNET))
    {
        String errorStr = "Configure wifi access point failed.";

        /* Fatal error */
        LOG_FATAL(errorStr);
        SysMsg::getInstance().show(errorStr);

        sm.setState(ErrorState::getInstance());
    }
    /* Passphrase must be greater or equal than 8 digits. */
    else if (WIFI_AP_PASSPHRASE_MIN_LEN > wifiApPassphrase.length())
    {
        String errorStr = "Wifi AP passphrase must have at least ";
        errorStr += WIFI_AP_PASSPHRASE_MIN_LEN;
        errorStr += " digits.";

        /* Fatal error */
        LOG_FATAL(errorStr);
        SysMsg::getInstance().show(errorStr);

        sm.setState(ErrorState::getInstance());
    }
    /* Set hostname. Note, wifi must be started, which is done
     * by setting the mode before.
     */
    else if (false == WiFi.softAPsetHostname(hostname.c_str()))
    {
        String errorStr = "Can't set AP hostname.";

        /* Fatal error */
        LOG_FATAL(errorStr);
        SysMsg::getInstance().show(errorStr);

        sm.setState(ErrorState::getInstance());
    }
    /* Setup wifi access point. */
    else if (false == WiFi.softAP(wifiApSSID.c_str(), wifiApPassphrase.c_str()))
    {
        String errorStr = "Setup wifi access point failed.";

        /* Fatal error */
        LOG_FATAL(errorStr);
        SysMsg::getInstance().show(errorStr);

        sm.setState(ErrorState::getInstance());
    }
    /* Wifi access point successful up. */
    else
    {
        String infoStr = "SSID: ";
        infoStr += wifiApSSID;

        /* Start DNS and redirect to webserver. */
        if (false == m_dnsServer.start(DNS_PORT, "*", WiFi.softAPIP()))
        {
            LOG_WARNING("Couldn't start DNS.");

            /* DNS couldn't be started, show IP of webserver too. */
            infoStr += " IP: ";
            infoStr += WiFi.softAPIP().toString();
        }
        else
        {
            /* If any other hostname than our is requested, it shall not send a error back,
             * otherwise the client stops instead of continue to the captive portal.
             */
            m_dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        }

        LOG_INFO(infoStr);
        SysMsg::getInstance().show(infoStr);
    }
}

void APState::process(StateMachine& sm)
{
    m_dnsServer.processNextRequest();

    if (true == CaptivePortal::isRestartRequested())
    {
        sm.setState(RestartState::getInstance());
    }
}

void APState::exit(StateMachine& sm)
{
    UTIL_NOT_USED(sm);

    /* Disconnect all connections */
    (void)WiFi.softAPdisconnect();

    /* Stop DNS */
    m_dnsServer.stop();
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

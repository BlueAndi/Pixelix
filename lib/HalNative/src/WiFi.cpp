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
 * @file   WiFi.cpp
 * @brief  Virtual wifi for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WiFi.h"

#include <string.h>

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

WiFiClass WiFi;

/** Default hostname, as long as none is set. */
static const char* DEFAULT_HOSTNAME = "pixelix";

/** The virtual wifi provides the loopback address, the webserver is local. */
static const IPAddress LOOPBACK_IP(127U, 0U, 0U, 1U);

/** Virtual access point address, like the target uses it. */
static const IPAddress AP_IP(192U, 168U, 4U, 1U);

/** Virtual MAC address. */
static const char* VIRTUAL_MAC_ADDRESS = "02:00:00:00:00:01";

/** Virtual signal strength in dBm. */
static const int32_t VIRTUAL_RSSI      = -50;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

WiFiClass::WiFiClass() :
    m_mode(WIFI_MODE_NULL),
    m_status(WL_IDLE_STATUS),
    m_ssid(),
    m_apSSID(),
    m_apIp(AP_IP),
    m_hostname{ 0 },
    m_apHostname{ 0 }
{
    strncpy(m_hostname, DEFAULT_HOSTNAME, sizeof(m_hostname) - 1U);
    strncpy(m_apHostname, DEFAULT_HOSTNAME, sizeof(m_apHostname) - 1U);
}

WiFiClass::~WiFiClass()
{
}

bool WiFiClass::mode(wifi_mode_t mode)
{
    m_mode = mode;

    if (WIFI_MODE_NULL == mode)
    {
        m_status = WL_IDLE_STATUS;
    }

    return true;
}

wifi_mode_t WiFiClass::getMode() const
{
    return m_mode;
}

wl_status_t WiFiClass::begin(const char* ssid, const char* passphrase)
{
    /* The passphrase is not verified by the virtual wifi. */
    (void)passphrase;

    if (nullptr != ssid)
    {
        m_ssid = ssid;
    }

    /* There is no remote network to connect to, but the webserver shall be
     * reachable. Therefore the connection is established right away.
     */
    m_status = WL_CONNECTED;

    return m_status;
}

bool WiFiClass::disconnect()
{
    m_status = WL_DISCONNECTED;
    m_ssid.clear();

    return true;
}

wl_status_t WiFiClass::status() const
{
    return m_status;
}

bool WiFiClass::isConnected() const
{
    return (WL_CONNECTED == m_status);
}

String WiFiClass::SSID() const
{
    return m_ssid;
}

int32_t WiFiClass::RSSI() const
{
    return VIRTUAL_RSSI;
}

IPAddress WiFiClass::localIP() const
{
    return LOOPBACK_IP;
}

String WiFiClass::macAddress() const
{
    return String(VIRTUAL_MAC_ADDRESS);
}

const char* WiFiClass::getHostname() const
{
    return m_hostname;
}

bool WiFiClass::setHostname(const char* hostname)
{
    bool isSuccessful = false;

    if (nullptr != hostname)
    {
        strncpy(m_hostname, hostname, sizeof(m_hostname) - 1U);
        m_hostname[sizeof(m_hostname) - 1U] = '\0';

        isSuccessful                        = true;
    }

    return isSuccessful;
}

bool WiFiClass::setAutoReconnect(bool autoReconnect)
{
    /* The virtual wifi never loses the connection. */
    (void)autoReconnect;

    return true;
}

void WiFiClass::persistent(bool persistent)
{
    /* There is nothing to store. */
    (void)persistent;
}

bool WiFiClass::softAP(const char* ssid, const char* passphrase)
{
    /* The passphrase is not used by the virtual wifi. */
    (void)passphrase;

    if (nullptr != ssid)
    {
        m_apSSID = ssid;
    }

    return true;
}

bool WiFiClass::softAPConfig(const IPAddress& localIp, const IPAddress& gateway, const IPAddress& subnet)
{
    /* Gateway and subnet are not used by the virtual wifi. */
    (void)gateway;
    (void)subnet;

    m_apIp = localIp;

    return true;
}

bool WiFiClass::softAPdisconnect()
{
    m_apSSID.clear();

    return true;
}

IPAddress WiFiClass::softAPIP() const
{
    return m_apIp;
}

const char* WiFiClass::softAPgetHostname() const
{
    return m_apHostname;
}

bool WiFiClass::softAPsetHostname(const char* hostname)
{
    bool isSuccessful = false;

    if (nullptr != hostname)
    {
        strncpy(m_apHostname, hostname, sizeof(m_apHostname) - 1U);
        m_apHostname[sizeof(m_apHostname) - 1U] = '\0';

        isSuccessful                            = true;
    }

    return isSuccessful;
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

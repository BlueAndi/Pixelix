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
 * @file   WiFi.h
 * @brief  Virtual wifi for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * There is no wifi on the host. The virtual wifi reports a established
 * connection right away and provides the loopback address, so the system state
 * machine walks through to the connected state and the webserver is started.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef WIFI_H
#define WIFI_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <Arduino.h>
#include <IPAddress.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Wifi mode, compatible to the ESP32 wifi_mode_t. */
typedef enum
{
    WIFI_MODE_NULL = 0, /**< Wifi is off. */
    WIFI_MODE_STA,      /**< Station mode. */
    WIFI_MODE_AP,       /**< Access point mode. */
    WIFI_MODE_APSTA,    /**< Access point and station mode. */
    WIFI_MODE_MAX       /**< Number of modes. */

} wifi_mode_t;

/** Wifi connection status, compatible to the Arduino wl_status_t. */
typedef enum
{
    WL_IDLE_STATUS = 0, /**< Idle */
    WL_NO_SSID_AVAIL,   /**< No SSID available */
    WL_SCAN_COMPLETED,  /**< Scan completed */
    WL_CONNECTED,       /**< Connected to a remote network */
    WL_CONNECT_FAILED,  /**< Connection failed */
    WL_CONNECTION_LOST, /**< Connection lost */
    WL_DISCONNECTED,    /**< Disconnected */
    WL_NO_SHIELD = 255U /**< No wifi hardware available */

} wl_status_t;

/**
 * Virtual wifi for the native environment.
 */
class WiFiClass
{
public:

    /**
     * Constructs the virtual wifi.
     */
    WiFiClass();

    /**
     * Destroys the virtual wifi.
     */
    ~WiFiClass();

    /**
     * Set the wifi mode.
     *
     * @param[in] mode  Wifi mode
     *
     * @return If successful set, it will return true otherwise false.
     */
    bool mode(wifi_mode_t mode);

    /**
     * Get the wifi mode.
     *
     * @return Wifi mode
     */
    wifi_mode_t getMode() const;

    /**
     * Connect to a remote wifi network.
     * On the host the connection is established right away.
     *
     * @param[in] ssid          SSID of the remote network.
     * @param[in] passphrase    Passphrase of the remote network.
     *
     * @return Connection status
     */
    wl_status_t begin(const char* ssid, const char* passphrase);

    /**
     * Disconnect from the remote wifi network.
     *
     * @return If successful disconnected, it will return true otherwise false.
     */
    bool disconnect();

    /**
     * Get the connection status.
     *
     * @return Connection status
     */
    wl_status_t status() const;

    /**
     * Is a connection to a remote wifi network established?
     *
     * @return If connected, it will return true otherwise false.
     */
    bool isConnected() const;

    /**
     * Get the SSID of the remote wifi network.
     *
     * @return SSID
     */
    String SSID() const;

    /**
     * Get the received signal strength indicator.
     *
     * @return RSSI in dBm
     */
    int32_t RSSI() const;

    /**
     * Get the IP address in station mode.
     *
     * @return IP address
     */
    IPAddress localIP() const;

    /**
     * Get the MAC address.
     *
     * @return MAC address
     */
    String macAddress() const;

    /**
     * Get the hostname in station mode.
     *
     * @return Hostname
     */
    const char* getHostname() const;

    /**
     * Set the hostname in station mode.
     *
     * @param[in] hostname  Hostname
     *
     * @return If successful set, it will return true otherwise false.
     */
    bool setHostname(const char* hostname);

    /**
     * Enable or disable the automatic reconnect.
     *
     * @param[in] autoReconnect Enable or disable it.
     *
     * @return If successful set, it will return true otherwise false.
     */
    bool setAutoReconnect(bool autoReconnect);

    /**
     * Enable or disable the persistent storage of the wifi configuration.
     *
     * @param[in] persistent    Enable or disable it.
     */
    void persistent(bool persistent);

    /**
     * Start the access point.
     *
     * @param[in] ssid          SSID of the access point.
     * @param[in] passphrase    Passphrase of the access point.
     *
     * @return If successful started, it will return true otherwise false.
     */
    bool softAP(const char* ssid, const char* passphrase);

    /**
     * Configure the access point.
     *
     * @param[in] localIp   IP address of the access point.
     * @param[in] gateway   Gateway address.
     * @param[in] subnet    Subnet mask.
     *
     * @return If successful configured, it will return true otherwise false.
     */
    bool softAPConfig(const IPAddress& localIp, const IPAddress& gateway, const IPAddress& subnet);

    /**
     * Stop the access point.
     *
     * @return If successful stopped, it will return true otherwise false.
     */
    bool softAPdisconnect();

    /**
     * Get the IP address of the access point.
     *
     * @return IP address
     */
    IPAddress softAPIP() const;

    /**
     * Get the hostname of the access point.
     *
     * @return Hostname
     */
    const char* softAPgetHostname() const;

    /**
     * Set the hostname of the access point.
     *
     * @param[in] hostname  Hostname
     *
     * @return If successful set, it will return true otherwise false.
     */
    bool softAPsetHostname(const char* hostname);

private:

    /** Max. length of a hostname (incl. string termination). */
    static const size_t MAX_HOSTNAME_SIZE = 64U;

    wifi_mode_t         m_mode;                          /**< Current wifi mode. */
    wl_status_t         m_status;                        /**< Current connection status. */
    String              m_ssid;                          /**< SSID of the remote network. */
    String              m_apSSID;                        /**< SSID of the access point. */
    IPAddress           m_apIp;                          /**< IP address of the access point. */
    char                m_hostname[MAX_HOSTNAME_SIZE];   /**< Hostname in station mode. */
    char                m_apHostname[MAX_HOSTNAME_SIZE]; /**< Hostname of the access point. */

    WiFiClass(const WiFiClass& wifi);
    WiFiClass& operator=(const WiFiClass& wifi);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

/** The virtual wifi, like the Arduino provides it. */
extern WiFiClass WiFi;

#endif /* WIFI_H */

/** @} */

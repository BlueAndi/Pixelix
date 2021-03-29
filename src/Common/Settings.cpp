/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Settings
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Settings.h"

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

/** Settings namespace used for preferences */
static const char*  PREF_NAMESPACE                  = "settings";

/* ---------- Keys ---------- */

/* Note:
 * Zero-terminated ASCII string containing a key name.
 * Maximum string length is 15 bytes, excluding a zero terminator.
 * https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/storage/nvs_flash.html
 */

/** Wifi network key */
static const char*  KEY_WIFI_SSID                   = "sta_ssid";

/** Wifi network passphrase key */
static const char*  KEY_WIFI_PASSPHRASE             = "sta_passphrase";

/** Wifi access point network key */
static const char*  KEY_WIFI_AP_SSID                = "ap_ssid";

/** Wifi access point network passphrase key */
static const char*  KEY_WIFI_AP_PASSPHRASE          = "ap_passphrase";

/** Hostname key */
static const char*  KEY_HOSTNAME                    = "hostname";

/** Automatic brightness control key */
static const char*  KEY_AUTO_BRIGHTNESS_CTRL        = "a_brightn_ctrl";

/** Plugin installation key */
static const char*  KEY_PLUGIN_INSTALLATION         = "plugin_install";

/** POSIX timezone string key */
static const char*  KEY_TIMEZONE                    = "timezone";

/** NTP server key */
static const char*  KEY_NTP_SERVER                  = "ntp_server";

/** Time format key */
static const char*  KEY_TIME_FORMAT                 = "time_format";

/** Date format key */
static const char*  KEY_DATE_FORMAT                 = "date_format";

/** Max. number of display slots key */
static const char*  KEY_MAX_SLOTS                   = "max_slots";

/** Display slot configuration key */
static const char*  KEY_SLOT_CONFIG                 = "slot_cfg";

/** Scroll pause key */
static const char*  KEY_SCROLL_PAUSE                = "scroll_pause";

/* ---------- Key value pair names ---------- */

/** Wifi network name of key value pair */
static const char*  NAME_WIFI_SSID                  = "Wifi SSID";

/** Wifi network passphrase name of key value pair */
static const char*  NAME_WIFI_PASSPHRASE            = "Wifi passphrase";

/** Wifi access point network name of key value pair */
static const char*  NAME_WIFI_AP_SSID               = "Wifi AP SSID";

/** Wifi access point network passphrase name of key value pair */
static const char*  NAME_WIFI_AP_PASSPHRASE         = "Wifi AP passphrase";

/** Hostname name of key value pair */
static const char*  NAME_HOSTNAME                   = "Hostname";

/** Automatic brightness control name of key value pair */
static const char*  NAME_AUTO_BRIGHTNESS_CTRL       = "Autom. brightness control";

/** Plugin installation name of key value pair */
static const char*  NAME_PLUGIN_INSTALLATION        = "Plugin installation";

/** POSIX timezone string name of key value pair. */
static const char*  NAME_TIMEZONE                   = "POSIX timezone string";

/** NTP server name of key value pair */
static const char*  NAME_NTP_SERVER                 = "NTP server address";

/** Time format name of key value pair */
static const char*  NAME_TIME_FORMAT_CTRL           = "Time format: true = 24h, false = 12h (AM/PM)";

/** Date format name of key value pair */
static const char*  NAME_DATE_FORMAT_CTRL           = "Date format: true = DD:MM, false = MM:DD";

/** Max. number of display slots name of key value pair */
static const char*  NAME_MAX_SLOTS                  = "Max. slots";

/** Display slot configuration name */
static const char*  NAME_SLOT_CONFIG                = "Display slot configuration";

/** Scroll pause name */
static const char*  NAME_SCROLL_PAUSE               = "Text scroll pause [ms]";

/* ---------- Default values ---------- */

/** Wifi network default value */
static const char*      DEFAULT_WIFI_SSID               = "";

/** Wifi network passphrase default value */
static const char*      DEFAULT_WIFI_PASSPHRASE         = "";

/** Wifi access point network default value */
static const char*      DEFAULT_WIFI_AP_SSID            = "pixelix";

/** Wifi access point network passphrase default value */
static const char*      DEFAULT_WIFI_AP_PASSPHRASE      = "Luke, I am your father.";

/** Hostname default value */
static const char*      DEFAULT_HOSTNAME                = "pixelix";

/** Automatic brightness control default value */
static bool             DEFAULT_AUTO_BRIGHTNESS_CTRL    = false;

/** Plugin installation default value */
static const char*      DEFAULT_PLUGIN_INSTALLATION     = "";

/** POSIX timezone string default value */
static const char*      DEFAULT_TIMEZONE                = "WEST-1DWEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";

/** NTP server default value */
static const char*      DEFAULT_NTP_SERVER              = "pool.ntp.org";

/** Time format control default value */
static bool             DEFAULT_TIME_FORMAT_CTRL        = true;

/** Date format control default value */
static bool             DEFAULT_DATE_FORMAT_CTRL        = true;

/** Max. number of display slots default value */
static uint8_t          DEFAULT_MAX_SLOTS               = 8U;

/** Display slot configuration default value */
static const char*      DEFAULT_SLOT_CONFIG             = "";

/** Scroll pause default value in ms */
static uint32_t         DEFAULT_SCROLL_PAUSE            = 80U;

/* ---------- Minimum values ---------- */

/** Wifi network SSID min. length. Section 7.3.2.1 of the 802.11-2007 specification. */
static const size_t     MIN_VALUE_WIFI_SSID            = 0;

/** Wifi network passphrase min. length */
static const size_t     MIN_VALUE_WIFI_PASSPHRASE      = 8U;

/** Wifi access point network SSID min. length. Section 7.3.2.1 of the 802.11-2007 specification. */
static const size_t     MIN_VALUE_WIFI_AP_SSID         = 0;

/** Wifi access point network passphrase min. length */
static const size_t     MIN_VALUE_WIFI_AP_PASSPHRASE   = 8U;

/** Hostname min. length */
static const size_t     MIN_VALUE_HOSTNAME             = 1U;

/*                      MIN_VALUE_AUTO_BRIGHTNESS_CTRL */

/** Plugin installation min. length */
static const size_t     MIN_VALUE_PLUGIN_INSTALLATION   = 0U;

/** POSIX timezone min. length */
static const size_t     MIN_VALUE_TIMEZONE              = 4U;

/** NTP server address min. length */
static const size_t     MIN_VALUE_NTP_SERVER            = 12U;

/*                      MIN_VALUE_TIME_FORMAT_CTRL */

/*                      MIN_VALUE_TIME_FORMAT_CTRL */

/** Max. number of display slots minimum value */
static uint8_t          MIN_MAX_SLOTS                   = 2U;

/** Display slot configuration min. length */
static const size_t     MIN_VALUE_SLOT_CONFIG           = 0U;

/** Scroll pause minimum value in ms */
static uint32_t         MIN_VALUE_SCROLL_PAUSE          = 20U;

/* ---------- Maximum values ---------- */

/** Wifi network SSID max. length. Section 7.3.2.1 of the 802.11-2007 specification. */
static const size_t     MAX_VALUE_WIFI_SSID            = 32U;

/** Wifi network passphrase max. length */
static const size_t     MAX_VALUE_WIFI_PASSPHRASE      = 64U;

/** Wifi access point network SSID max. length. Section 7.3.2.1 of the 802.11-2007 specification. */
static const size_t     MAX_VALUE_WIFI_AP_SSID         = 32U;

/** Wifi access point network passphrase max. length */
static const size_t     MAX_VALUE_WIFI_AP_PASSPHRASE   = 64U;

/** Hostname max. length */
static const size_t     MAX_VALUE_HOSTNAME             = 63U;

/*                      MAX_VALUE_AUTO_BRIGHTNESS_CTRL */

/** Plugin installation max. length */
static const size_t     MAX_VALUE_PLUGIN_INSTALLATION  = 512U;

/** POSIX timezone max. length */
static const size_t     MAX_VALUE_TIMEZONE             = 128U;

/** NTP server address max. length */
static const size_t     MAX_VALUE_NTP_SERVER            = 30U;

/*                      MAX_VALUE_TIME_FORMAT_CTRL */

/*                      MAX_VALUE_TIME_FORMAT_CTRL */

/** Max. number of display slots maximum value */
static uint8_t          MAX_MAX_SLOTS                   = 11U;

/** Display slot configuration max. length */
static const size_t     MAX_VALUE_SLOT_CONFIG           = 256U;

/** Scroll pause maximum value in ms */
static uint32_t         MAX_VALUE_SCROLL_PAUSE          = 500U;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool Settings::open(bool readOnly)
{
    /* Open Preferences with namespace. Each application module, library, etc
     * has to use a namespace name to prevent key name collisions. We will open storage in
     * RW-mode (second parameter has to be false).
     * Note: Namespace name is limited to 15 chars.
     */
    bool status = m_preferences.begin(PREF_NAMESPACE, readOnly);

    /* If settings storage doesn't exist, it will be created. */
    if ((false == status) &&
        (true == readOnly))
    {
        status = m_preferences.begin(PREF_NAMESPACE, false);

        if (true == status)
        {
            m_preferences.end();
            status = m_preferences.begin(PREF_NAMESPACE, readOnly);
        }
    }

    return status;
}

void Settings::close()
{
    m_preferences.end();
    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

Settings::Settings() :
    m_preferences(),
    m_keyValueList(),
    m_wifiSSID              (m_preferences, KEY_WIFI_SSID,              NAME_WIFI_SSID,             DEFAULT_WIFI_SSID,              MIN_VALUE_WIFI_SSID,            MAX_VALUE_WIFI_SSID),
    m_wifiPassphrase        (m_preferences, KEY_WIFI_PASSPHRASE,        NAME_WIFI_PASSPHRASE,       DEFAULT_WIFI_PASSPHRASE,        MIN_VALUE_WIFI_PASSPHRASE,      MAX_VALUE_WIFI_PASSPHRASE),
    m_apSSID                (m_preferences, KEY_WIFI_AP_SSID,           NAME_WIFI_AP_SSID,          DEFAULT_WIFI_AP_SSID,           MIN_VALUE_WIFI_AP_SSID,         MAX_VALUE_WIFI_AP_SSID),
    m_apPassphrase          (m_preferences, KEY_WIFI_AP_PASSPHRASE,     NAME_WIFI_AP_PASSPHRASE,    DEFAULT_WIFI_AP_PASSPHRASE,     MIN_VALUE_WIFI_AP_PASSPHRASE,   MAX_VALUE_WIFI_AP_PASSPHRASE),
    m_hostname              (m_preferences, KEY_HOSTNAME,               NAME_HOSTNAME,              DEFAULT_HOSTNAME,               MIN_VALUE_HOSTNAME,             MAX_VALUE_HOSTNAME),
    m_autoBrightnessCtrl    (m_preferences, KEY_AUTO_BRIGHTNESS_CTRL,   NAME_AUTO_BRIGHTNESS_CTRL,  DEFAULT_AUTO_BRIGHTNESS_CTRL),
    m_pluginInstallation    (m_preferences, KEY_PLUGIN_INSTALLATION,    NAME_PLUGIN_INSTALLATION,   DEFAULT_PLUGIN_INSTALLATION,    MIN_VALUE_PLUGIN_INSTALLATION,  MAX_VALUE_PLUGIN_INSTALLATION),
    m_timezone              (m_preferences, KEY_TIMEZONE,               NAME_TIMEZONE,              DEFAULT_TIMEZONE,               MIN_VALUE_TIMEZONE,             MAX_VALUE_TIMEZONE),
    m_ntpServer             (m_preferences, KEY_NTP_SERVER,             NAME_NTP_SERVER,            DEFAULT_NTP_SERVER,             MIN_VALUE_NTP_SERVER,           MAX_VALUE_NTP_SERVER),
    m_timeFormatCtrl        (m_preferences, KEY_TIME_FORMAT,            NAME_TIME_FORMAT_CTRL,      DEFAULT_TIME_FORMAT_CTRL),
    m_dateFormatCtrl        (m_preferences, KEY_DATE_FORMAT,            NAME_DATE_FORMAT_CTRL,      DEFAULT_DATE_FORMAT_CTRL),
    m_maxSlots              (m_preferences, KEY_MAX_SLOTS,              NAME_MAX_SLOTS,             DEFAULT_MAX_SLOTS,              MIN_MAX_SLOTS,                  MAX_MAX_SLOTS),
    m_slotConfig            (m_preferences, KEY_SLOT_CONFIG,            NAME_SLOT_CONFIG,           DEFAULT_SLOT_CONFIG,            MIN_VALUE_SLOT_CONFIG,          MAX_VALUE_SLOT_CONFIG),
    m_scrollPause           (m_preferences, KEY_SCROLL_PAUSE,           NAME_SCROLL_PAUSE,          DEFAULT_SCROLL_PAUSE,           MIN_VALUE_SCROLL_PAUSE,         MAX_VALUE_SCROLL_PAUSE)
{
    m_keyValueList[0] = &m_wifiSSID;
    m_keyValueList[1] = &m_wifiPassphrase;
    m_keyValueList[2] = &m_apSSID;
    m_keyValueList[3] = &m_apPassphrase;
    m_keyValueList[4] = &m_hostname;
    m_keyValueList[5] = &m_autoBrightnessCtrl;
    m_keyValueList[6] = &m_pluginInstallation;
    m_keyValueList[7] = &m_timezone;
    m_keyValueList[8] = &m_ntpServer;
    m_keyValueList[9] = &m_timeFormatCtrl;
    m_keyValueList[10] = &m_dateFormatCtrl;
    m_keyValueList[11] = &m_maxSlots;
    m_keyValueList[12] = &m_slotConfig;
    m_keyValueList[13] = &m_scrollPause;
}

Settings::~Settings()
{
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

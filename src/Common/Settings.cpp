/* MIT License
 *
 * Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>
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

/** Website login user account key */
static const char*  KEY_WEB_LOGIN_USER              = "web_login_user";

/** Website login user password key */
static const char*  KEY_WEB_LOGIN_PASSWORD          = "web_login_pass";

/** Hostname key */
static const char*  KEY_HOSTNAME                    = "hostname";

/** Brightness key */
static const char*  KEY_BRIGHTNESS                  = "brightness";

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

/** NotifyURL key */
static const char*  KEY_NOTIFY_URL                  = "notify_url";

/* ---------- Key value pair names ---------- */

/** Wifi network name of key value pair */
static const char*  NAME_WIFI_SSID                  = "Wifi SSID";

/** Wifi network passphrase name of key value pair */
static const char*  NAME_WIFI_PASSPHRASE            = "Wifi passphrase";

/** Wifi access point network name of key value pair */
static const char*  NAME_WIFI_AP_SSID               = "Wifi AP SSID";

/** Wifi access point network passphrase name of key value pair */
static const char*  NAME_WIFI_AP_PASSPHRASE         = "Wifi AP passphrase";

/** Website login user account name of key value pair */
static const char*  NAME_WEB_LOGIN_USER             = "Website login user";

/** Website login user password name of key value pair */
static const char*  NAME_WEB_LOGIN_PASSWORD         = "Website login password";

/** Hostname name of key value pair */
static const char*  NAME_HOSTNAME                   = "Hostname";

/** Brightness name of key value pair */
static const char*  NAME_BRIGHTNESS                 = "Brightness set at startup in %";

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

/** NotifyURL name */
static const char*  NAME_NOTIFY_URL                 = "URL to be triggered when PIXELIX has connected to a remote network.";

/* ---------- Default values ---------- */

/** Wifi network default value */
static const char*      DEFAULT_WIFI_SSID               = "";

/** Wifi network passphrase default value */
static const char*      DEFAULT_WIFI_PASSPHRASE         = "";

/** Wifi access point network default value */
static const char*      DEFAULT_WIFI_AP_SSID            = "pixelix";

/** Wifi access point network passphrase default value */
static const char*      DEFAULT_WIFI_AP_PASSPHRASE      = "Luke, I am your father.";

/** Website login user account default value */
static const char*      DEFAULT_WEB_LOGIN_USER          = "luke";

/** Website login user password default value */
static const char*      DEFAULT_WEB_LOGIN_PASSWORD      = "skywalker";

/** Hostname default value */
static const char*      DEFAULT_HOSTNAME                = "pixelix";

/** Brightness default value in % */
static const uint8_t    DEFAULT_BRIGHTNESS              = 10U; /* If powered via USB, keep this at 10% to avoid damage. */

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

/** NotifyURL default value */
static const char*     DEFAULT_NOTIFY_URL               = "";

/* ---------- Minimum values ---------- */

/** Wifi network SSID min. length. Section 7.3.2.1 of the 802.11-2007 specification. */
static const size_t     MIN_VALUE_WIFI_SSID             = 0;

/** Wifi network passphrase min. length */
static const size_t     MIN_VALUE_WIFI_PASSPHRASE       = 8U;

/** Wifi access point network SSID min. length. Section 7.3.2.1 of the 802.11-2007 specification. */
static const size_t     MIN_VALUE_WIFI_AP_SSID          = 0;

/** Wifi access point network passphrase min. length */
static const size_t     MIN_VALUE_WIFI_AP_PASSPHRASE    = 8U;

/** Website login user account min. length */
static const size_t     MIN_VALUE_WEB_LOGIN_USER        = 4U;

/** Website login user password min. length */
static const size_t     MIN_VALUE_WEB_LOGIN_PASSWORD    = 4U;

/** Hostname min. length */
static const size_t     MIN_VALUE_HOSTNAME              = 1U;

/** Brightness min. value in % */
static const uint8_t    MIN_VALUE_BRIGHTNESS            = 0U;

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

/** NotifyURL min. length */
static const size_t     MIN_VALUE_NOTIFY_URL            = 0U;

/* ---------- Maximum values ---------- */

/** Wifi network SSID max. length. Section 7.3.2.1 of the 802.11-2007 specification. */
static const size_t     MAX_VALUE_WIFI_SSID             = 32U;

/** Wifi network passphrase max. length */
static const size_t     MAX_VALUE_WIFI_PASSPHRASE       = 64U;

/** Wifi access point network SSID max. length. Section 7.3.2.1 of the 802.11-2007 specification. */
static const size_t     MAX_VALUE_WIFI_AP_SSID          = 32U;

/** Wifi access point network passphrase max. length */
static const size_t     MAX_VALUE_WIFI_AP_PASSPHRASE    = 64U;

/** Website login user account max. length */
static const size_t     MAX_VALUE_WEB_LOGIN_USER        = 16U;

/** Website login user password max. length */
static const size_t     MAX_VALUE_WEB_LOGIN_PASSWORD    = 32U;

/** Hostname max. length */
static const size_t     MAX_VALUE_HOSTNAME              = 63U;

/** Brightness max. value in % */
static const uint8_t    MAX_VALUE_BRIGHTNESS            = 100U;

/*                      MAX_VALUE_AUTO_BRIGHTNESS_CTRL */

/** Plugin installation max. length */
static const size_t     MAX_VALUE_PLUGIN_INSTALLATION   = 1280U;

/** POSIX timezone max. length */
static const size_t     MAX_VALUE_TIMEZONE              = 128U;

/** NTP server address max. length */
static const size_t     MAX_VALUE_NTP_SERVER            = 30U;

/*                      MAX_VALUE_TIME_FORMAT_CTRL */

/*                      MAX_VALUE_TIME_FORMAT_CTRL */

/** Max. number of display slots maximum value */
static uint8_t          MAX_MAX_SLOTS                   = 11U;

/** Display slot configuration max. length */
static const size_t     MAX_VALUE_SLOT_CONFIG           = 512U;

/** Scroll pause maximum value in ms */
static uint32_t         MAX_VALUE_SCROLL_PAUSE          = 500U;

/** NotifyURL max. length */
static const size_t     MAX_VALUE_NOTIFY_URL            = 64U;

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

KeyValue* Settings::getSettingByKey(const char* key)
{
    uint8_t     idx             = 0U;
    KeyValue*   keyValuePair    = nullptr;

    while((KEY_VALUE_PAIR_NUM > idx) && (0 != strcmp(m_keyValueList[idx]->getKey(), key)))
    {
        ++idx;
    }

    if (KEY_VALUE_PAIR_NUM > idx)
    {
        keyValuePair = m_keyValueList[idx];
    }

    return keyValuePair;
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
    m_wifiPassphrase        (m_preferences, KEY_WIFI_PASSPHRASE,        NAME_WIFI_PASSPHRASE,       DEFAULT_WIFI_PASSPHRASE,        MIN_VALUE_WIFI_PASSPHRASE,      MAX_VALUE_WIFI_PASSPHRASE,      true),
    m_apSSID                (m_preferences, KEY_WIFI_AP_SSID,           NAME_WIFI_AP_SSID,          DEFAULT_WIFI_AP_SSID,           MIN_VALUE_WIFI_AP_SSID,         MAX_VALUE_WIFI_AP_SSID),
    m_apPassphrase          (m_preferences, KEY_WIFI_AP_PASSPHRASE,     NAME_WIFI_AP_PASSPHRASE,    DEFAULT_WIFI_AP_PASSPHRASE,     MIN_VALUE_WIFI_AP_PASSPHRASE,   MAX_VALUE_WIFI_AP_PASSPHRASE,   true),
    m_webLoginUser          (m_preferences, KEY_WEB_LOGIN_USER,         NAME_WEB_LOGIN_USER,        DEFAULT_WEB_LOGIN_USER,         MIN_VALUE_WEB_LOGIN_USER,       MAX_VALUE_WEB_LOGIN_USER),
    m_webLoginPassword      (m_preferences, KEY_WEB_LOGIN_PASSWORD,     NAME_WEB_LOGIN_PASSWORD,    DEFAULT_WEB_LOGIN_PASSWORD,     MIN_VALUE_WEB_LOGIN_PASSWORD,   MAX_VALUE_WEB_LOGIN_PASSWORD,   true),
    m_hostname              (m_preferences, KEY_HOSTNAME,               NAME_HOSTNAME,              DEFAULT_HOSTNAME,               MIN_VALUE_HOSTNAME,             MAX_VALUE_HOSTNAME),
    m_brightness            (m_preferences, KEY_BRIGHTNESS,             NAME_BRIGHTNESS,            DEFAULT_BRIGHTNESS,             MIN_VALUE_BRIGHTNESS,           MAX_VALUE_BRIGHTNESS),
    m_autoBrightnessCtrl    (m_preferences, KEY_AUTO_BRIGHTNESS_CTRL,   NAME_AUTO_BRIGHTNESS_CTRL,  DEFAULT_AUTO_BRIGHTNESS_CTRL),
    m_pluginInstallation    (m_preferences, KEY_PLUGIN_INSTALLATION,    NAME_PLUGIN_INSTALLATION,   DEFAULT_PLUGIN_INSTALLATION,    MIN_VALUE_PLUGIN_INSTALLATION,  MAX_VALUE_PLUGIN_INSTALLATION),
    m_timezone              (m_preferences, KEY_TIMEZONE,               NAME_TIMEZONE,              DEFAULT_TIMEZONE,               MIN_VALUE_TIMEZONE,             MAX_VALUE_TIMEZONE),
    m_ntpServer             (m_preferences, KEY_NTP_SERVER,             NAME_NTP_SERVER,            DEFAULT_NTP_SERVER,             MIN_VALUE_NTP_SERVER,           MAX_VALUE_NTP_SERVER),
    m_timeFormatCtrl        (m_preferences, KEY_TIME_FORMAT,            NAME_TIME_FORMAT_CTRL,      DEFAULT_TIME_FORMAT_CTRL),
    m_dateFormatCtrl        (m_preferences, KEY_DATE_FORMAT,            NAME_DATE_FORMAT_CTRL,      DEFAULT_DATE_FORMAT_CTRL),
    m_maxSlots              (m_preferences, KEY_MAX_SLOTS,              NAME_MAX_SLOTS,             DEFAULT_MAX_SLOTS,              MIN_MAX_SLOTS,                  MAX_MAX_SLOTS),
    m_slotConfig            (m_preferences, KEY_SLOT_CONFIG,            NAME_SLOT_CONFIG,           DEFAULT_SLOT_CONFIG,            MIN_VALUE_SLOT_CONFIG,          MAX_VALUE_SLOT_CONFIG),
    m_scrollPause           (m_preferences, KEY_SCROLL_PAUSE,           NAME_SCROLL_PAUSE,          DEFAULT_SCROLL_PAUSE,           MIN_VALUE_SCROLL_PAUSE,         MAX_VALUE_SCROLL_PAUSE),
    m_notifyURL             (m_preferences, KEY_NOTIFY_URL,             NAME_NOTIFY_URL,            DEFAULT_NOTIFY_URL,             MIN_VALUE_NOTIFY_URL,           MAX_VALUE_NOTIFY_URL)
{
    uint8_t idx = 0;

    m_keyValueList[idx] = &m_wifiSSID;
    ++idx;
    m_keyValueList[idx] = &m_wifiPassphrase;
    ++idx;
    m_keyValueList[idx] = &m_apSSID;
    ++idx;
    m_keyValueList[idx] = &m_apPassphrase;
    ++idx;
    m_keyValueList[idx] = &m_webLoginUser;
    ++idx;
    m_keyValueList[idx] = &m_webLoginPassword;
    ++idx;
    m_keyValueList[idx] = &m_hostname;
    ++idx;
    m_keyValueList[idx] = &m_brightness;
    ++idx;
    m_keyValueList[idx] = &m_autoBrightnessCtrl;
    ++idx;
    m_keyValueList[idx] = &m_pluginInstallation;
    ++idx;
    m_keyValueList[idx] = &m_timezone;
    ++idx;
    m_keyValueList[idx] = &m_ntpServer;
    ++idx;
    m_keyValueList[idx] = &m_timeFormatCtrl;
    ++idx;
    m_keyValueList[idx] = &m_dateFormatCtrl;
    ++idx;
    m_keyValueList[idx] = &m_maxSlots;
    ++idx;
    m_keyValueList[idx] = &m_slotConfig;
    ++idx;
    m_keyValueList[idx] = &m_scrollPause;
    ++idx;
    m_keyValueList[idx] = &m_notifyURL;
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

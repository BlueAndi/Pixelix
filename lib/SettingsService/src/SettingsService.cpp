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
 * @brief  Settings service
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "SettingsService.h"
#include "nvs.h"

#include <Logging.h>
#include <algorithm>

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

/** SettingsService namespace used for preferences */
static const char*  PREF_NAMESPACE                  = "settings";

/* ---------- Keys ---------- */

/* Note:
 * Zero-terminated ASCII string containing a key name.
 * Maximum string length is 15 bytes, excluding a zero terminator.
 * https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/storage/nvs_flash.html
 */

/** Settings version key */
static const char*  KEY_VERSION                     = "version";

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

/** POSIX timezone string key */
static const char*  KEY_TIMEZONE                    = "timezone";

/** NTP server key */
static const char*  KEY_NTP_SERVER                  = "ntp_server";

/** Max. number of display slots key */
static const char*  KEY_MAX_SLOTS                   = "max_slots";

/** Scroll pause key */
static const char*  KEY_SCROLL_PAUSE                = "scroll_pause";

/** NotifyURL key */
static const char*  KEY_NOTIFY_URL                  = "notify_url";

/** Quiet mode key */
static const char*  KEY_QUIET_MODE                  = "quiet_mode";

/* ---------- Key value pair names ---------- */

/** SettingsService version name */
static const char*  NAME_VERSION                    = "SettingsService version";

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

/** POSIX timezone string name of key value pair. */
static const char*  NAME_TIMEZONE                   = "POSIX timezone string";

/** NTP server name of key value pair */
static const char*  NAME_NTP_SERVER                 = "NTP server address";

/** Max. number of display slots name of key value pair */
static const char*  NAME_MAX_SLOTS                  = "Max. slots";

/** Scroll pause name */
static const char*  NAME_SCROLL_PAUSE               = "Text scroll pause [ms]";

/** NotifyURL name */
static const char*  NAME_NOTIFY_URL                 = "URL to be triggered when PIXELIX has connected to a remote network.";

/** Quiet mode name */
static const char*  NAME_QUIET_MODE                 = "Quiet mode (skip unnecessary system messages)";

/* ---------- Default values ---------- */

/** SettingsService version default value */
static const uint32_t   DEFAULT_VERSION                 = 0U; /* 0 is important to detect whether the version is not stored yet. */

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
static const uint8_t    DEFAULT_BRIGHTNESS              = 25U; /* If powered via USB, keep this at 25% to avoid damage. */

/** Automatic brightness control default value */
static const bool       DEFAULT_AUTO_BRIGHTNESS_CTRL    = false;

/** POSIX timezone string default value */
static const char*      DEFAULT_TIMEZONE                = "WEST-1DWEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";

/** NTP server default value */
static const char*      DEFAULT_NTP_SERVER              = "pool.ntp.org";

/** Max. number of display slots default value */
static const uint8_t    DEFAULT_MAX_SLOTS               = 8U;

/** Scroll pause default value in ms */
static const uint32_t   DEFAULT_SCROLL_PAUSE            = 80U;

/** NotifyURL default value */
static const char*      DEFAULT_NOTIFY_URL              = "";

/** Quiet mode default value */
static const bool       DEFAULT_QUIET_MODE              = false;

/* ---------- Minimum values ---------- */

/** SettingsService version min. value */
static const uint32_t   MIN_VALUE_VERSION               = 0;

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
static const uint8_t    MIN_VALUE_BRIGHTNESS            = 10U;

/*                      MIN_VALUE_AUTO_BRIGHTNESS_CTRL */

/** POSIX timezone min. length */
static const size_t     MIN_VALUE_TIMEZONE              = 4U;

/** NTP server address min. length */
static const size_t     MIN_VALUE_NTP_SERVER            = 12U;

/** Max. number of display slots minimum value */
static const uint8_t    MIN_MAX_SLOTS                   = 2U;

/** Scroll pause minimum value in ms */
static const uint32_t   MIN_VALUE_SCROLL_PAUSE          = 20U;

/** NotifyURL min. length */
static const size_t     MIN_VALUE_NOTIFY_URL            = 0U;

/*                      MIN_VALUE_QUIET_MODE */

/* ---------- Maximum values ---------- */

/** SettingsService version max. value */
static const uint32_t   MAX_VALUE_VERSION               = UINT32_MAX;

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

/** POSIX timezone max. length */
static const size_t     MAX_VALUE_TIMEZONE              = 128U;

/** NTP server address max. length */
static const size_t     MAX_VALUE_NTP_SERVER            = 30U;

/**
 * Max. number of display slots maximum value.
 * The number of slots can not be increased infinite. Please consider the following:
 * - The available heap memory will be reduced. How many should be available at least can
 *      not easy determined. E.g. the network stack needs heap to handle requests, the REST
 *      API needs it to handle JSON documents, etc.
 */
static const uint8_t    MAX_MAX_SLOTS                   = 16U;

/** Scroll pause maximum value in ms */
static const uint32_t   MAX_VALUE_SCROLL_PAUSE          = 500U;

/** NotifyURL max. length */
static const size_t     MAX_VALUE_NOTIFY_URL            = 64U;

/*                      MAX_VALUE_QUIET_MODE */

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool SettingsService::start()
{
    LOG_INFO("Settings service started.");

    return true;
}

void SettingsService::stop()
{
    LOG_INFO("Settings service stopped.");
}

void SettingsService::process()
{
    /* Nothing to do. */
}

bool SettingsService::open(bool readOnly)
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

void SettingsService::close()
{
    m_preferences.end();
}

void SettingsService::cleanUp()
{
    uint32_t storedVersion = m_version.getValue();

    /* Clean up is only necessary, if settings version is different. */
    if (VERSION != storedVersion)
    {
        nvs_iterator_t it = nvs_entry_find(NVS_DEFAULT_PART_NAME, PREF_NAMESPACE, NVS_TYPE_ANY);

        while (nullptr != it)
        {
            nvs_entry_info_t info;

            nvs_entry_info(it, &info);
            it = nvs_entry_next(it);

            /* Obsolete setting?
             * m_version key must be handled separate, because its not part of the settings list.
             */
            if ((0 != strcmp(m_version.getKey(), info.key)) &&
                (nullptr == getSettingByKey(info.key)))
            {
                LOG_WARNING("Obsolete key %s removed from settings.", info.key);

                if (false == m_preferences.remove(info.key))
                {
                    LOG_ERROR("Failed to remove key %s removed from settings.", info.key);
                }
            }
            else
            {
                LOG_INFO("SettingsService key %s is valid.", info.key);
            }
        };

        /* Update version */
        m_version.setValue(VERSION);
    }
}

KeyValue* SettingsService::getSettingByKey(const char* key)
{
    std::vector<KeyValue*>::const_iterator  it;
    KeyValue*                               keyValuePair    = nullptr;

    for(it = m_keyValueList.begin(); it != m_keyValueList.end(); ++it)
    {
        if (nullptr != *it)
        {
            if (0 == strcmp((*it)->getKey(), key))
            {
                keyValuePair = *it;
                break;
            }
        }
    }

    return keyValuePair;
}

bool SettingsService::registerSetting(KeyValue* setting)
{
    bool isSuccessful = false;

    if (nullptr != setting)
    {
        /* Register setting only once! */
        if (std::find(m_keyValueList.begin(), m_keyValueList.end(), setting) == m_keyValueList.end())
        {
            setting->setPersistentStorage(m_preferences);
            m_keyValueList.push_back(setting);

            isSuccessful = true;
        }
    }

    return isSuccessful;
}

void SettingsService::unregisterSetting(KeyValue* setting)
{
    std::vector<KeyValue*>::iterator it = m_keyValueList.begin();

    while(m_keyValueList.end() != it)
    {
        if (setting == *it)
        {
            it = m_keyValueList.erase(it);
            break;
        }
        else
        {
            ++it;
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

SettingsService::SettingsService() :
    m_preferences(),
    m_keyValueList(),
    m_version               (m_preferences, KEY_VERSION,                NAME_VERSION,               DEFAULT_VERSION,                MIN_VALUE_VERSION,              MAX_VALUE_VERSION),
    m_wifiSSID              (m_preferences, KEY_WIFI_SSID,              NAME_WIFI_SSID,             DEFAULT_WIFI_SSID,              MIN_VALUE_WIFI_SSID,            MAX_VALUE_WIFI_SSID),
    m_wifiPassphrase        (m_preferences, KEY_WIFI_PASSPHRASE,        NAME_WIFI_PASSPHRASE,       DEFAULT_WIFI_PASSPHRASE,        MIN_VALUE_WIFI_PASSPHRASE,      MAX_VALUE_WIFI_PASSPHRASE,      true),
    m_apSSID                (m_preferences, KEY_WIFI_AP_SSID,           NAME_WIFI_AP_SSID,          DEFAULT_WIFI_AP_SSID,           MIN_VALUE_WIFI_AP_SSID,         MAX_VALUE_WIFI_AP_SSID),
    m_apPassphrase          (m_preferences, KEY_WIFI_AP_PASSPHRASE,     NAME_WIFI_AP_PASSPHRASE,    DEFAULT_WIFI_AP_PASSPHRASE,     MIN_VALUE_WIFI_AP_PASSPHRASE,   MAX_VALUE_WIFI_AP_PASSPHRASE,   true),
    m_webLoginUser          (m_preferences, KEY_WEB_LOGIN_USER,         NAME_WEB_LOGIN_USER,        DEFAULT_WEB_LOGIN_USER,         MIN_VALUE_WEB_LOGIN_USER,       MAX_VALUE_WEB_LOGIN_USER),
    m_webLoginPassword      (m_preferences, KEY_WEB_LOGIN_PASSWORD,     NAME_WEB_LOGIN_PASSWORD,    DEFAULT_WEB_LOGIN_PASSWORD,     MIN_VALUE_WEB_LOGIN_PASSWORD,   MAX_VALUE_WEB_LOGIN_PASSWORD,   true),
    m_hostname              (m_preferences, KEY_HOSTNAME,               NAME_HOSTNAME,              DEFAULT_HOSTNAME,               MIN_VALUE_HOSTNAME,             MAX_VALUE_HOSTNAME),
    m_brightness            (m_preferences, KEY_BRIGHTNESS,             NAME_BRIGHTNESS,            DEFAULT_BRIGHTNESS,             MIN_VALUE_BRIGHTNESS,           MAX_VALUE_BRIGHTNESS),
    m_autoBrightnessCtrl    (m_preferences, KEY_AUTO_BRIGHTNESS_CTRL,   NAME_AUTO_BRIGHTNESS_CTRL,  DEFAULT_AUTO_BRIGHTNESS_CTRL),
    m_timezone              (m_preferences, KEY_TIMEZONE,               NAME_TIMEZONE,              DEFAULT_TIMEZONE,               MIN_VALUE_TIMEZONE,             MAX_VALUE_TIMEZONE),
    m_ntpServer             (m_preferences, KEY_NTP_SERVER,             NAME_NTP_SERVER,            DEFAULT_NTP_SERVER,             MIN_VALUE_NTP_SERVER,           MAX_VALUE_NTP_SERVER),
    m_maxSlots              (m_preferences, KEY_MAX_SLOTS,              NAME_MAX_SLOTS,             DEFAULT_MAX_SLOTS,              MIN_MAX_SLOTS,                  MAX_MAX_SLOTS),
    m_scrollPause           (m_preferences, KEY_SCROLL_PAUSE,           NAME_SCROLL_PAUSE,          DEFAULT_SCROLL_PAUSE,           MIN_VALUE_SCROLL_PAUSE,         MAX_VALUE_SCROLL_PAUSE),
    m_notifyURL             (m_preferences, KEY_NOTIFY_URL,             NAME_NOTIFY_URL,            DEFAULT_NOTIFY_URL,             MIN_VALUE_NOTIFY_URL,           MAX_VALUE_NOTIFY_URL),
    m_quietMode             (m_preferences, KEY_QUIET_MODE,             NAME_QUIET_MODE,            DEFAULT_QUIET_MODE)
{

    /* Skip m_version, because it shall not be modified by the user. */
    m_keyValueList.push_back(&m_wifiSSID);
    m_keyValueList.push_back(&m_wifiPassphrase);
    m_keyValueList.push_back(&m_apSSID);
    m_keyValueList.push_back(&m_apPassphrase);
    m_keyValueList.push_back(&m_webLoginUser);
    m_keyValueList.push_back(&m_webLoginPassword);
    m_keyValueList.push_back(&m_hostname);
    m_keyValueList.push_back(&m_brightness);
    m_keyValueList.push_back(&m_autoBrightnessCtrl);
    m_keyValueList.push_back(&m_timezone);
    m_keyValueList.push_back(&m_ntpServer);
    m_keyValueList.push_back(&m_maxSlots);
    m_keyValueList.push_back(&m_scrollPause);
    m_keyValueList.push_back(&m_notifyURL);
    m_keyValueList.push_back(&m_quietMode);
}

SettingsService::~SettingsService()
{
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

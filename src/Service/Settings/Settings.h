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
 * @brief  Settings
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup settings
 *
 * @{
 */

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Preferences.h>
#include "KeyValue.h"
#include "KeyValueString.h"
#include "KeyValueBool.h"
#include "KeyValueUInt8.h"
#include "KeyValueInt32.h"
#include "KeyValueUInt32.h"
#include "KeyValueJson.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Settings class for easy access to persistent stored key:value pairs.
 */
class Settings
{
public:

    /**
     * Get the settings instance.
     *
     * @return Settings instance
     */
    static Settings& getInstance()
    {
        static Settings instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
    }

    /**
     * Open settings.
     * If the settings storage doesn't exist, it will be created.
     *
     * @param[in] readOnly  Open read only or read/write
     *
     * @return Status
     * @retval false    Failed to open
     * @retval true     Successful opened
     */
    bool open(bool readOnly);

    /**
     * Close settings.
     */
    void close();

    /**
     * Get remote wifi network SSID.
     *
     * @return Key value pair
     */
    KeyValueString& getWifiSSID()
    {
        return m_wifiSSID;
    }

    /**
     * Get remote wifi network passphrase.
     *
     * @return Key value pair
     */
    KeyValueString& getWifiPassphrase()
    {
        return m_wifiPassphrase;
    }

    /**
     * Get wifi access point network SSID.
     *
     * @return Key value pair
     */
    KeyValueString& getWifiApSSID()
    {
        return m_apSSID;
    }

    /**
     * Get wifi access point network passphrase.
     *
     * @return Key value pair
     */
    KeyValueString& getWifiApPassphrase()
    {
        return m_apPassphrase;
    }

    /**
     * Get website login user account.
     *
     * @return Website login user account
     */
    KeyValueString& getWebLoginUser()
    {
        return m_webLoginUser;
    }

    /**
     * Get website login user password.
     *
     * @return Website login user password
     */
    KeyValueString& getWebLoginPassword()
    {
        return m_webLoginPassword;
    }

    /**
     * Get hostname.
     *
     * @return Key value pair
     */
    KeyValueString& getHostname()
    {
        return m_hostname;
    }

    /**
     * Get brightness in %.
     *
     * @return Key value pair
     */
    KeyValueUInt8& getBrightness()
    {
        return m_brightness;
    }

    /**
     * Get state of automatic brightness adjustment.
     *
     * @return Key value pair
     */
    KeyValueBool& getAutoBrightnessAdjustment()
    {
        return m_autoBrightnessCtrl;
    }

    /**
     * Get POSIX timezone string.
     *
     * @return POSIC timezone string
     */
    KeyValueString& getTimezone()
    {
        return m_timezone;
    }

    /**
     * Get NTP server address.
     *
     * @return Key value pair
     */
    KeyValueString& getNTPServerAddress()
    {
        return m_ntpServer;
    }

    /**
     * Get time format according to strftime().
     *
     * @return Key value pair
     */
    KeyValueString& getTimeFormat()
    {
        return m_timeFormat;
    }

    /**
     * Get date format according to strftime().
     *
     * @return Key value pair
     */
    KeyValueString& getDateFormat()
    {
        return m_dateFormat;
    }

    /**
     * Get max. number of display slots.
     *
     * @return Key value pair
     */
    KeyValueUInt8& getMaxSlots()
    {
        return m_maxSlots;
    }

    /**
     * Get scrolling pause.
     *
     * @return Key value pair
     */
    KeyValueUInt32& getScrollPause()
    {
        return m_scrollPause;
    }

   /**
     * Get notifyURL.
     *
     * @return The URL to be triggered
     */
    KeyValueString& getNotifyURL()
    {
        return m_notifyURL;
    }
    /**
     * Get a list of all key value pairs.
     *
     * @return List of key value pairs.
     */
    KeyValue**  getList()
    {
        return m_keyValueList;
    }

    /**
     * Clear all key value pairs, which means set them to
     * factory defaults.
     *
     * @return If successful cleared, it will return true otherwise false.
     */
    bool clear()
    {
        return m_preferences.clear();
    }

    /**
     * Get key value pair by key.
     * 
     * @param key The key is used to search for the right key value pair.
     * @return If key is found, it will return the key value pair otherwise nullptr.
     */
    KeyValue* getSettingByKey(const char* key);

    /** Number of key value pairs. */
    static const uint8_t KEY_VALUE_PAIR_NUM = 16U;

private:

    Preferences     m_preferences;                      /**< Persistent storage */
    KeyValue*       m_keyValueList[KEY_VALUE_PAIR_NUM]; /**< List of all key value pairs */

    KeyValueString  m_wifiSSID;             /**< Remote wifi network SSID */
    KeyValueString  m_wifiPassphrase;       /**< Remote wifi network passphrase */
    KeyValueString  m_apSSID;               /**< Access point SSID */
    KeyValueString  m_apPassphrase;         /**< Access point passphrase */
    KeyValueString  m_webLoginUser;         /**< Website login user account */
    KeyValueString  m_webLoginPassword;     /**< Website login user password */
    KeyValueString  m_hostname;             /**< Hostname */
    KeyValueUInt8   m_brightness;           /**< The brightness level in % set at startup. */
    KeyValueBool    m_autoBrightnessCtrl;   /**< Automatic brightness control switch */
    KeyValueString  m_timezone;             /**< POSIX timezone string */
    KeyValueString  m_ntpServer;            /**< NTP server address */
    KeyValueString  m_timeFormat;           /**< Time format according to strftime(). */
    KeyValueString  m_dateFormat;           /**< Date format according to strftime(). */
    KeyValueUInt8   m_maxSlots;             /**< Max. number of display slots. */
    KeyValueUInt32  m_scrollPause;          /**< Text scroll pause */
    KeyValueString  m_notifyURL;            /**< URL to be triggered when PIXELIX has connected to a remote network. */

    /**
     * Constructs the settings instance.
     */
    Settings();

    /**
     * Destroys the settings instance.
     */
    ~Settings();

    /* An instance shall not be copied. */
    Settings(const Settings& settings);
    Settings& operator=(const Settings& settings);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __SETTINGS_H__ */

/** @} */
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
 *
 * @addtogroup utilities
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
     * Get hostname.
     *
     * @return Key value pair
     */
    KeyValueString& getHostname()
    {
        return m_hostname;
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
     * Get plugin installation.
     *
     * @return Key value pair
     */
    KeyValueJson& getPluginInstallation()
    {
        return m_pluginInstallation;
    }

    /**
     * Get GMT offset.
     *
     * @return Key value pair
     */
    KeyValueInt32& getGmtOffset()
    {
        return m_gmtOffset;
    }

    /**
     * Get state of daylight saving adjustment.
     *
     * @return Key value pair
     */
    KeyValueBool& getDaylightSavingAdjustment()
    {
        return m_isDaylightSaving;
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
     * Get state of time format adjustment.
     *
     * @return Key value pair
     */
    KeyValueBool& getTimeFormatAdjustment()
    {
        return m_timeFormatCtrl;
    }

    /**
     * Get state of date format adjustment.
     *
     * @return Key value pair
     */
    KeyValueBool& getDateFormatAdjustment()
    {
        return m_dateFormatCtrl;
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
     * Get display slot configuration.
     *
     * @return Key value pair
     */
    KeyValueJson& getDisplaySlotConfig()
    {
        return m_slotConfig;
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

    /** Number of key value pairs. */
    static const uint8_t KEY_VALUE_PAIR_NUM = 15U;

private:

    Preferences     m_preferences;                      /**< Persistent storage */
    KeyValue*       m_keyValueList[KEY_VALUE_PAIR_NUM]; /**< List of all key value pairs */

    KeyValueString  m_wifiSSID;             /**< Remote wifi network SSID */
    KeyValueString  m_wifiPassphrase;       /**< Remote wifi network passphrase */
    KeyValueString  m_apSSID;               /**< Access point SSID */
    KeyValueString  m_apPassphrase;         /**< Access point passphrase */
    KeyValueString  m_hostname;             /**< Hostname */
    KeyValueBool    m_autoBrightnessCtrl;   /**< Automatic brightness control switch */
    KeyValueJson    m_pluginInstallation;   /**< Plugin installation */
    KeyValueInt32   m_gmtOffset;            /**< GMT offset */
    KeyValueBool    m_isDaylightSaving;     /**< Daylight saving time switch */
    KeyValueString  m_ntpServer;            /**< NTP server address */
    KeyValueBool    m_timeFormatCtrl;       /**< Time format control */
    KeyValueBool    m_dateFormatCtrl;       /**< Date format control */
    KeyValueUInt8   m_maxSlots;             /**< Max. number of display slots. */
    KeyValueJson    m_slotConfig;           /**< Display slot configuration */
    KeyValueUInt32  m_scrollPause;          /**< Text scroll pause */

    /**
     * Constructs the settings instance.
     */
    Settings();

    /**
     * Destroys the i/o pin instance.
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
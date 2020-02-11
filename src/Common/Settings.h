/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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
        return m_instance;
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
    KeyValueString& getPluginInstallation()
    {
        return m_pluginInstallation;
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
    static const uint8_t KEY_VALUE_PAIR_NUM = 7U;

private:

    static Settings m_instance;     /**< Settings instance */

    Preferences     m_preferences;                      /**< Persistent storage */
    KeyValue*       m_keyValueList[KEY_VALUE_PAIR_NUM]; /**< List of all key value pairs */

    KeyValueString  m_wifiSSID;             /**< Remote wifi network SSID */
    KeyValueString  m_wifiPassphrase;       /**< Remote wifi network passphrase */
    KeyValueString  m_apSSID;               /**< Access point SSID */
    KeyValueString  m_apPassphrase;         /**< Access point passphrase */
    KeyValueString  m_hostname;             /**< Hostname */
    KeyValueBool    m_autoBrightnessCtrl;   /**< Automatic brightness control switch */
    KeyValueString  m_pluginInstallation;   /**< Plugin installation */

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
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
 *
 * @addtogroup settings
 *
 * @{
 */

#ifndef SETTINGS_SERVICE_H
#define SETTINGS_SERVICE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Preferences.h>
#include <IService.hpp>
#include <vector>

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
 * Persistent storage of key value pairs.
 */
class SettingsService : public IService
{
public:

    /**
     * Get settings service.
     * 
     * @return Settings service
     */
    static SettingsService& getInstance()
    {
        static SettingsService instance; /* idiom */

        return instance;
    }

    /**
     * Start the service.
     * 
     * @return If successful started, it will return true otherwise false.
     */
    bool start() final;

    /**
     * Stop the service.
     */
    void stop() final;

    /**
     * Process the service.
     */
    void process() final;

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
     * Remove obsolete keys in the persistency. It can be used to prevent a
     * growing up persistency with obsolete key/value pairs.
     * 
     * The clean-up itself is only performed if the stored version number is
     * different from the settings version number.
     * 
     * Note, the settings must be opened in write mode!
     */
    void cleanUp();

    /**
     * Get key value pair by key.
     * 
     * @param key The key is used to search for the right key value pair.
     * @return If key is found, it will return the key value pair otherwise nullptr.
     */
    KeyValue* getSettingByKey(const char* key);

    /**
     * Register a single setting.
     * 
     * @param[in] setting   Setting which to register
     * 
     * @return If successful, it will return true otherwise false.
     */
    bool registerSetting(KeyValue* setting);

    /**
     * Unregister setting.
     * 
     * @param[in] setting   Setting which to unregister
     */
    void unregisterSetting(KeyValue* setting);

    /**
     * Get a list of all key value pairs.
     *
     * @param[out] count    Number of key value pairs in the list.
     * 
     * @return List of key value pairs.
     */
    KeyValue** getList(size_t& count)
    {
        count = m_keyValueList.size();

        return m_keyValueList.data();
    }

    /**
     * Clear all key value pairs, which means set them to factory defaults.
     *
     * @return If successful cleared, it will return true otherwise false.
     */
    bool clear()
    {
        return m_preferences.clear();
    }

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
    * Get quite mode state.
    *
    * @return Is quiet mode enabled or not?
    */
    KeyValueBool& getQuietMode()
    {
        return m_quietMode;
    }

    /**
     * Settings version
     * The version number shall be increased by 1 after:
     * - a new setting was added or
     * - a existing setting changed
     * - a existing setting was removed
     */
    static const uint32_t   VERSION = 2U;

private:

    Preferences             m_preferences;          /**< Persistent storage */
    std::vector<KeyValue*>  m_keyValueList;         /**< List of key/value pairs, stored in persistent storage. */

    KeyValueUInt32          m_version;              /**< Settings version (just an consequtive incremented number) */
    KeyValueString          m_wifiSSID;             /**< Remote wifi network SSID */
    KeyValueString          m_wifiPassphrase;       /**< Remote wifi network passphrase */
    KeyValueString          m_apSSID;               /**< Access point SSID */
    KeyValueString          m_apPassphrase;         /**< Access point passphrase */
    KeyValueString          m_webLoginUser;         /**< Website login user account */
    KeyValueString          m_webLoginPassword;     /**< Website login user password */
    KeyValueString          m_hostname;             /**< Hostname */
    KeyValueUInt8           m_brightness;           /**< The brightness level in % set at startup. */
    KeyValueBool            m_autoBrightnessCtrl;   /**< Automatic brightness control switch */
    KeyValueString          m_timezone;             /**< POSIX timezone string */
    KeyValueString          m_ntpServer;            /**< NTP server address */
    KeyValueUInt8           m_maxSlots;             /**< Max. number of display slots. */
    KeyValueUInt32          m_scrollPause;          /**< Text scroll pause */
    KeyValueString          m_notifyURL;            /**< URL to be triggered when PIXELIX has connected to a remote network. */
    KeyValueBool            m_quietMode;            /**< Quiet mode (skip unnecessary system messages) */

    /**
     * Constructs the settings service instance.
     */
    SettingsService();

    /**
     * Destroys the settings service instance.
     */
    ~SettingsService();

    /* An instance shall not be copied. */
    SettingsService(const SettingsService& service);
    SettingsService& operator=(const SettingsService& service);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* SETTINGS_SERVICE_H */

/** @} */
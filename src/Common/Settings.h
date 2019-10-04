/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
    bool open(bool readOnly)
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

    /**
     * Close settings.
     */
    void close()
    {
        m_preferences.end();
        return;
    }

    /**
     * Get remote wifi network SSID.
     * If there is no SSID available, a empty string will be returned.
     * 
     * @return SSID
     */
    String getWifiSSID()
    {
        return m_preferences.getString(KEY_WIFI_SSID, "");
    }

    /**
     * Store a remote wifi network SSID.
     * 
     * @param[in] ssid  Wifi network SSID
     */
    void setWifiSSID(const String& ssid)
    {
        m_preferences.putString(KEY_WIFI_SSID, ssid);
        return;
    }

    /**
     * Get remote wifi network passphrase.
     * If there is no passphrase available, a empty string will be returned.
     * 
     * @return Passphrase
     */
    String getWifiPassphrase()
    {
        return m_preferences.getString(KEY_WIFI_PASSPHRASE, "");
    }

    /**
     * Store a remote wifi network passphrase.
     * 
     * @param[in] passphrase Wifi network passphrase
     */
    void setWifiPassphrase(const String& passphrase)
    {
        m_preferences.putString(KEY_WIFI_PASSPHRASE, passphrase);
        return;
    }

private:

    static Settings     m_instance;     /**< Settings instance */

    Preferences         m_preferences;  /**< Persistent storage */

    static const char*  PREF_NAMESPACE;         /**< Settings namespace used for preferences */
    static const char*  KEY_WIFI_SSID;          /**< Wifi network key */
    static const char*  KEY_WIFI_PASSPHRASE;    /**< Wifi network passphrase */

    /**
     * Constructs the settings instance.
     */
    Settings() :
        m_preferences()
    {
    }

    /**
     * Destroys the i/o pin instance.
     */
    ~Settings()
    {
    }

    /* An instance shall not be copied. */
    Settings(const Settings& settings);
    Settings& operator=(const Settings& settings);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __SETTINGS_H__ */

/** @} */
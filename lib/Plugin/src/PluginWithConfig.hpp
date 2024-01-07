/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Plugin base with configuration
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef PLUGIN_WITH_CONFIG_HPP
#define PLUGIN_WITH_CONFIG_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Plugin.hpp"

#include <SimpleTimer.hpp>
#include <JsonFile.h>
#include <ArduinoJson.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This base plugin class handles additional a configuration file in the
 * filesystem to store data persistent.
 * 
 * Attention: Every derived class must call start(), stop() and process()
 * of this base class to get the configuration file handling working!
 */
class PluginWithConfig : public Plugin
{
public:

    /**
     * Destroy the base plugin.
     */
    ~PluginWithConfig()
    {
    }

    /**
     * Start the plugin. This is called only once during plugin lifetime.
     * It can be used as deferred initialization (after the constructor)
     * and provides the canvas size.
     * 
     * If your display layout depends on canvas or font size, calculate it
     * here.
     * 
     * Overwrite it if your plugin needs to know that it was installed.
     * 
     * @param[in] width     Display width in pixel
     * @param[in] height    Display height in pixel
     */
    void start(uint16_t width, uint16_t height) override
    {
        PLUGIN_NOT_USED(width);
        PLUGIN_NOT_USED(height);

        /* Try to load configuration. If there is no configuration available, a default configuration
         * will be created.
         */
        if (false == loadConfiguration())
        {
            (void)saveConfiguration();
        }
        else
        {
            /* Remember current timestamp to detect updates of the configuration in the
             * filesystem without using the plugin API.
             */
            m_timestampOfLastFileUpdate = getLastConfigurationUpdate();
        }

        m_cfgReloadTimer.start(CFG_RELOAD_PERIOD);
    }

    /**
     * Stop the plugin. This is called only once during plugin lifetime.
     * It can be used as a first clean-up, before the plugin will be destroyed.
     * 
     * Overwrite it if your plugin needs to know that it will be uninstalled.
     */
    void stop() override
    {
        m_cfgReloadTimer.stop();
        (void)m_fs.remove(getFullPathToConfiguration());
    }

    /**
     * Process the plugin.
     * Overwrite it if your plugin has cyclic stuff to do without being in a
     * active slot.
     * 
     * @param[in] isConnected   The network connection status. If network
     *                          connection is established, it will be true otherwise false.
     */
    void process(bool isConnected) override
    {
        PLUGIN_NOT_USED(isConnected);

        /* Configuration in persistent memory updated? */
        if ((true == m_cfgReloadTimer.isTimerRunning()) &&
            (true == m_cfgReloadTimer.isTimeout()))
        {
            if (true == isConfigurationUpdated())
            {
                m_reloadConfigReq = true;
            }

            m_cfgReloadTimer.restart();
        }

        if (true == m_storeConfigReq)
        {
            (void)saveConfiguration();
            m_storeConfigReq = false;
        }
        else if (true == m_reloadConfigReq)
        {
            if (true == loadConfiguration())
            {
                m_timestampOfLastFileUpdate = getLastConfigurationUpdate();
            }

            m_reloadConfigReq = false;
        }
        else
        {
            ;
        }
    }

protected:

    PluginWithConfig();
    PluginWithConfig(const PluginWithConfig& handler);
    PluginWithConfig& operator=(const PluginWithConfig& handler);

    /**
     * Construct the base plugin.
     * 
     * @param[in] name  Plugin name (must exist over lifetime)
     * @param[in] uid   Unique id
     * @param[in] fs    The filesystem where to load and save the configuration file.
     */
    PluginWithConfig(const char* name, uint16_t uid, FS& fs) :
        Plugin(name, uid),
        m_fs(fs),
        m_timestampOfLastFileUpdate(0U),
        m_cfgReloadTimer(),
        m_storeConfigReq(false),
        m_reloadConfigReq(false)
    {
    }

    /**
     * Get configuration in JSON.
     * 
     * @param[out] cfg  Configuration
     */
    virtual void getConfiguration(JsonObject& cfg) const = 0;

    /**
     * Set configuration in JSON.
     * 
     * @param[in] cfg   Configuration
     * 
     * @return If successful set, it will return true otherwise false.
     */
    virtual bool setConfiguration(JsonObjectConst& cfg) = 0;

    /**
     * Request to store configuration to persistent memory.
     */
    void requestStoreToPersistentMemory()
    {
        m_storeConfigReq = true;
    }

    /**
     * Get full path (path + filename) to plugin instance specific configuration
     * in JSON format.
     * 
     * @return Full path to configuration file
     */
    String getFullPathToConfiguration() const
    {
        return generateFullPath(getUID(), ".json");
    }

    /**
     * Get timestamp of the last configuration update in the persistent memory.
     * 
     * @return Timestamp in unix time format.
     */
    time_t getLastConfigurationUpdate() const
    {
        String  configurationFilename   = getFullPathToConfiguration();
        File    fd                      = m_fs.open(configurationFilename, "r");
        time_t  timestamp               = 0U;

        if (true == fd)
        {
            timestamp = fd.getLastWrite();
            fd.close();
        }

        return timestamp;
    }

    /**
     * Is the configuration in persistent memory updated without using the
     * plugin API?
     * 
     * @return If updated, it will return true otherwise false.
     */
    bool isConfigurationUpdated() const
    {
        bool    isConfigurationUpdated      = false;
        time_t  timestampOfLastFileUpdate   = getLastConfigurationUpdate();
        
        if (timestampOfLastFileUpdate != m_timestampOfLastFileUpdate)
        {
            isConfigurationUpdated = true;
        }

        return isConfigurationUpdated;
    }

    /**
     * Saves current configuration to JSON file.
     * 
     * @return If successful saved, it will return true otherwise false.
     */
    bool saveConfiguration()
    {
        bool                status                  = true;
        JsonFile            jsonFile(m_fs);
        const size_t        JSON_DOC_SIZE           = 1024U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonRootObject          = jsonDoc.to<JsonObject>();
        String              configurationFilename   = getFullPathToConfiguration();

        getConfiguration(jsonRootObject);
        
        if (false == jsonFile.save(configurationFilename, jsonDoc))
        {
            status = false;
        }
        else
        {
            m_timestampOfLastFileUpdate = getLastConfigurationUpdate();
        }

        return status;
    }

    /**
     * Load configuration from JSON file.
     * 
     * @return If successful loaded, it will return true otherwise false.
     */
    bool loadConfiguration()
    {
        bool                status                  = true;
        JsonFile            jsonFile(m_fs);
        const size_t        JSON_DOC_SIZE           = 1024U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObjectConst     jsonRootObject          = jsonDoc.to<JsonObject>();
        String              configurationFilename   = getFullPathToConfiguration();

        if (false == jsonFile.load(configurationFilename, jsonDoc))
        {
            status = false;
        }
        else
        {
            status = setConfiguration(jsonRootObject);
        }

        return status;
    }

private:

    /**
     * The configuration in the persistent memory shall be cyclic loaded.
     * This mechanism ensure that manual changes in the file are considered.
     * This is the reload period in ms.
     */
    static const uint32_t CFG_RELOAD_PERIOD = SIMPLE_TIMER_SECONDS(30U);

    FS&         m_fs;                           /**< Filesystem used to load and save the configuration file. */
    time_t      m_timestampOfLastFileUpdate;    /**< Configuration in persistent memory written the last time (unix timeformat). */
    SimpleTimer m_cfgReloadTimer;               /**< Timer is used to cyclic reload the configuration from persistent memory. */
    bool        m_storeConfigReq;               /**< Is requested to store the configuration in persistent memory? */
    bool        m_reloadConfigReq;              /**< Is requested to reload the configuration from persistent memory? */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* PLUGIN_WITH_CONFIG_HPP */

/** @} */
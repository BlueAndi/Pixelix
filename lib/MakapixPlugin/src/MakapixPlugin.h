/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @file   MakapixPlugin.h
 * @brief  Makapix player plugin
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef MAKAPIX_PLUGIN_H
#define MAKAPIX_PLUGIN_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "./internal/View.h"

#include <stdint.h>
#include <PluginWithConfig.hpp>
#include <Mutex.hpp>
#include <FileSystem.h>
#include <FileMgrService.h>
#include <HttpServiceTypes.h>
#include <HttpFileResponseHandler.h>
#include <SimpleTimer.hpp>
#include <MqttTypes.h>
#include "FileCache.h"
#include "Playlist.h"
#include "ArtworkDownloader.h"
#include "CommandHandler.h"
#include "RequestHandler.h"
#include "Channel.h"
#include "MakapixTypes.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Makapix player plugin class.
 * https://makapix.club/
 *
 * Protocol documentation:
 * https://github.com/fabkury/makapix/blob/main/docs/PHYSICAL_PLAYER.md
 *
 * The plugin supports only one instance.
 */
class MakapixPlugin : public PluginWithConfig
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name (must exist over lifetime)
     * @param[in] uid   Unique id
     */
    MakapixPlugin(const char* name, uint16_t uid);

    /**
     * Destroys the plugin.
     */
    ~MakapixPlugin();

    /**
     * Plugin creation method, used to register on the plugin manager.
     *
     * @param[in] name  Plugin name (must exist over lifetime)
     * @param[in] uid   Unique id
     *
     * @return If successful, it will return the pointer to the plugin instance, otherwise nullptr.
     */
    static IPluginMaintenance* create(const char* name, uint16_t uid)
    {
        IPluginMaintenance* plugin = nullptr;

        if (MAX_INSTANCE_COUNT > instanceCnt)
        {
            plugin = new (std::nothrow) MakapixPlugin(name, uid);

            if (nullptr != plugin)
            {
                ++instanceCnt;
            }
        }

        return plugin;
    }

    /**
     * Get plugin topics, which can be get/set via different communication
     * interfaces like REST, websocket, MQTT, etc.
     *
     * Example:
     * <code>{.json}
     * {
     *     "topics": [
     *         "text"
     *     ]
     * }
     * </code>
     *
     * By default a topic is readable and writeable.
     * This can be set explicit with the "access" key with the following possible
     * values:
     * - Only readable: "r"
     * - Only writeable: "w"
     * - Readable and writeable: "rw"
     *
     * Example:
     * <code>{.json}
     * {
     *     "topics": [{
     *         "name": "text",
     *         "access": "r"
     *     }]
     * }
     * </code>
     *
     * Home Assistant MQTT discovery support can be added with the "ha" JSON object inside
     * the "extra" JSON object. The Home Assistant extension supports only loading by file.
     * <code>{.json}
     * {
     *     "topics": [{
     *         "name": "text",
     *         "extra": {
     *             "ha": "myHomeAssistantConfig.json"
     *         }
     *     }]
     * }
     * </code>
     *
     * Extra information can be loaded from a file too. This is useful for complex
     * configurations and to keep program memory usage low.
     * <code>{.json}
     * {
     *     "topics": [{
     *         "name": "text",
     *         "extra": "extra.json"
     *    }]
     * }
     * </code>
     *
     * @param[out] topics   Topis in JSON format
     */
    void getTopics(JsonArray& topics) const final;

    /**
     * Get a topic data.
     * Note, currently only JSON format is supported.
     *
     * @param[in]   topic   The topic which data shall be retrieved.
     * @param[out]  value   The topic value in JSON format.
     *
     * @return If successful it will return true otherwise false.
     */
    bool getTopic(const String& topic, JsonObject& value) const final;

    /**
     * Set a topic data.
     * Note, currently only JSON format is supported.
     *
     * @param[in]   topic   The topic which data shall be retrieved.
     * @param[in]   value   The topic value in JSON format.
     *
     * @return If successful it will return true otherwise false.
     */
    bool setTopic(const String& topic, const JsonObjectConst& value) final;

    /**
     * Is the topic content changed since last time?
     * Every readable volatile topic shall support this. Otherwise the topic
     * handlers might not be able to provide updated information.
     *
     * @param[in] topic The topic which to check.
     *
     * @return If the topic content changed since last time, it will return true otherwise false.
     */
    bool hasTopicChanged(const String& topic) final;

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
    void start(uint16_t width, uint16_t height) final;

    /**
     * Stop the plugin. This is called only once during plugin lifetime.
     * It can be used as a first clean-up, before the plugin will be destroyed.
     *
     * Overwrite it if your plugin needs to know that it will be uninstalled.
     */
    void stop() final;

    /**
     * Process the plugin.
     * Overwrite it if your plugin has cyclic stuff to do without being in a
     * active slot.
     *
     * @param[in] isConnected   The network connection status. If network
     *                          connection is established, it will be true otherwise false.
     */
    void process(bool isConnected) final;

    /**
     * Update the display.
     * The scheduler will call this method periodically.
     *
     * @param[in] gfx   Display graphics interface
     */
    void update(YAGfx& gfx) final;

private:

    /** Display modes. */
    typedef enum
    {
        DISPLAY_MODE_SINGLE_ARTWORK = 0, /**< Single artwork display mode. */
        DISPLAY_MODE_PLAY_CHANNEL        /**< Play channel mode. */

    } DisplayMode;

    /** Maximum number of instances allowed. */
    static const uint32_t MAX_INSTANCE_COUNT = 1U;

    /** Instance counter to keep track of the number of instances. */
    static uint32_t instanceCnt;

    /**
     * Plugin topic, used to read/write the configuration.
     */
    static const char* TOPIC_CONFIG;

    /**
     * Plugin topic, used to control playback (next, previous, play channel).
     */
    static const char* TOPIC_PLAY_CONTROL;

    /**
     * Artwork cache location path where to store downloaded artwork files.
     */
    static const char* ARTWORK_CACHE_PATH;

    /** Default channel name to play on start. */
    static const char*     DEFAULT_CHANNEL_NAME;

    _MakapixPlugin::View   m_view;                 /**< View with all widgets. */
    String                 m_playerKey;            /**< Makapix player key. */
    uint8_t                m_mqttInstance;         /**< MQTT instance index. */
    mutable MutexRecursive m_mutex;                /**< Mutex to protect against concurrent access. */
    bool                   m_hasTopicChanged;      /**< Has the topic content changed? */
    DisplayMode            m_displayMode;          /**< Current display mode. */
    FileCache              m_fileCache;            /**< File cache for downloaded artwork files. */
    Playlist               m_playlist;             /**< Local playlist of artworks. */
    ArtworkDownloader      m_artworkDownloader;    /**< Artwork downloader. */
    bool                   m_isDownloadingArtwork; /**< Is artwork download in progress? */
    CommandHandler         m_commandHandler;       /**< Command handler. */
    Channel                m_channel;              /**< Artwork channel. */
    SimpleTimer            m_dwellTimer;           /**< Timer for dwell time between artwork changes. */
    String                 m_currentFilePath;      /**< Current artwork file path. */
    int32_t                m_currentPlaylistIdx;   /**< Current playlist index. */

    /**
     * Get configuration in JSON.
     *
     * @param[out] jsonCfg   Configuration
     */
    void getConfiguration(JsonObject& jsonCfg) const final;

    /**
     * Set configuration in JSON.
     *
     * @param[in] jsonCfg   Configuration
     *
     * @return If successful set, it will return true otherwise false.
     */
    bool setConfiguration(const JsonObjectConst& jsonCfg) final;

    /**
     * Process artwork download.
     */
    void processArtworkDownload();

    /**
     * Process display mode.
     */
    void processDisplayMode();

    /**
     * Show artwork and start dwell timer.
     * If artwork is cached, it will be shown directly otherwise it will be downloaded first.
     * If a download is in progress, it will fail.
     *
     * @param[in] artUrl       Artwork URL.
     * @param[in] storageKey   Storage key.
     * @param[in] dwellTime    Dwell time in ms.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool showArtwork(const String& artUrl, const String& storageKey, uint32_t dwellTime);

    /**
     * Show next artwork from playlist.
     *
     * If at the end of the playlist, it will wrap around to the beginning.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool nextArtwork();

    /**
     * Show previous artwork from playlist.
     *
     * If at the beginning of the playlist, it will wrap around to the end.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool prevArtwork();

    /**
     * Play a channel by its name.
     *
     * @param[in] channelName  Channel name
     *
     * @return If successful, it will return true otherwise false.
     */
    bool playChannel(const char* channelName);

    /**
     * Get cache file path for artwork URL.
     *
     * @param[in] artUrl    Artwork URL
     *
     * @return Cache file path
     */
    String getCacheFilePath(const char* artUrl) const;

    /**
     * Get file cache ID for storage key.
     *
     * @param[in] storageKey   Storage key
     *
     * @return File cache ID
     */
    String getCacheFileId(const String& storageKey) const;

    /**
     * Command callback to play the next artwork.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool cmdNextArtwork();

    /**
     * Command callback to play the previous artwork.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool cmdPrevArtwork();

    /**
     * Command callback to play a channel.
     *
     * @param[in] channelName  Channel name
     *
     * @return If successful, it will return true otherwise false.
     */
    bool cmdPlayChannel(const char* channelName);

    /**
     * Command callback to show the current selected artwork.
     */
    void cmdShowArtwork();

    /**
     * Command callback to pause the playback.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool cmdPause();

    /**
     * Command callback to continue the playback.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool cmdContinue();

    /**
     * Command callback to get the current status.
     *
     * @param[out] status   Status in JSON format
     *
     * @return If successful, it will return true otherwise false.
     */
    bool cmdGetStatus(JsonObject& jsonStatus) const;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* MAKAPIX_PLUGIN_H */

/** @} */

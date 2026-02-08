/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   MakapixPlugin.cpp
 * @brief  Makapix player plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MakapixPlugin.h"

#include <Logging.h>
#include <ArduinoJson.h>
#include <MqttService.h>
#include <HttpService.h>
#include <FileSystem.h>
#include <Version.h>

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

/** Initialize instance counter. */
uint32_t MakapixPlugin::instanceCnt              = 0U;

/* Initialize plugin topic. */
const char* MakapixPlugin::TOPIC_CONFIG          = "makapixCfg";
const char* MakapixPlugin::TOPIC_PLAY_CONTROL    = "makapixPlayCtrl";
const char* MakapixPlugin::TOPIC_PROVISION       = "makapixProvision";
const char* MakapixPlugin::ARTWORK_CACHE_PATH    = "/tmp/";
const char* MakapixPlugin::DEFAULT_CHANNEL_NAME  = "promoted";
const char* MakapixPlugin::MAKAPIX_PROVISION_URL = "https://makapix.club/api/player/provision";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

MakapixPlugin::MakapixPlugin(const char* name, uint16_t uid) :
    PluginWithConfig(name, uid, FILESYSTEM),
    m_view(),
    m_playerKey(),
    m_mqttInstance(0U),
    m_mutex(),
    m_hasTopicChanged(false),
    m_displayMode(DISPLAY_MODE_PLAY_CHANNEL),
    m_fileCache(),
    m_playlist(),
    m_artworkDownloader(),
    m_isDownloadingArtwork(false),
    m_commandHandler(m_playlist, m_channel),
    m_channel(m_playlist),
    m_dwellTimer(),
    m_isPaused(false),
    m_isActive(false),
    m_currentFilePath(),
    m_currentPlaylistIdx(m_playlist.selected()),
    m_viewUpdate(m_channel),
    m_provisionHttpJobId(INVALID_HTTP_JOB_ID),
    m_provisionPayload(),
    m_registrationCode(),
    m_registrationCodeExpiresAt(),
    m_mqttBrokerHost(),
    m_mqttBrokerPort(0U)
{
    MakapixNextArtworkCallback cmdNextArtworkCb =
        [this]() {
            (void)this->cmdNextArtwork();
        };
    MakapixPrevArtworkCallback cmdPrevArtworkCb =
        [this]() {
            (void)this->cmdPrevArtwork();
        };
    MakapixPlayChannelCallback cmdPlayChannelCb =
        [this](const char* channelName) {
            (void)this->cmdPlayChannel(channelName);
        };
    MakapixShowArtworkCallback cmdShowArtworkCb =
        [this]() {
            this->cmdShowArtwork();
        };
    MakapixNextArtworkCallback channelNextArtworkCb =
        [this]() {
            (void)this->nextArtwork();
        };

    m_commandHandler.init(cmdNextArtworkCb, cmdPrevArtworkCb, cmdPlayChannelCb, cmdShowArtworkCb);
    m_channel.init(channelNextArtworkCb);

    (void)m_mutex.create();
}

/**
 * Destroys the plugin.
 */
MakapixPlugin::~MakapixPlugin()
{
    m_mutex.destroy();
}

void MakapixPlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC_CONFIG);
    (void)topics.add(TOPIC_PLAY_CONTROL);
    (void)topics.add(TOPIC_PROVISION);
}

bool MakapixPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    /* Configuration topic? */
    if (true == topic.equals(TOPIC_CONFIG))
    {
        getConfiguration(value);
        isSuccessful = true;
    }
    /* Play control topic? */
    else if (true == topic.equals(TOPIC_PLAY_CONTROL))
    {
        isSuccessful = cmdGetStatus(value);
    }
    /* Provision topic? */
    else if (true == topic.equals(TOPIC_PROVISION))
    {
        isSuccessful = cmdGetProvisionStatus(value);
    }
    /* Unknown topic. */
    else
    {
        /* Nothing to do. */
        ;
    }

    return isSuccessful;
}

bool MakapixPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    /* Configuration topic? */
    if (true == topic.equals(TOPIC_CONFIG))
    {
        const size_t        JSON_DOC_SIZE = 1024U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg          = jsonDoc.to<JsonObject>();
        JsonVariantConst    jsonPlayerKey    = value["playerKey"];
        JsonVariantConst    jsonMqttInstance = value["mqttInstance"];

        /* The received configuration may not contain all single key/value pair.
         * Therefore read first the complete internal configuration and
         * overwrite them with the received ones.
         */
        getConfiguration(jsonCfg);

        /* Note:
         * Check only for the key/value pair availability.
         * The type check will follow in the setConfiguration().
         */

        if (false == jsonPlayerKey.isNull())
        {
            jsonCfg["playerKey"] = jsonPlayerKey.as<String>();
            isSuccessful         = true;
        }

        if (false == jsonMqttInstance.isNull())
        {
            jsonCfg["mqttInstance"] = jsonMqttInstance.as<uint8_t>();
            isSuccessful            = true;
        }

        if (true == isSuccessful)
        {
            JsonObjectConst jsonCfgConst = jsonCfg;

            isSuccessful                 = setConfiguration(jsonCfgConst);

            if (true == isSuccessful)
            {
                requestStoreToPersistentMemory();
            }
        }
    }
    /* Play control topic? */
    else if (true == topic.equals(TOPIC_PLAY_CONTROL))
    {
        const JsonVariantConst jsonAction = value["action"];

        if (true == jsonAction.is<String>())
        {
            const String action = jsonAction.as<String>();

            if (true == action.equals("next"))
            {
                isSuccessful = cmdNextArtwork();
            }
            else if (true == action.equals("previous"))
            {
                isSuccessful = cmdPrevArtwork();
            }
            else if (true == action.equals("playChannel"))
            {
                JsonVariantConst jsonChannelName = value["channelName"];
                JsonVariantConst jsonUserSqid    = value["userSqid"];
                JsonVariantConst jsonHashtag     = value["hashtag"];

                if (true == jsonChannelName.is<String>())
                {
                    const char* channelName = jsonChannelName.as<const char*>();
                    const char* userSqid    = nullptr;
                    const char* hashtag     = nullptr;

                    if (false == jsonUserSqid.isNull())
                    {
                        userSqid = jsonUserSqid.as<const char*>();
                    }

                    if (false == jsonHashtag.isNull())
                    {
                        hashtag = jsonHashtag.as<const char*>();
                    }

                    isSuccessful = cmdPlayChannel(channelName, userSqid, hashtag);
                }
                else
                {
                    LOG_WARNING("JSON channelName not found or invalid type.");
                }
            }
            else if (true == action.equals("pause"))
            {
                isSuccessful = cmdPause();
            }
            else if (true == action.equals("continue"))
            {
                isSuccessful = cmdContinue();
            }
            else
            {
                LOG_WARNING("Unknown action '%s'.", action.c_str());
            }
        }
        else
        {
            LOG_WARNING("JSON action not found or invalid type.");
        }
    }
    /* Provision topic? */
    else if (true == topic.equals(TOPIC_PROVISION))
    {
        isSuccessful = cmdProvisionPlayer(value);
    }
    /* Unknown topic. */
    else
    {
        /* Nothing to do. */
        ;
    }

    return isSuccessful;
}

bool MakapixPlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive> guard(m_mutex);
    bool                       hasTopicChanged = m_hasTopicChanged;

    /* Only a single topic, therefore its not necessary to check. */
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void MakapixPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.init(width, height);

    PluginWithConfig::start(width, height);

    if (false == m_fileCache.init(ARTWORK_CACHE_PATH))
    {
        LOG_ERROR("Failed to initialize artwork file cache handler.");
    }

    /* Configure with initial loaded configuration. */
    m_commandHandler.configure(m_playerKey, m_mqttInstance);
    m_channel.configure(m_playerKey, m_mqttInstance);
    m_viewUpdate.configure(m_playerKey, m_mqttInstance);
}

void MakapixPlugin::stop()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_dwellTimer.stop();

    m_commandHandler.unsubscribe();

    /* Abort a pending artwork download. */
    m_artworkDownloader.abort();

    PluginWithConfig::stop();
}

void MakapixPlugin::process(bool isConnected)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    PluginWithConfig::process(isConnected);

    m_commandHandler.process();
    processArtworkDownload();
    m_channel.process();
    processDisplayMode();
    m_view.process();
    processProvision();
    m_viewUpdate.process();
}

void MakapixPlugin::active(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    UTIL_NOT_USED(gfx);

    if (false == m_isPaused)
    {
        m_dwellTimer.resume();
    }

    m_isActive = true;
}

void MakapixPlugin::inactive()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_dwellTimer.stop();

    m_isActive = false;
}

void MakapixPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.update(gfx);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void MakapixPlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["playerKey"]    = m_playerKey;
    jsonCfg["mqttInstance"] = m_mqttInstance;
}

bool MakapixPlugin::setConfiguration(const JsonObjectConst& jsonCfg)
{
    bool             status           = false;
    JsonVariantConst jsonPlayerKey    = jsonCfg["playerKey"];
    JsonVariantConst jsonMqttInstance = jsonCfg["mqttInstance"];

    if (false == jsonPlayerKey.is<String>())
    {
        LOG_WARNING("JSON playerKey not found or invalid type.");
    }
    else if (false == jsonMqttInstance.is<uint8_t>())
    {
        LOG_WARNING("JSON mqttInstance not found or invalid type.");
    }
    else if (MqttService::MAX_MQTT_COUNT <= jsonMqttInstance.as<uint8_t>())
    {
        LOG_WARNING("JSON mqttInstance value out of range.");
    }
    else
    {
        MutexGuard<MutexRecursive> guard(m_mutex);
        const char*                playerKey    = jsonPlayerKey.as<const char*>();
        uint8_t                    mqttInstance = jsonMqttInstance.as<uint8_t>();

        if ((m_playerKey != playerKey) ||
            (m_mqttInstance != mqttInstance))
        {
            m_commandHandler.unsubscribe();
            m_channel.abort();

            m_playerKey       = playerKey;
            m_mqttInstance    = mqttInstance;
            m_hasTopicChanged = true;

            m_commandHandler.configure(m_playerKey, m_mqttInstance);
            m_channel.configure(m_playerKey, m_mqttInstance);
            m_viewUpdate.configure(m_playerKey, m_mqttInstance);

            m_commandHandler.subscribe();
        }

        status = true;
    }

    return status;
}

void MakapixPlugin::processArtworkDownload()
{
    /* Any pending artwork download? */
    if (true == m_isDownloadingArtwork)
    {
        bool isSuccessful = false;

        /* Is download finished? */
        if (true == m_artworkDownloader.getStatus(isSuccessful))
        {
            if (false == isSuccessful)
            {
                LOG_WARNING("Artwork download failed.");

                if (false == m_isPaused)
                {
                    m_dwellTimer.resume();
                }
            }
            else
            {
                String dstFilePath = m_artworkDownloader.getDestinationFilePath();

                if (false == m_view.loadIcon(dstFilePath, FILE_CACHE_FS))
                {
                    LOG_WARNING("Loading artwork from cache failed.");
                }

                m_dwellTimer.start(m_playlist.getDwellTime());

                if (true == m_isPaused)
                {
                    m_dwellTimer.stop();
                }

                m_currentFilePath = dstFilePath;
                m_fileCache.addFile(dstFilePath);

                m_viewUpdate.setPostId(m_playlist.getPostId());
            }

            m_isDownloadingArtwork = false;
        }
    }
}

void MakapixPlugin::processDisplayMode()
{
    /* No artwork in the playlist? */
    if (0 == m_playlist.length())
    {
        ;
    }
    /* Artwork download is pending? */
    else if (true == m_isDownloadingArtwork)
    {
        ;
    }
    else if (DISPLAY_MODE_SINGLE_ARTWORK == m_displayMode)
    {
        /* If no icon is shown yet, show the selected artwork. */
        if (false == m_view.isIconShown())
        {
            int32_t selectedIdx = m_playlist.selected();

            /* Anything selected in the playlist? */
            if (0 <= selectedIdx)
            {
                if (false == showArtwork())
                {
                    LOG_WARNING("Showing artwork failed.");
                }
                else
                {
                    m_currentPlaylistIdx = selectedIdx;
                }
            }
        }
    }
    else if (DISPLAY_MODE_PLAY_CHANNEL == m_displayMode)
    {
        /* Pending channel request? */
        if (true == m_channel.isPending())
        {
            ;
        }
        /* If no icon is shown yet, show the selected artwork. */
        else if (false == m_view.isIconShown())
        {
            int32_t selectedIdx = m_playlist.selected();

            /* Anything selected in the playlist? */
            if (0 <= selectedIdx)
            {
                if (false == showArtwork())
                {
                    LOG_WARNING("Showing artwork failed.");
                }
                else
                {
                    m_currentPlaylistIdx = selectedIdx;
                }
            }
        }
        /* Pause? */
        else if (false == m_dwellTimer.isTimerRunning())
        {
            /* Nothing to do. */
            ;
        }
        /* Dwell timer expired? */
        else if (true == m_dwellTimer.isTimeout())
        {
            int32_t playlistIdx = m_playlist.selected();

            m_dwellTimer.stop();

            /* Selected artwork in the playlist changed? */
            if (m_currentPlaylistIdx != playlistIdx)
            {
                if (false == showArtwork())
                {
                    LOG_WARNING("Showing artwork failed.");
                }
                else
                {
                    m_currentPlaylistIdx = playlistIdx;
                }
            }
            /* Otherwise select just next artwork in the playlist. */
            else
            {
                (void)m_channel.playNext();
            }

            if (false == m_dwellTimer.isTimerRunning())
            {
                /* In case showing the artwork failed, restart the dwell timer
                 * to try again after a short period.
                 */
                m_dwellTimer.start(SIMPLE_TIMER_SECONDS(1U));
            }
        }
        else
        {
            /* Nothing to do. */
            ;
        }
    }
    else
    {
        /* Invalid display mode. */
        ;
    }
}

bool MakapixPlugin::showArtwork()
{
    bool isSuccessful = false;

    if (0U < m_playlist.length())
    {
        String   artUrl     = m_playlist.getUrl();
        String   storageKey = m_playlist.getStorageKey();
        uint32_t dwellTime  = m_playlist.getDwellTime();
        uint32_t postId     = m_playlist.getPostId();

        LOG_INFO("Show artwork %s.", storageKey.c_str());

        if ((false == artUrl.isEmpty()) &&
            (false == storageKey.isEmpty()))
        {
            String dstFilePath = getCacheFilePath(artUrl.c_str());

            /* Artwork already shown?
             * Use the destination file path as unique identifier.
             */
            if (m_currentFilePath == dstFilePath)
            {
                LOG_INFO("Artwork is already shown. Ignoring this request.");

                m_dwellTimer.restart();

                if (true == m_isPaused)
                {
                    m_dwellTimer.stop();
                }

                isSuccessful = true;
            }
            /* Artwork download is required, but if there is a pending artwork download,
             * this new request will be ignored.
             */
            else if (true == m_isDownloadingArtwork)
            {
                LOG_WARNING("Another artwork is currently being downloaded. Ignoring this request.");
            }
            else
            {
                String cacheFileId    = getCacheFileId(storageKey);
                String filePath       = m_fileCache.getFilePathById(cacheFileId);
                bool   downloadNeeded = true;

                /* Artwork is already cached? */
                if (false == filePath.isEmpty())
                {
                    LOG_INFO("Artwork is already cached. Load it from cache.");

                    if (false == m_view.loadIcon(filePath, FILE_CACHE_FS))
                    {
                        LOG_WARNING("Loading artwork from cache failed.");
                        m_fileCache.remove(storageKey);
                    }
                    else
                    {
                        m_dwellTimer.start(dwellTime);

                        if (true == m_isPaused)
                        {
                            m_dwellTimer.stop();
                        }

                        m_viewUpdate.setPostId(postId);

                        m_currentFilePath = filePath;
                        downloadNeeded    = false;
                        isSuccessful      = true;
                    }
                }

                if (true == downloadNeeded)
                {
                    /* Download artwork from URL. */
                    if (true == m_artworkDownloader.download(artUrl, dstFilePath))
                    {
                        m_isDownloadingArtwork = true;
                        isSuccessful           = true;
                    }
                }
            }
        }
    }

    return isSuccessful;
}

bool MakapixPlugin::nextArtwork()
{
    bool isSuccessful = false;

    LOG_INFO("Swap to next artwork.");

    if (true == m_playlist.next())
    {
        if (false == showArtwork())
        {
            ;
        }
        else
        {
            m_currentPlaylistIdx = m_playlist.selected();
            isSuccessful         = true;
        }
    }

    return isSuccessful;
}

bool MakapixPlugin::prevArtwork()
{
    bool isSuccessful = false;

    LOG_INFO("Swap to previous artwork.");

    if (true == m_playlist.prev())
    {
        if (false == showArtwork())
        {
            ;
        }
        else
        {
            m_currentPlaylistIdx = m_playlist.selected();
            isSuccessful         = true;
        }
    }

    return isSuccessful;
}

String MakapixPlugin::getCacheFilePath(const char* artUrl) const
{
    String filePath;

    /* <HTTP-METHOD>://<HOSTNAME><PATH><STORAGE-KEY>.<FILE-EXTENSISON> */

    if (nullptr != artUrl)
    {
        const char* filename = strrchr(artUrl, '/'); /* Filename is at the end. */

        /* No '/' found? */
        if (nullptr == filename)
        {
            LOG_ERROR("Invalid art URL.");
        }
        else
        {
            const char* storageKey    = strrchr(artUrl, '/');
            const char* fileExtension = strrchr(artUrl, '.');
            String      cacheFileId;

            /* No '/' found? */
            if (nullptr == fileExtension)
            {
                LOG_ERROR("Invalid art URL.");
            }
            /* No '.' found? */
            else if (nullptr == fileExtension)
            {
                LOG_ERROR("Invalid art URL.");
            }
            else
            {
                const size_t STORAGE_KEY_LEN = fileExtension - storageKey - 1U;

                ++storageKey;    /* Skip '/' character. */
                ++fileExtension; /* Skip '.' character. */

                cacheFileId  = getCacheFileId(String(storageKey, STORAGE_KEY_LEN).c_str());

                filePath     = ARTWORK_CACHE_PATH;
                filePath    += cacheFileId;
                filePath    += ".";
                filePath    += fileExtension;
            }
        }
    }

    return filePath;
}

String MakapixPlugin::getCacheFileId(const String& storageKey) const
{
    String  fileId;
    int32_t dashIdx = storageKey.indexOf('-');

    /* Storage key: <FIRST-BLOCK>-<SECOND-BLOCK>-<THIRD-BLOCK>-<FOURTH-BLOCK>-<FIFTH-BLOCK>
     * Example    : f373e5e7-9ed6-46a9-b616-313f2670602a -> 9ed6-46a9-b616-313f2670602a
     */

    if (0 < dashIdx)
    {
        /* Because of filename length limitations, truncate storage key by first block. */
        fileId = storageKey.substring(dashIdx + 1);
    }

    return fileId;
}

bool MakapixPlugin::cmdNextArtwork()
{
    bool                       isSuccessful = false;
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (false == nextArtwork())
    {
        m_view.showActionIconFail();
    }
    else
    {
        m_view.showActionIconNext();
        isSuccessful = true;
    }

    return isSuccessful;
}

bool MakapixPlugin::cmdPrevArtwork()
{
    bool                       isSuccessful = false;
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (false == prevArtwork())
    {
        m_view.showActionIconFail();
    }
    else
    {
        m_view.showActionIconPrev();
        isSuccessful = true;
    }

    return isSuccessful;
}

bool MakapixPlugin::cmdPlayChannel(const char* channelName, const char* userSqid, const char* hashtag)
{
    bool                       isSuccessful = false;
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (false == m_channel.play(channelName, userSqid, hashtag))
    {
        m_view.showActionIconFail();
    }
    else
    {
        m_view.showActionIconPlay();
        m_displayMode = DISPLAY_MODE_PLAY_CHANNEL;
        isSuccessful  = true;
    }

    return isSuccessful;
}

void MakapixPlugin::cmdShowArtwork()
{
    if (0U < m_playlist.length())
    {
        if (false == showArtwork())
        {
            m_view.showActionIconFail();
        }
        else
        {
            m_currentPlaylistIdx = m_playlist.selected();
            m_view.showActionIconPlay();
        }
    }

    m_displayMode = DISPLAY_MODE_SINGLE_ARTWORK;
}

bool MakapixPlugin::cmdPause()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    LOG_INFO("Pause playback.");

    /* Player is playing? */
    if (false == m_isPaused)
    {
        /* Pause it. */
        m_isPaused = true;
        m_dwellTimer.stop();

        /* If plugin is active, show pause icon. */
        if (true == m_isActive)
        {
            m_view.showActionIconPause();
        }
    }

    return true;
}

bool MakapixPlugin::cmdContinue()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    LOG_INFO("Continue playback.");

    /* Player is paused? */
    if (true == m_isPaused)
    {
        /* Resume. */
        m_isPaused = false;

        /* If plugin is active, resume dwell timer and show play icon. */
        if (true == m_isActive)
        {
            m_dwellTimer.resume();
            m_view.showActionIconPlay();
        }
    }

    return true;
}

bool MakapixPlugin::cmdGetStatus(JsonObject& jsonStatus) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonStatus["channel"]  = m_channel.getChannelName();
    jsonStatus["userSqid"] = m_channel.getUserSqid();
    jsonStatus["hashtag"]  = m_channel.getHashtag();

    /* Pause? */
    if (true == m_isPaused)
    {
        jsonStatus["status"] = "paused";
    }
    else
    {
        jsonStatus["status"] = "playing";
    }

    return true;
}

bool MakapixPlugin::cmdGetProvisionStatus(JsonObject& jsonStatus) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    /* Is a player provision pending? */
    if (INVALID_HTTP_JOB_ID != m_provisionHttpJobId)
    {
        jsonStatus["status"] = "pending";
    }
    else if (true == m_registrationCode.isEmpty())
    {
        jsonStatus["status"] = "idle";
    }
    else
    {
        jsonStatus["status"]                    = "completed";
        jsonStatus["playerKey"]                 = m_playerKey;
        jsonStatus["registrationCode"]          = m_registrationCode;
        jsonStatus["registrationCodeExpiresAt"] = m_registrationCodeExpiresAt;
        jsonStatus["mqttBroker"]                = JsonObject();
        jsonStatus["mqttBroker"]["host"]        = m_mqttBrokerHost;
        jsonStatus["mqttBroker"]["port"]        = m_mqttBrokerPort;
    }

    return true;
}

bool MakapixPlugin::cmdProvisionPlayer(const JsonObjectConst& jsonCmd)
{
    bool                       isSuccessful = false;
    HttpService&               httpService  = HttpService::getInstance();
    MutexGuard<MutexRecursive> guard(m_mutex);

    UTIL_NOT_USED(jsonCmd);

    /* Provision only allowed if no player key is set. */
    if (false == m_playerKey.isEmpty())
    {
        LOG_WARNING("Player is already provisioned.");
    }
    /* Is a provision already pending? */
    else if (INVALID_HTTP_JOB_ID != m_provisionHttpJobId)
    {
        LOG_WARNING("Player provision is already pending.");
    }
    /* Start provision. */
    else
    {
        m_provisionPayload    = "{\"device_model\":\"Pixelix\",\"firmware_version\":\"";
        m_provisionPayload   += Version::getSoftwareVersion();
        m_provisionPayload   += "\"}";

        m_provisionHttpJobId  = httpService.post(MAKAPIX_PROVISION_URL,
            reinterpret_cast<const uint8_t*>(m_provisionPayload.c_str()),
            m_provisionPayload.length());

        if (INVALID_HTTP_JOB_ID != m_provisionHttpJobId)
        {
            LOG_DEBUG("Player provision http job id: %u", m_provisionHttpJobId);

            isSuccessful = true;
        }
    }

    return isSuccessful;
}

void MakapixPlugin::processProvision()
{
    /* Is a player provision pending? */
    if (INVALID_HTTP_JOB_ID != m_provisionHttpJobId)
    {
        HttpService& httpService = HttpService::getInstance();
        HttpRsp      httpResponse;

        if (true == httpService.getResponse(m_provisionHttpJobId, httpResponse))
        {
            if ((HTTP_CODE_OK != httpResponse.statusCode) &&
                (HTTP_CODE_CREATED != httpResponse.statusCode))
            {
                LOG_WARNING("Player provision HTTP request failed with status code %d.", httpResponse.statusCode);
            }
            else
            {
                DynamicJsonDocument  jsonDoc(1024U);
                DeserializationError error = deserializeJson(jsonDoc, httpResponse.payload);

                if (DeserializationError::Ok != error)
                {
                    LOG_WARNING("Player provision response JSON deserialization failed: %s", error.c_str());
                }
                else
                {
                    JsonVariantConst jsonPlayerKey                 = jsonDoc["player_key"];
                    JsonVariantConst jsonRegistrationCode          = jsonDoc["registration_code"];
                    JsonVariantConst jsonRegistrationCodeExpiresAt = jsonDoc["registration_code_expires_at"];
                    JsonVariantConst jsonMqttBrokerHost            = jsonDoc["mqtt_broker"]["host"];
                    JsonVariantConst jsonMqttBrokerPort            = jsonDoc["mqtt_broker"]["port"];

                    if (false == jsonPlayerKey.is<String>())
                    {
                        LOG_WARNING("Player provision response JSON player_key not found or invalid type.");
                    }
                    else if (false == jsonRegistrationCode.is<String>())
                    {
                        LOG_WARNING("Player provision response JSON registration_code not found or invalid type.");
                    }
                    else if (false == jsonRegistrationCodeExpiresAt.is<String>())
                    {
                        LOG_WARNING("Player provision response JSON registration_code_expires_at not found or invalid type.");
                    }
                    else if (false == jsonMqttBrokerHost.is<String>())
                    {
                        LOG_WARNING("Player provision response JSON mqtt_broker.host not found or invalid type.");
                    }
                    else if (false == jsonMqttBrokerPort.is<uint16_t>())
                    {
                        LOG_WARNING("Player provision response JSON mqtt_broker.port not found or invalid type.");
                    }
                    else
                    {
                        m_playerKey                 = jsonPlayerKey.as<String>();
                        m_registrationCode          = jsonRegistrationCode.as<String>();
                        m_registrationCodeExpiresAt = jsonRegistrationCodeExpiresAt.as<String>();
                        m_mqttBrokerHost            = jsonMqttBrokerHost.as<String>();
                        m_mqttBrokerPort            = jsonMqttBrokerPort.as<uint16_t>();

                        m_hasTopicChanged           = true;

                        LOG_INFO("Player provision successful.");
                    }
                }
            }

            m_provisionHttpJobId = INVALID_HTTP_JOB_ID;
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

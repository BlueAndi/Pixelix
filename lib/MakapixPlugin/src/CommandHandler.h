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
 * @file   CommandHandler.h
 * @brief  Makapix command handler
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <WString.h>
#include <SimpleTimer.hpp>
#include <MqttTypes.h>
#include "MakapixTypes.h"
#include "Playlist.h"
#include "Channel.h"
#include "ViewUpdateTimer.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Makapix command handler class.
 *
 * The command handler is responsible to handle incoming commands via MQTT.
 * It sends periodic status updates via MQTT.
 * It sends view updates via MQTT.
 *
 * It adds artworks to the playlist via commands and triggers playlist actions
 * like next/previous artwork or play channel.
 */
class CommandHandler
{
public:

    /**
     * Constructs the command handler.
     *
     * @param[in] playlist  Playlist of artworks.
     * @param[in] channel   Channel to play artworks from.
     */
    CommandHandler(Playlist& playlist, const Channel& channel) :
        m_playlist(playlist),
        m_channel(channel),
        m_nextArtworkCallback(nullptr),
        m_prevArtworkCallback(nullptr),
        m_playChannelCallback(nullptr),
        m_showArtworkCallback(nullptr),
        m_playerKey(),
        m_mqttInstance(0U),
        m_statusTimer(),
        m_mqttConnectionState(MqttTypes::STATE_DISCONNECTED),
        m_artworkPostId(0U),
        m_viewUpdateTimer()
    {
    }

    /**
     * Destroys the command handler.
     */
    ~CommandHandler()
    {
    }

    /**
     * Initialize the command handler.
     *
     * @param[in] nextArtworkCallback   Next artwork callback.
     * @param[in] prevArtworkCallback   Previous artwork callback.
     * @param[in] playChannelCallback   Play channel callback.
     * @param[in] showArtworkCallback    Show artwork callback.
     */
    void init(MakapixNextArtworkCallback nextArtworkCallback, MakapixPrevArtworkCallback prevArtworkCallback, MakapixPlayChannelCallback playChannelCallback, MakapixShowArtworkCallback showArtworkCallback);

    /**
     * Configure the command handler.
     * It will unsubscribe any previous subscription.
     *
     * @param[in] playerKey      Player key
     * @param[in] mqttInstance   MQTT instance index
     */
    void configure(const String& playerKey, uint8_t mqttInstance);

    /**
     * Process command handler.
     */
    void process();

    /**
     * Subscribe MQTT topic to be informed about value changes.
     * Additional it will start the periodic status update timer.
     *
     * Note, if no player key is set or MQTT is not connected, it will do nothing.
     */
    void subscribe();

    /**
     * Unsubscribe MQTT topic to stop on change notifications.
     * Additionally it will stop the periodic status update timer.
     *
     * Note, if no player key is set or MQTT is not connected, it will only
     * stop the peridodic status update timer.
     */
    void unsubscribe();

    /**
     * Set the artwork post ID, which is currently shown on the display.
     *
     * @param[in] postId   Artwork post ID
     */
    void setPostId(uint32_t postId);

    /**
     * Notify status update via MQTT.
     * This includes player key, current post ID, firmware version and online status.
     *
     * @param[in] isOnline  Online status
     * 
     * @return If status update notification was sent, it will return true otherwise false.
     */
    bool notifyStatusUpdate(bool isOnline);

private:

    /**
     * Status update period in milliseconds.
     */
    static const uint32_t STATUS_UPDATE_PERIOD       = SIMPLE_TIMER_MINUTES(5U);

    /**
     * Default dwell time in milliseconds.
     */
    static const uint32_t      DEFAULT_DWELL_TIME_MS = SIMPLE_TIMER_SECONDS(30U);

    Playlist&                  m_playlist;            /**< Local playlist of artworks. */
    const Channel&             m_channel;             /**< Channel to play artworks from. */
    MakapixNextArtworkCallback m_nextArtworkCallback; /**< Next artwork callback. */
    MakapixPrevArtworkCallback m_prevArtworkCallback; /**< Previous artwork callback. */
    MakapixPlayChannelCallback m_playChannelCallback; /**< Play channel callback. */
    MakapixShowArtworkCallback m_showArtworkCallback; /**< Show artwork callback. */
    String                     m_playerKey;           /**< Player key. */
    uint8_t                    m_mqttInstance;        /**< MQTT instance index. */
    SimpleTimer                m_statusTimer;         /**< Timer for periodic status updates via MQTT. */
    MqttTypes::State           m_mqttConnectionState; /**< MQTT connection state. */
    uint32_t                   m_artworkPostId;       /**< Artwork post ID, which is currently shown on the display. */
    ViewUpdateTimer            m_viewUpdateTimer;     /**< Timer for view updates via MQTT. */

    CommandHandler()                                            = delete;
    CommandHandler(const CommandHandler& cmdHandler)            = delete;
    CommandHandler& operator=(const CommandHandler& cmdHandler) = delete;

    /**
     * The MQTT callback is registered by subscription and will be called on change by
     * the MQTT service.
     *
     * @param[in] topic     Topic
     * @param[in] payload   Topic payload
     * @param[in] size      Topic payload size in byte
     */
    void mqttTopicCallback(const String& topic, const uint8_t* payload, size_t size);

    /**
     * On received command.
     *
     * @param[in] jsonDoc   The command as JSON document.
     */
    void onCommand(const JsonDocument& jsonDoc);

    /**
     * Swap to next artwork.
     */
    void swapNext();

    /**
     * Swap to previous artwork.
     */
    void swapBack();

    /**
     * Show artwork immediately.
     *
     * @param[in] jsonPayload   The artwork information as JSON object.
     */
    void showArtwork(const JsonObjectConst& jsonPayload);

    /**
     * Play channel.
     *
     * @param[in] jsonPayload   The channel information as JSON object.
     */
    void playChannel(const JsonObjectConst& jsonPayload);

    /**
     * Get width and height from canvas string.
     *
     * @param[in]  canvas   Canvas as string, e.g. "32x8"
     * @param[out] width    Width in pixel.
     * @param[out] height   Height in pixel.
     */
    void getWidthHeight(const char* canvas, uint16_t& width, uint16_t& height) const;

    /**
     * Notify view update via MQTT.
     * 
     * @return If view update notification was sent, it will return true otherwise false.
     */
    bool notifyViewUpdate();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* COMMAND_HANDLER_H */

/** @} */

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
 * @file   Channel.h
 * @brief  Makapix channel
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef CHANNEL_H
#define CHANNEL_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <WString.h>
#include "MakapixTypes.h"
#include "RequestHandler.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Makapix channel class.
 *
 * A channel is responsible to fetch artworks from a specific channel id
 * and fill the playlist with it.
 */
class Channel
{
public:

    /** This type defines the different channel ids. */
    typedef enum
    {
        CHANNEL_ID_ALL = 0,  /**< Recent posts from all users (respects visibility settings) */
        CHANNEL_ID_PROMOTED, /**< Only promoted posts (editor picks, frontpage, etc.) */
        CHANNEL_ID_USER,     /**< Posts from the player owner's account */
        CHANNEL_ID_BY_USER   /**< Posts from an arbitrary user specified by user_handle */

    } ChannelId;

    /** This type defines the possible sorting orders. */
    typedef enum
    {
        SORT_ORDER_SERVER = 0, /**< Original insertion order (by post ID) */
        SORT_ORDER_CREATED_AT, /**< Chronological order by creation timestamp */
        SORT_ORDER_RANDOM      /**< Random order with optional seed for reproducibility */

    } SortOrder;

    /**
     * Constructs the request handler.
     *
     * @param[in] playlist  Playlist of artworks.
     */
    Channel(MakapixPlaylist& playlist);

    /**
     * Destroys the request handler.
     */
    ~Channel();

    /**
     * Initialize the channel.
     *
     * @param[in] nextArtworkCallback   Next artwork callback.
     */
    void init(MakapixNextArtworkCallback nextArtworkCallback);

    /**
     * Configure the channel.
     *
     * @param[in] playerKey     Player key.
     * @param[in] mqttInstance  MQTT instance index.
     */
    void configure(const String& playerKey, uint8_t mqttInstance);

    /**
     * Process the channel.
     */
    void process();

    /**
     * Play the channel.
     *
     * @param[in] channelName   The channel name.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool play(const char* channelName);

    /**
     * Play the channel.
     *
     * @param[in] channelId   The channel id.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool play(ChannelId channelId);

    /**
     * Play next artwork.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool playNext();

    /**
     * Get current channel name.
     *
     * @return Current channel name.
     */
    const char* getChannelName() const;

    /**
     * Check if play request is pending.
     *
     * @return If a play request is pending, it will return true otherwise false.
     */
    bool isPending() const;

    /**
     * Abort pending request.
     */
    void abort();

private:

    MakapixPlaylist&           m_playlist;            /**< Playlist of artworks. */
    MakapixNextArtworkCallback m_nextArtworkCallback; /**< Next artwork callback. */
    RequestHandler             m_requestHandler;      /**< Request handler. */
    ChannelId                  m_channelId;           /**< Artwork channel id. */
    SortOrder                  m_sortOrder;           /**< Artwork channel sort order. */
    String                     m_userHandle;          /**< User handle for user specific requests. */
    uint32_t                   m_page;                /**< Current page for pagination. */
    uint32_t                   m_idx;                 /**< Current index within the page. */
    uint32_t                   m_total;               /**< Total number of artworks available. */
    bool                       m_hasMore;             /**< Indicates if there are more pages available. */
    bool                       m_showArtwork;         /**< Indicates that the next artwork should be shown. */

    Channel()                          = delete;
    Channel(const Channel&)            = delete;
    Channel& operator=(const Channel&) = delete;

    /**
     * On received response.
     *
     * @param[in] jsonDoc   The response as JSON document.
     */
    void onResponse(const JsonDocument& jsonDoc);

    /**
     * On received artwork information.
     *
     * @param[in] jsonObj   The artwork information as JSON object.
     */
    void onResponseArtwork(const JsonObjectConst& jsonObj);

    /**
     * On received playlist information.
     *
     * @param[in] jsonObj   The playlist information as JSON object.
     */
    void onResponsePlaylist(const JsonObjectConst& jsonObj);

    /**
     * Convert channel id to string.
     *
     * @param[in] channel   The channel id.
     *
     * @return Channel as string.
     */
    const char* channelIdToStr(ChannelId channel) const;

    /**
     * Convert channel name to id.
     *
     * @param[in] channelName   The channel name.
     *
     * @return Channel id.
     */
    ChannelId strToChannelId(const char* channelName) const;

    /**
     * Convert sort order to string.
     *
     * @param[in] sortOrder   The sort order.
     *
     * @return Sort order as string.
     */
    const char* sortOrderToStr(SortOrder sortOrder) const;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* CHANNEL_H */

/** @} */

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
 * @file   Channel.cpp
 * @brief  Makapix channel
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Channel.h"

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

/** Channel as string. */
static const char* gChannelAsStr[] = {
    "all",
    "promoted",
    "user",
    "by_user"
};

/** Sort order as string. */
static const char* gSortOrderAsStr[] = {
    "server_order",
    "created_at",
    "random"
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

Channel::Channel(Playlist& playlist) :
    m_playlist(playlist),
    m_nextArtworkCallback(nullptr),
    m_requestHandler(),
    m_channelId(CHANNEL_ID_ALL),
    m_sortOrder(SORT_ORDER_SERVER),
    m_userHandle(),
    m_page(0U),
    m_idx(0U),
    m_total(0U),
    m_hasMore(false),
    m_showArtwork(false)
{
    MakapixOnResponseCallback onResponseCallback =
        [this](const JsonDocument& jsonDoc) {
            this->onResponse(jsonDoc);
        };

    m_requestHandler.init(onResponseCallback);
}

Channel::~Channel()
{
}

void Channel::init(MakapixNextArtworkCallback nextArtworkCallback)
{
    m_nextArtworkCallback = nextArtworkCallback;
}

void Channel::configure(const String& playerKey, uint8_t mqttInstance)
{
    m_requestHandler.configure(playerKey, mqttInstance);
}

void Channel::process()
{
    m_requestHandler.process();
}

bool Channel::play(const char* channelName)
{
    bool isSuccessful = false;

    LOG_INFO("Play channel \"%s\"", channelName);

    /* Reset channel parameters. */
    m_channelId = strToChannelId(channelName);
    m_page      = 0U;
    m_idx       = 0U;
    m_hasMore   = true;

    /* Request artwork for this channel. */
    if (false == m_requestHandler.request(channelName, nullptr, sortOrderToStr(m_sortOrder), m_page, Constant::CHANNEL_PAGE_ITEM_LIMIT))
    {
        LOG_WARNING("Failed to request new artwork for channel \"%s\".", channelName);
    }
    else
    {
        isSuccessful = true;
    }

    return isSuccessful;
}

bool Channel::play(ChannelId channelId)
{
    return play(channelIdToStr(channelId));
}

bool Channel::playNext()
{
    bool     isSuccessful = false;
    uint32_t idx          = m_idx;
    uint32_t page         = m_page;

    LOG_INFO("Play next artwork in channel \"%s\"", channelIdToStr(m_channelId));

    ++idx;

    /* Next artwork to request? */
    if (m_total <= idx)
    {
        idx = 0U;

        if (false == m_hasMore)
        {
            page = 0U;
        }
        else
        {
            ++page;
        }

        /* Request next artwork for this channel. */
        if (false == m_requestHandler.request(channelIdToStr(m_channelId), nullptr, sortOrderToStr(m_sortOrder), page, Constant::CHANNEL_PAGE_ITEM_LIMIT))
        {
            LOG_WARNING("Failed to request next artwork for channel \"%s\".", channelIdToStr(m_channelId));
        }
        else
        {
            isSuccessful = true;
        }
    }
    /* Its already in the playlist. */
    else if (nullptr == m_nextArtworkCallback)
    {
        LOG_WARNING("Next artwork callback not set.");
    }
    else
    {
        m_nextArtworkCallback();
        isSuccessful = true;
    }

    if (true == isSuccessful)
    {
        m_page = page;
        m_idx  = idx;
    }

    return isSuccessful;
}

const char* Channel::getChannelName() const
{
    return channelIdToStr(m_channelId);
}

bool Channel::isPending() const
{
    return m_requestHandler.isPending();
}

void Channel::abort()
{
    LOG_INFO("Abort channel \"%s\"", channelIdToStr(m_channelId));

    m_requestHandler.abort();
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void Channel::onResponse(const JsonDocument& jsonDoc)
{
    JsonVariantConst jsonRequestId = jsonDoc["request_id"];
    JsonVariantConst jsonSuccess   = jsonDoc["success"];
    String           requestIdStr  = jsonRequestId.as<String>();
    int32_t          requestId     = requestIdStr.toInt();
    bool             isSuccessful  = jsonSuccess.as<bool>();

    LOG_DEBUG("Response for request ID %d received.", requestId);

    if (false == isSuccessful)
    {
        JsonVariantConst jsonErrorCode = jsonDoc["error_code"];
        JsonVariantConst jsonError     = jsonDoc["error"];

        LOG_WARNING("Request was not successful, error: %d", jsonErrorCode.as<int32_t>());

        if (true == jsonError.is<String>())
        {
            LOG_WARNING("Error description: %s", jsonError.as<const char*>());
        }
    }
    else
    {
        JsonVariantConst jsonHasMore = jsonDoc["has_more"];
        JsonVariantConst jsonPosts   = jsonDoc["posts"];

        if (true == jsonHasMore.is<bool>())
        {
            m_hasMore = jsonHasMore.as<bool>();
        }

        if (true == jsonPosts.is<JsonArrayConst>())
        {
            JsonArrayConst jsonPostsArray = jsonPosts.as<JsonArrayConst>();

            m_total                       = jsonPostsArray.size();

            /* The first artwork that is added to the playlist shall be shown
             * immediately.
             */
            m_showArtwork                 = true;

            for (JsonVariantConst postVariant : jsonPostsArray)
            {
                JsonVariantConst jsonKind = postVariant["kind"];

                if (true == jsonKind.is<const char*>())
                {
                    const char* kind = jsonKind.as<const char*>();

                    if (0 == strcmp(kind, "artwork"))
                    {
                        onResponseArtwork(postVariant.as<JsonObjectConst>());
                    }
                    else if (0 == strcmp(kind, "playlist"))
                    {
                        onResponsePlaylist(postVariant.as<JsonObjectConst>());
                    }
                    else
                    {
                        LOG_WARNING("Unknown post kind in response: %s", kind);
                    }
                }
            }
        }
    }
}

void Channel::onResponseArtwork(const JsonObjectConst& jsonObj)
{
    JsonVariantConst jsonPostId       = jsonObj["post_id"];
    JsonVariantConst jsonStorageKey   = jsonObj["storage_key"];
    JsonVariantConst jsonStorageShard = jsonObj["storage_shard"];
    JsonVariantConst jsonNativeFormat = jsonObj["native_format"];
    JsonVariantConst jsonWidth        = jsonObj["width"];
    JsonVariantConst jsonHeight       = jsonObj["height"];
    JsonVariantConst jsonDwellTimeMs  = jsonObj["dwell_time_ms"];

    if (false == jsonPostId.is<uint32_t>())
    {
        LOG_WARNING("Response artwork post_id not found or invalid type.");
    }
    else if (false == jsonStorageKey.is<const char*>())
    {
        LOG_WARNING("Response artwork storage_key not found or invalid type.");
    }
    else if (false == jsonStorageShard.is<const char*>())
    {
        LOG_WARNING("Response artwork storage_shard not found or invalid type.");
    }
    else if (false == jsonNativeFormat.is<const char*>())
    {
        LOG_WARNING("Response artwork native_format not found or invalid type.");
    }
    else if (false == jsonWidth.is<uint16_t>())
    {
        LOG_WARNING("Response artwork width not found or invalid type.");
    }
    else if (false == jsonHeight.is<uint16_t>())
    {
        LOG_WARNING("Response artwork height not found or invalid type.");
    }
    else if (false == jsonDwellTimeMs.is<uint32_t>())
    {
        LOG_WARNING("Response artwork dwell_time_ms not found or invalid type.");
    }
    else
    {
        uint16_t width  = jsonWidth.as<uint16_t>();
        uint16_t height = jsonHeight.as<uint16_t>();

        if ((CONFIG_LED_MATRIX_WIDTH < width) ||
            (CONFIG_LED_MATRIX_HEIGHT < height))
        {
            LOG_WARNING("Canvas size %ux%u not supported.", width, height);
        }
        else
        {
            uint32_t    postId       = jsonPostId.as<uint32_t>();
            const char* storageKey   = jsonStorageKey.as<const char*>();
            const char* storageShard = jsonStorageShard.as<const char*>();
            const char* nativeFormat = jsonNativeFormat.as<const char*>();
            uint32_t    dwellTimeMs  = jsonDwellTimeMs.as<uint32_t>();
            int32_t     playlistIdx  = m_playlist.add(postId, storageKey, storageShard, nativeFormat, dwellTimeMs, true);

            if (0 > playlistIdx)
            {
                LOG_WARNING("Failed to add artwork to playlist.");
            }
            else
            {
                LOG_INFO("Artwork %s added to playlist.", storageKey);

                if (true == m_showArtwork)
                {
                    LOG_INFO("Select artwork %s in playlist.", storageKey);

                    m_playlist.select(static_cast<uint8_t>(playlistIdx));

                    m_showArtwork = false;
                }
            }
        }
    }
}

void Channel::onResponsePlaylist(const JsonObjectConst& jsonObj)
{
    LOG_WARNING("Not implemented yet.");
}

const char* Channel::channelIdToStr(ChannelId channel) const
{
    const char* channelStr = "unknown";

    if (static_cast<size_t>(channel) < (sizeof(gChannelAsStr) / sizeof(gChannelAsStr[0U])))
    {
        channelStr = gChannelAsStr[static_cast<size_t>(channel)];
    }

    return channelStr;
}

Channel::ChannelId Channel::strToChannelId(const char* channelStr) const
{
    ChannelId channelId = CHANNEL_ID_ALL;

    for (size_t idx = 0U; idx < (sizeof(gChannelAsStr) / sizeof(gChannelAsStr[0U])); ++idx)
    {
        if (0 == strcmp(channelStr, gChannelAsStr[idx]))
        {
            channelId = static_cast<ChannelId>(idx);
            break;
        }
    }

    return channelId;
}

const char* Channel::sortOrderToStr(SortOrder sortOrder) const
{
    const char* sortOrderStr = "unknown";

    if (static_cast<size_t>(sortOrder) < (sizeof(gSortOrderAsStr) / sizeof(gSortOrderAsStr[0U])))
    {
        sortOrderStr = gSortOrderAsStr[static_cast<size_t>(sortOrder)];
    }

    return sortOrderStr;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

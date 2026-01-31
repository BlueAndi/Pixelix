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
 * @file   Playlist.h
 * @brief  Artwork playlist
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef PLAYLIST_H
#define PLAYLIST_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <WString.h>
#include <Logging.h>
#include <BitmapWidget.h>
#include <FileUtil.h>
#include <vector>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The playlist manages a list of artwork ids.
 */
class Playlist
{
public:

    /**
     * Constructs the playlist.
     *
     * @param[in] maxEntries   Maximum number of playlist entries.
     */
    Playlist(uint16_t maxEntries = PLAYLIST_MAX_ENTRIES) :
        m_maxEntries(maxEntries),
        m_playlist(),
        m_beginIdx(0U),
        m_selectedIdx(-1)
    {
    }

    /**
     * Copy constructor.
     *
     * @param[in] other    Other playlist
     */
    Playlist(const Playlist& other) :
        m_maxEntries(other.m_maxEntries),
        m_playlist(other.m_playlist),
        m_beginIdx(other.m_beginIdx),
        m_selectedIdx(other.m_selectedIdx)
    {
    }

    /**
     * Destroys the playlist.
     */
    ~Playlist()
    {
    }

    /**
     * Assignment operator.
     *
     * @param[in] other    Other playlist
     *
     * @return This playlist
     */
    Playlist& operator=(const Playlist& other);

    /**
     * Get number of entries in playlist.
     *
     * @return Number of entries.
     */
    uint32_t length() const
    {
        return m_playlist.size();
    }

    /**
     * Add artwork to playlist.
     *
     * @param[in] postId       Artwork post ID.
     * @param[in] storageKey   Artwork storage key.
     * @param[in] storageShard Artwork storage shard.
     * @param[in] nativeFormat Artwork native format.
     * @param[in] dwellTime    Dwell time in ms.
     * @param[in] overwrite    If true and playlist is full, it will overwrite the oldest entry.
     *
     * @return If successful, it will return the playlist index otherwise -1.
     */
    int32_t add(uint32_t postId, const String& storageKey, const String& storageShard, const String& nativeFormat, uint32_t dwellTime, bool overwrite)
    {
        return add(postId, storageKey.c_str(), storageShard.c_str(), nativeFormat.c_str(), dwellTime, overwrite);
    }

    /**
     * Add artwork to playlist.
     * If playlist is full and overwrite is false, the artwork will not be added.
     * If playlist is fulll and overwrite is true, the oldest artwork will be replaced.
     *
     * @param[in] postId       Artwork post ID.
     * @param[in] storageKey   Artwork storage key.
     * @param[in] storageShard Artwork storage shard.
     * @param[in] nativeFormat Artwork native format.
     * @param[in] dwellTime    Dwell time in ms.
     * @param[in] overwrite    If true and playlist is full, it will overwrite the oldest entry.
     *
     * @return If successful, it will return the playlist index otherwise -1.
     */
    int32_t add(uint32_t postId, const char* storageKey, const char* storageShard, const char* nativeFormat, uint32_t dwellTime, bool overwrite);

    /**
     * Get index of selected artwork.
     *
     * @return If playlist is empty, it will return -1 otherwise the selected index.
     */
    int32_t selected() const
    {
        return m_selectedIdx;
    }

    /**
     * Select artwork by index.
     *
     * @param[in] index   Playlist index.
     */
    void select(uint32_t index);

    /**
     * Get post ID of current selected artwork.
     *
     * @return If successful, it will return the post ID otherwise 0.
     */
    uint32_t getPostId() const;

    /**
     * Get storage key of current selected artwork.
     *
     * @return If successful, it will return the storage key otherwise an empty string.
     */
    String getStorageKey() const;

    /**
     * Get URL of current selected artwork.
     *
     * @return If successful, it will return the URL otherwise an empty string.
     */
    String getUrl() const;

    /**
     * Get dwell time of current selected artwork.
     *
     * @return If successful, it will return the dwell time in ms otherwise 0.
     */
    uint32_t getDwellTime() const;
    /**
     * Select next artwork in playlist.
     *
     * @return If playlist is empty, it will return false otherwise true.
     */
    bool next();

    /**
     * Select previous artwork in playlist.
     *
     * @return If playlist is empty, it will return false otherwise true.
     */
    bool prev();

    /** Maximum number of playlist entries. */
    static const uint16_t PLAYLIST_MAX_ENTRIES = 10U;

private:

    /** Playlist entry. */
    struct Entry
    {
        /**
         * Artwork post ID, which is the server side database ID.
         * Its required for status updates.
         */
        uint32_t postId;

        /**
         * Artwork storage key, which is the unique UUID.
         */
        String storageKey;

        /**
         * Storage shard used for vault path resolution.
         */
        String storageShard;

        /**
         * Artwork native format (e.g. jpg, png, webp).
         */
        String nativeFormat;

        /**
         * Artwork dwell time in milliseconds.
         */
        uint32_t dwellTime;

        /** Default constructor. */
        Entry() :
            postId(0U),
            storageKey(),
            storageShard(),
            nativeFormat(),
            dwellTime(0U)
        {
        }
    };

    /** Base URL for artwork retrieval. */
    static const char* BASE_URL;

    /**
     * Playlist type.
     */
    typedef std::vector<Entry> PlaylistType;

    uint16_t                   m_maxEntries;  /**< Maximum number of playlist entries. */
    PlaylistType               m_playlist;    /**< Playlist entries. */
    uint32_t                   m_beginIdx;    /**< Index of begin playlist entry. */
    int32_t                    m_selectedIdx; /**< Index of current selected playlist entry. */

    /**
     * Adjust artwork URL to download only supported image formats.
     *
     * @param[in,out] artworkUrl   Artwork URL to adjust.
     */
    void adjustArtworkUrlForSupportedImageFormats(String& artworkUrl) const;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* PLAYLIST_H */

/** @} */

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
 * @file   Playlist.hpp
 * @brief  Artwork playlist
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef PLAYLIST_HPP
#define PLAYLIST_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <WString.h>
#include <Logging.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The playlist manages a list of artwork ids.
 */
template < int32_t MAX_ENTRIES>
class Playlist
{
public:

    /**
     * Constructs the playlist.
     */
    Playlist() :
        m_playlist(),
        m_beginIdx(0U),
        m_length(0U),
        m_selectedIdx(-1)
    {
    }

    /**
     * Destroys the playlist.
     */
    ~Playlist()
    {
    }

    /**
     * Get number of entries in playlist.
     *
     * @return Number of entries.
     */
    uint32_t length() const
    {
        return m_length;
    }

    /**
     * Add artwork to playlist.
     *
     * @param[in] postId       Artwork post ID.
     * @param[in] storageKey   Artwork storage key.
     * @param[in] url          Artwork URL.
     * @param[in] dwellTime    Dwell time in ms.
     * @param[in] overwrite    If true and playlist is full, it will overwrite the oldest entry.
     *
     * @return If successful, it will return the playlist index otherwise -1.
     */
    int32_t add(uint32_t postId, const String& storageKey, const String& url, uint32_t dwellTime, bool overwrite)
    {
        return add(postId, storageKey.c_str(), url.c_str(), dwellTime, overwrite);
    }

    /**
     * Add artwork to playlist.
     *
     * @param[in] postId       Artwork post ID.
     * @param[in] storageKey   Artwork storage key.
     * @param[in] url          Artwork URL.
     * @param[in] dwellTime    Dwell time in ms.
     * @param[in] overwrite    If true and playlist is full, it will overwrite the oldest entry.
     *
     * @return If successful, it will return the playlist index otherwise -1.
     */
    int32_t add(uint32_t postId, const char* storageKey, const char* url, uint32_t dwellTime, bool overwrite)
    {
        int32_t playlistIdx = -1;

        if ((nullptr != storageKey) &&
            (nullptr != url) &&
            ('\0' != storageKey[0U]) &&
            ('\0' != url[0U]) &&
            (true == isImageFormatSupported(url)))
        {
            if ((MAX_ENTRIES > m_length) ||                         /* Playlist not full? */
                ((MAX_ENTRIES == m_length) && (true == overwrite))) /* Playlist full, but allowed to overwrite? */
            {
                uint8_t insertIdx = (m_beginIdx + m_length) % MAX_ENTRIES;

                if (MAX_ENTRIES > m_length)
                {
                    ++m_length;
                }
                else
                {
                    m_beginIdx = (m_beginIdx + 1U) % MAX_ENTRIES;
                }

                m_playlist[insertIdx].postId     = postId;
                m_playlist[insertIdx].storageKey = storageKey;
                m_playlist[insertIdx].url        = url;
                m_playlist[insertIdx].dwellTime  = dwellTime;

                playlistIdx                      = insertIdx;
            }
        }

        return playlistIdx;
    }

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
    void select(uint32_t index)
    {
        if (m_length > index)
        {
            m_selectedIdx = index;
        }
    }

    /**
     * Get post ID of current selected artwork.
     *
     * @return If successful, it will return the post ID otherwise 0.
     */
    uint32_t getPostId() const
    {
        uint32_t postId = 0U;

        if (0U < m_length)
        {
            postId = m_playlist[m_selectedIdx].postId;
        }

        return postId;
    }

    /**
     * Get storage key of current selected artwork.
     *
     * @return If successful, it will return the storage key otherwise an empty string.
     */
    String getStorageKey() const
    {
        String storageKey;

        if (0U < m_length)
        {
            storageKey = m_playlist[m_selectedIdx].storageKey;
        }

        return storageKey;
    }

    /**
     * Get URL of current selected artwork.
     *
     * @return If successful, it will return the URL otherwise an empty string.
     */
    String getUrl() const
    {
        String url;

        if (0U < m_length)
        {
            url = m_playlist[m_selectedIdx].url;
        }

        return url;
    }

    /**
     * Get dwell time of current selected artwork.
     *
     * @return If successful, it will return the dwell time in ms otherwise 0.
     */
    uint32_t getDwellTime() const
    {
        uint32_t dwellTime = 0U;

        if (0U < m_length)
        {
            dwellTime = m_playlist[m_selectedIdx].dwellTime;
        }

        return dwellTime;
    }

    /**
     * Select next artwork in playlist.
     *
     * @return If playlist is empty, it will return false otherwise true.
     */
    bool next()
    {
        bool isSuccessful = false;

        if (0U < m_length)
        {
            ++m_selectedIdx;
            m_selectedIdx %= m_length;

            isSuccessful   = true;
        }

        return isSuccessful;
    }

    /**
     * Select previous artwork in playlist.
     *
     * @return If playlist is empty, it will return false otherwise true.
     */
    bool prev()
    {
        bool isSuccessful = false;

        if (0U < m_length)
        {
            if (0U == m_selectedIdx)
            {
                m_selectedIdx = m_length - 1U;
            }
            else
            {
                --m_selectedIdx;
            }

            isSuccessful = true;
        }

        return isSuccessful;
    }

private:

    /** Playlist entry. */
    struct Entry
    {
        uint32_t postId;     /**< Artwork post ID. */
        String   storageKey; /**< Artwork storage key. */
        String   url;        /**< Artwork URL. */
        uint32_t dwellTime;  /**< Dwell time in ms. */

        /** Default constructor. */
        Entry() :
            postId(0U),
            storageKey(),
            url(),
            dwellTime(0U)
        {
        }
    };

    Entry    m_playlist[MAX_ENTRIES]; /**< Playlist entries. */
    uint32_t m_beginIdx;              /**< Index of begin playlist entry. */
    uint32_t m_length;                /**< Number of playlist entries. */
    int32_t  m_selectedIdx;           /**< Index of current selected playlist entry. */

    /**
     * Check if image format is supported.
     *
     * @param[in] artworkUrl  Artwork URL
     *
     * @return If image format is supported, it will return true otherwise false.
     */
    bool isImageFormatSupported(const char* artworkUrl) const
    {
        bool isSupported = false;

        if (nullptr != artworkUrl)
        {
            const char* extension = strrchr(artworkUrl, '.');

            if (nullptr != extension)
            {
                ++extension; /* Skip '.' character. */

                if (0 == strcasecmp("bmp", extension))
                {
                    isSupported = true;
                }
                else if (0 == strcasecmp("gif", extension))
                {
                    isSupported = true;
                }
                else
                {
                    LOG_WARNING("Image format '%s' is not supported.", extension);
                }
            }
            else
            {
                LOG_WARNING("No image format found in artwork URL.");
            }
        }

        return isSupported;
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* PLAYLIST_HPP */

/** @} */

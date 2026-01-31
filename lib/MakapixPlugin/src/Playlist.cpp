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
 * @file   Playlist.cpp
 * @brief  Artwork playlist
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Playlist.h"

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

/* Initialize constant values. */
const char* Playlist::BASE_URL = "http://vault.makapix.club/";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

Playlist& Playlist::operator=(const Playlist& other)
{
    if (this != &other)
    {
        m_maxEntries  = other.m_maxEntries;
        m_playlist    = other.m_playlist;
        m_beginIdx    = other.m_beginIdx;
        m_selectedIdx = other.m_selectedIdx;
    }

    return *this;
}

int32_t Playlist::add(uint32_t postId, const char* storageKey, const char* storageShard, const char* nativeFormat, uint32_t dwellTime, bool overwrite)
{
    int32_t playlistIdx = -1;

    if ((nullptr != storageKey) &&
        (nullptr != storageShard) &&
        (nullptr != nativeFormat) &&
        ('\0' != storageKey[0U]) &&
        ('\0' != storageShard[0U]) &&
        ('\0' != nativeFormat[0U]))
    {
        if ((m_maxEntries > m_playlist.size()) ||                         /* Playlist not full? */
            ((m_maxEntries == m_playlist.size()) && (true == overwrite))) /* Playlist full, but allowed to overwrite? */
        {
            Entry   entry;
            uint8_t insertIdx  = (m_beginIdx + m_playlist.size()) % m_maxEntries;

            entry.postId       = postId;
            entry.storageKey   = storageKey;
            entry.storageShard = storageShard;
            entry.nativeFormat = nativeFormat;
            entry.dwellTime    = dwellTime;

            if (m_maxEntries > m_playlist.size())
            {
                m_playlist.emplace_back(entry);
            }
            else
            {
                m_playlist[insertIdx] = entry;
                m_beginIdx            = (m_beginIdx + 1U) % m_maxEntries;
            }


            playlistIdx = insertIdx;
        }
    }

    return playlistIdx;
}

void Playlist::select(uint32_t index)
{
    if (m_playlist.size() > index)
    {
        m_selectedIdx = index;
    }
}

uint32_t Playlist::getPostId() const
{
    uint32_t postId = 0U;

    if (0U < m_playlist.size())
    {
        const Entry& entry = m_playlist[m_selectedIdx];

        postId             = entry.postId;
    }

    return postId;
}

String Playlist::getStorageKey() const
{
    String storageKey;

    if (0U < m_playlist.size())
    {
        const Entry& entry = m_playlist[m_selectedIdx];

        storageKey         = entry.storageKey;
    }

    return storageKey;
}

String Playlist::getUrl() const
{
    String url;

    if (0U < m_playlist.size())
    {
        const Entry& entry  = m_playlist[m_selectedIdx];

        url                 = BASE_URL;
        url                += entry.storageShard;
        url                += "/";
        url                += entry.storageKey;
        url                += ".";
        url                += entry.nativeFormat;

        adjustArtworkUrlForSupportedImageFormats(url);
    }

    return url;
}

uint32_t Playlist::getDwellTime() const
{
    uint32_t dwellTime = 0U;

    if (0U < m_playlist.size())
    {
        const Entry& entry = m_playlist[m_selectedIdx];

        dwellTime          = entry.dwellTime;
    }

    return dwellTime;
}

bool Playlist::next()
{
    bool isSuccessful = false;

    if (0U < m_playlist.size())
    {
        ++m_selectedIdx;
        m_selectedIdx %= m_playlist.size();

        isSuccessful   = true;
    }

    return isSuccessful;
}

bool Playlist::prev()
{
    bool isSuccessful = false;

    if (0U < m_playlist.size())
    {
        if (0U == m_selectedIdx)
        {
            m_selectedIdx = m_playlist.size() - 1U;
        }
        else
        {
            --m_selectedIdx;
        }

        isSuccessful = true;
    }

    return isSuccessful;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void Playlist::adjustArtworkUrlForSupportedImageFormats(String& artworkUrl) const
{
    bool isSupportedFormat = BitmapWidget::isImageTypeSupported(artworkUrl);

    /* If not, try to adjust the URL to use a supported format. */
    if (false == isSupportedFormat)
    {
        /* Use always the GIF format, because it's available for static images
         * and for animations.
         */
        String imageFormat = FileUtil::getFileExtension(artworkUrl);

        artworkUrl.replace(imageFormat, "gif");
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

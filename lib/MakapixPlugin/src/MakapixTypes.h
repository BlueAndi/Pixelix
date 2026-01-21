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
 * @file   MakapixTypes.h
 * @brief  Makapix common types
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef MAKAPIX_TYPES_H
#define MAKAPIX_TYPES_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <functional>
#include <ArduinoJson.h>
#include "FileCache.hpp"
#include "Playlist.hpp"

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Constants */
namespace Constant
{
/**
 * Cache maximum number of artwork files.
 */
static constexpr uint8_t CACHE_MAX_FILES          = 10U;

/**
 * Playlist maximum number of entries.
 */
static constexpr uint8_t PLAYLIST_MAX_ENTRIES     = 20U;

/**
 * Maximum number of items to add to the playlist once when playing a channel.
 * It should be lower than or equal to PLAYLIST_MAX_ENTRIES. But consider the
 * required JSON document size for the MQTT response.
 */
static constexpr uint32_t CHANNEL_PAGE_ITEM_LIMIT = 5U;

} /* namespace Constant */

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Makapix file cache type. */
typedef FileCache<Constant::CACHE_MAX_FILES> MakapixFileCache;

/** Makapix playlist type. */
typedef Playlist<Constant::PLAYLIST_MAX_ENTRIES> MakapixPlaylist;

/** Show artwork callback prototype. */
typedef std::function<void(void)> MakapixShowArtworkCallback;

/** Next artwork callback prototype. */
typedef std::function<void(void)> MakapixNextArtworkCallback;

/** Previous artwork callback prototype. */
typedef std::function<void(void)> MakapixPrevArtworkCallback;

/** Play channel callback prototype. */
typedef std::function<void(const char* channelName)> MakapixPlayChannelCallback;

/** On response callback prototype. */
typedef std::function<void(const JsonDocument& jsonDoc)> MakapixOnResponseCallback;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* MAKAPIX_TYPES_H */

/** @} */

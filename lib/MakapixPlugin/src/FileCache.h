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
 * @file   FileCache.h
 * @brief  File cache handler
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef FILE_CACHE_H
#define FILE_CACHE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <FileSystem.h>
#include <FileUtil.h>
#include <Logging.h>
#include <BitmapWidget.h>
#include <vector>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The file cache handler is responsible to manage the cached files.
 */
class FileCache
{
public:

    /**
     * Constructs the file cache handler.
     *
     * @param[in] maxFiles  Maximum number of cached files.
     */
    FileCache(uint16_t maxFiles = CACHE_MAX_FILES) :
        m_maxEntries(maxFiles),
        m_fileList()
    {
    }

    /**
     * Constructs the file cache handler.
     *
     * @param[in] other Another file cache handler.
     */
    FileCache(const FileCache&) :
        m_maxEntries(0U),
        m_fileList()
    {
    }

    /**
     * Destroys the file cache handler.
     */
    ~FileCache()
    {
    }

    /**
     * Assignment operator.
     *
     * @param[in] other    Other file cache handler.
     *
     * @return This file cache handler.
     */
    FileCache& operator=(const FileCache& other);

    /**
     * Initialize the file cache handler.
     *
     * @param[in] path  Path where the cached files are stored. Path must end with "/".
     *
     * @return If successful, it will return true otherwise false.
     */
    bool init(const char* path);

    /**
     * Get file path by id.
     *
     * @param[in] id    File id, which is the filename without extension.
     *
     * @return If found, it will return the file path otherwise an empty string.
     */
    String getFilePathById(const char* id);

    /**
     * Get file path by id.
     *
     * @param[in] id    File id, which is the filename without extension.
     *
     * @return If found, it will return the file path otherwise an empty string.
     */
    String getFilePathById(const String& id)
    {
        return getFilePathById(id.c_str());
    }

    /**
     * Add file to cache.
     * If file already exists, it will update the timestamp.
     * If file doesn't exist, it will be added to the cache.
     * If the cache is full, the oldest file will be removed to make place for the new file.
     *
     * @param[in] filename  Filename with full path.
     */
    void addFile(const String& filename);

    /**
     * Remove file from cache by id.
     *
     * @param[in] id    File id, which is the filename without extension.
     */
    void remove(const char* id);

    /**
     * Remove file from cache by id.
     *
     * @param[in] id    File id, which is the filename without extension.
     */
    void remove(const String& id)
    {
        remove(id.c_str());
    }

    /**
     * Cache maximum number of artwork files.
     */
    static const uint16_t CACHE_MAX_FILES = 10U;

private:

    /** File cache entry. */
    struct Entry
    {
        String filename;  /**< Filename. */
        time_t timestamp; /**< Last access timestamp. */

        /** Default constructor. */
        Entry() :
            filename(),
            timestamp(0U)
        {
        }
    };

    /**
     * File list type.
     */
    typedef std::vector<Entry> FileList;

    uint16_t                   m_maxEntries; /**< Maximum number of cache entries. */
    FileList                   m_fileList;   /**< List of cached files. */

    /**
     * Scan for files in the cache directory.
     * It will setup the internal file list.
     *
     * @param[in] path  Path where the cached files are stored.
     */
    void scanForFiles(const char* path);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* FILE_CACHE_H */

/** @} */

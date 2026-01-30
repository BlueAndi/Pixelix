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
 * @file   FileCache.hpp
 * @brief  File cache handler
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef FILE_CACHE_HPP
#define FILE_CACHE_HPP

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

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The file cache handler is responsible to manage the cached files.
 */
template < uint8_t MAX_FILES >
class FileCache
{
public:

    /**
     * Constructs the file cache handler.
     */
    FileCache() :
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
     * Initialize the file cache handler.
     *
     * @param[in] path  Path where the cached files are stored. Path must end with "/".
     *
     * @return If successful, it will return true otherwise false.
     */
    bool init(const char* path)
    {
        bool isSuccessful = false;

        if (nullptr != path)
        {
            size_t pathLen = strlen(path);

            if ('/' == path[pathLen - 1U])
            {
                FileUtil::createDirectories(path);
                scanForFiles(path);

                isSuccessful = true;
            }
        }

        return isSuccessful;
    }

    /**
     * Get file path by id.
     *
     * @param[in] id    File id, which is the filename without extension.
     *
     * @return If found, it will return the file path otherwise an empty string.
     */
    String getFilePathById(const char* id)
    {
        String filePath;

        /* <PATH><FILENAME>.<FILENAME-EXTENSION>
         * Example: /makapix_cache/9ed6-46a9-b616-313f2670602a.gif -> id: 9ed6-46a9-b616-313f2670602a
         */

        if (nullptr != id)
        {
            for (uint8_t idx = 0U; idx < MAX_FILES; ++idx)
            {
                Entry& entry    = m_fileList[idx];
                String filename = FileUtil::getFileName(entry.filename);

                if (true == filename.startsWith(id))
                {
                    filePath = entry.filename;
                    break;
                }
            }
        }

        return filePath;
    }

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
    void addFile(const String& filename)
    {
        time_t  currentTime = time(nullptr);
        uint8_t idx;
        uint8_t emptyIdx   = MAX_FILES;
        uint8_t oldestIdx  = MAX_FILES;
        time_t  oldestTime = INT32_MAX;

        LOG_DEBUG("Adding file to cache: %s", filename.c_str());

        /* Search for existing file, empty slot or oldest file. */
        for (idx = 0U; idx < MAX_FILES; ++idx)
        {
            Entry& entry = m_fileList[idx];

            /* File already exists? */
            if (true == entry.filename.equals(filename))
            {
                /* Update timestamp of existing file. */
                entry.timestamp = currentTime;
                break;
            }

            /* Empty slot? */
            if (true == entry.filename.isEmpty())
            {
                emptyIdx = idx;
                break;
            }

            /* Remember oldest file. */
            if (entry.timestamp < oldestTime)
            {
                /* Remember oldest file. */
                oldestTime = entry.timestamp;
                oldestIdx  = idx;
            }
        }

        /* Add new file? */
        if (MAX_FILES != emptyIdx)
        {
            LOG_DEBUG("Adding file to cache (%u): %s", emptyIdx, filename.c_str());

            /* Use empty slot. */
            m_fileList[emptyIdx].filename  = filename;
            m_fileList[emptyIdx].timestamp = currentTime;
        }
        /* Use oldest slot? */
        else if (MAX_FILES != oldestIdx)
        {
            LOG_DEBUG("Remove oldest cached file: %s", m_fileList[oldestIdx].filename.c_str());

            /* Remove oldest file and use its slot. */
            if (false == FILESYSTEM.remove(m_fileList[oldestIdx].filename))
            {
                LOG_WARNING("Failed to remove oldest cached file: %s", m_fileList[oldestIdx].filename.c_str());
            }

            LOG_DEBUG("Adding file to cache (%u): %s", oldestIdx, filename.c_str());

            m_fileList[oldestIdx].filename  = filename;
            m_fileList[oldestIdx].timestamp = currentTime;
        }
        else if (MAX_FILES > idx)
        {
            LOG_DEBUG("File cache file (%u) timetamp updated.", idx);
        }
        else
        {
            LOG_ERROR("Internal file cache error: %s", filename.c_str());
        }
    }

    /**
     * Remove file from cache by id.
     *
     * @param[in] id    File id, which is the filename without extension.
     */
    void remove(const char* id)
    {
        if (nullptr != id)
        {
            for (uint8_t idx = 0U; idx < MAX_FILES; ++idx)
            {
                Entry& entry    = m_fileList[idx];
                String filename = FileUtil::getFileName(entry.filename);

                if (true == filename.startsWith(id))
                {
                    if (false == FILESYSTEM.remove(entry.filename))
                    {
                        LOG_WARNING("Failed to remove cached file: %s", entry.filename.c_str());
                    }

                    entry.filename.clear();
                    entry.timestamp = 0U;
                    break;
                }
            }
        }
    }

    /**
     * Remove file from cache by id.
     *
     * @param[in] id    File id, which is the filename without extension.
     */
    void remove(const String& id)
    {
        remove(id.c_str());
    }

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

    Entry m_fileList[MAX_FILES]; /**< List of cached files. */

    /**
     * Scan for files in the cache directory.
     * It will setup the internal file list.
     *
     * @param[in] path  Path where the cached files are stored.
     */
    void scanForFiles(const char* path)
    {
        if (nullptr != path)
        {
            File    fdRoot      = FILESYSTEM.open(path, "r");
            File    fd          = fdRoot.openNextFile();
            uint8_t fileListIdx = 0U;

            /* Scan for files only flat!
             * No subdirectory scanning.
             * If there are more files than MAX_FILES, the remaining files will be removed.
             */
            while (true == fd)
            {
                String fullPath = fd.path();

                if ((false == fd.isDirectory()) &&
                    (true == BitmapWidget::isImageTypeSupported(fullPath)))
                {
                    LOG_DEBUG("Found cached file: %s", fullPath.c_str());

                    if (MAX_FILES > fileListIdx)
                    {
                        Entry& entry    = m_fileList[fileListIdx];

                        entry.filename  = fullPath;
                        entry.timestamp = fd.getLastWrite();

                        ++fileListIdx;
                    }
                }

                fd.close();

                /* Remove files, if there are more than MAX_FILES. */
                if ((false == fd.isDirectory()) &&
                    (true == BitmapWidget::isImageTypeSupported(fullPath)) &&
                    (MAX_FILES <= fileListIdx))
                {
                    LOG_WARNING("Removing excess cached file: %s", fullPath.c_str());

                    if (false == FILESYSTEM.remove(fullPath))
                    {
                        LOG_WARNING("Failed to remove excess cached file: %s", fullPath.c_str());
                    }
                }

                fd = fdRoot.openNextFile();
            }

            /* Cleanup */
            if (true == fd)
            {
                fd.close();
            }

            if (true == fdRoot)
            {
                fdRoot.close();
            }

            /* Clear remaining entries. */
            while (MAX_FILES > fileListIdx)
            {
                Entry& entry = m_fileList[fileListIdx];

                entry.filename.clear();
                entry.timestamp = 0U;

                ++fileListIdx;
            }
        }
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* FILE_CACHE_HPP */

/** @} */

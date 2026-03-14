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
 * @file   FileCache.h
 * @brief  File cache handler
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "FileCache.h"
#include "MakapixTypes.h"

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

/******************************************************************************
 * Public Methods
 *****************************************************************************/

FileCache& FileCache::operator=(const FileCache& other)
{
    if (this != &other)
    {
        m_maxEntries = other.m_maxEntries;
        m_fileList   = other.m_fileList;
    }

    return *this;
}

bool FileCache::init(const char* path)
{
    bool isSuccessful = false;

    if (nullptr != path)
    {
        size_t pathLen = strlen(path);

        if ('/' == path[pathLen - 1U])
        {
            FileUtil::createDirectories(path, FILE_CACHE_FS);
            scanForFiles(path);

            isSuccessful = true;
        }
    }

    return isSuccessful;
}

String FileCache::getFilePathById(const char* id)
{
    String filePath;

    /* <PATH><FILENAME>.<FILENAME-EXTENSION>
     * Example: /makapix_cache/9ed6-46a9-b616-313f2670602a.gif -> id: 9ed6-46a9-b616-313f2670602a
     */

    if ((nullptr != id) &&
        (0U < m_fileList.size()))
    {
        FileList::const_iterator it  = m_fileList.cbegin();
        FileList::const_iterator end = m_fileList.cend();

        while (it != end)
        {
            const Entry& entry    = *it;
            String       filename = FileUtil::getFileName(entry.filename);

            if (true == filename.startsWith(id))
            {
                filePath = entry.filename;
                break;
            }

            ++it;
        }
    }

    return filePath;
}

void FileCache::addFile(const String& filename)
{
    time_t currentTime = time(nullptr);
    bool   found       = false;
    Entry* oldestEntry = nullptr;

    /* Search for existing file slot or oldest file. */
    if (0U < m_fileList.size())
    {
        FileList::iterator it  = m_fileList.begin();
        FileList::iterator end = m_fileList.end();

        while ((it != end) && (false == found))
        {
            Entry& entry = *it;

            /* File already exists? */
            if (true == entry.filename.equals(filename))
            {
                /* Update timestamp of existing file. */
                entry.timestamp = currentTime;
                found           = true;
            }
            /* Remember oldest file. */
            else if ((nullptr == oldestEntry) ||
                     (entry.timestamp < oldestEntry->timestamp))
            {
                /* Remember oldest file. */
                oldestEntry = &entry;
            }
            else
            {
                /* Nothing to do. */
                ;
            }

            ++it;
        }
    }

    /* Entry not found? */
    if (false == found)
    {
        /* A empty slot available? */
        if (m_maxEntries > m_fileList.size())
        {
            Entry entry;

            LOG_DEBUG("Adding file to cache (%u): %s", m_fileList.size(), filename.c_str());

            entry.filename  = filename;
            entry.timestamp = currentTime;

            m_fileList.emplace_back(entry);
        }
        /* Use oldest slot? */
        else if (nullptr != oldestEntry)
        {
            LOG_DEBUG("Remove oldest cached file: %s", oldestEntry->filename.c_str());

            /* Remove oldest file and use its slot. */
            if (false == FILE_CACHE_FS.remove(oldestEntry->filename))
            {
                LOG_WARNING("Failed to remove oldest cached file: %s", oldestEntry->filename.c_str());
            }

            LOG_DEBUG("Adding file to cache: %s", filename.c_str());

            oldestEntry->filename  = filename;
            oldestEntry->timestamp = currentTime;
        }
        else
        {
            LOG_ERROR("Internal file cache error: %s", filename.c_str());
        }
    }
}

void FileCache::remove(const char* id)
{
    if (nullptr != id)
    {
        FileList::iterator it  = m_fileList.begin();
        FileList::iterator end = m_fileList.end();

        while (it != end)
        {
            Entry& entry    = *it;
            String filename = FileUtil::getFileName(entry.filename);

            if (true == filename.startsWith(id))
            {
                if (false == FILE_CACHE_FS.remove(entry.filename))
                {
                    LOG_WARNING("Failed to remove cached file: %s", entry.filename.c_str());
                }

                /* Remove entry from cache. */
                (void)m_fileList.erase(it);
                break;
            }

            ++it;
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void FileCache::scanForFiles(const char* path)
{
    if (nullptr != path)
    {
        File    fdRoot      = FILE_CACHE_FS.open(path, "r");
        File    fd          = fdRoot.openNextFile();
        uint8_t fileListIdx = 0U;

        m_fileList.clear();

        /* Scan for files only flat!
         * No subdirectory scanning.
         * If there are more files than m_maxEntries, the remaining files will be removed.
         */
        while (true == fd)
        {
            String fullPath = fd.path();

            if ((false == fd.isDirectory()) &&
                (true == BitmapWidget::isImageTypeSupported(fullPath)))
            {
                LOG_DEBUG("Found cached file: %s", fullPath.c_str());

                if (m_maxEntries > fileListIdx)
                {
                    Entry entry;

                    entry.filename  = fullPath;
                    entry.timestamp = fd.getLastWrite();

                    m_fileList.emplace_back(entry);

                    ++fileListIdx;
                }
            }

            fd.close();

            /* Remove files, if there are more than m_maxEntries. */
            if ((false == fd.isDirectory()) &&
                (true == BitmapWidget::isImageTypeSupported(fullPath)) &&
                (m_maxEntries < fileListIdx))
            {
                LOG_WARNING("Removing excess cached file: %s", fullPath.c_str());

                if (false == FILE_CACHE_FS.remove(fullPath))
                {
                    LOG_WARNING("Failed to remove excess cached file: %s", fullPath.c_str());
                }
            }

            fd = fdRoot.openNextFile();
        }

        if (true == fdRoot)
        {
            fdRoot.close();
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

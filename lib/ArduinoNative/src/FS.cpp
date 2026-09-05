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
 * @file   FS.cpp
 * @brief  Arduino stuff for test
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "FS.h"

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

FS NativeFS;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

size_t File::size() const
{
    size_t fileSize = 0U;
    size_t currPos  = ftell(m_fd);

    if (0 == fseek(m_fd, 0, SEEK_END))
    {
        long pos = ftell(m_fd);

        if (0 <= pos)
        {
            fileSize = pos;
        }

        (void)fseek(m_fd, currPos, SEEK_SET);
    }

    return fileSize;
}

size_t File::write(uint8_t data)
{
    return write(&data, sizeof(data));
}

size_t File::write(const uint8_t* buf, size_t size)
{
    size_t written = 0U;

    if ((nullptr != m_fd) &&
        (nullptr != buf))
    {
        written = fwrite(buf, 1U, size, m_fd);
    }

    return written;
}

int File::available()
{
    int    remaining = 0;
    size_t currPos   = position();
    size_t fileSize  = size();

    if (currPos < fileSize)
    {
        remaining = static_cast<int>(fileSize - currPos);
    }

    return remaining;
}

int File::peek()
{
    int data = -1;

    if (nullptr != m_fd)
    {
        data = fgetc(m_fd);

        if (EOF != data)
        {
            (void)ungetc(data, m_fd);
        }
        else
        {
            data = -1;
        }
    }

    return data;
}

void File::flush()
{
    if (nullptr != m_fd)
    {
        (void)fflush(m_fd);
    }
}

time_t File::getLastWrite()
{
    std::string fullPath = toHostPath(m_path);
    struct stat info;
    time_t      lastWrite = 0;

    if (0 == stat(fullPath.c_str(), &info))
    {
        lastWrite = info.st_mtime;
    }

    return lastWrite;
}

File File::openNextFile(const char* mode)
{
    File file;

    if (nullptr != m_dir)
    {
        struct dirent* entry = readdir(m_dir);

        /* Skip the current and the parent directory. */
        while ((nullptr != entry) &&
               ((0 == strcmp(".", entry->d_name)) ||
                   (0 == strcmp("..", entry->d_name))))
        {
            entry = readdir(m_dir);
        }

        if (nullptr != entry)
        {
            std::string entryPath = m_path;

            if ((false == entryPath.empty()) &&
                ('/' != entryPath[entryPath.length() - 1U]))
            {
                entryPath += "/";
            }

            entryPath            += entry->d_name;

            std::string fullPath  = toHostPath(entryPath);
            struct stat info;

            if ((0 == stat(fullPath.c_str(), &info)) &&
                (0 != (info.st_mode & S_IFDIR)))
            {
                DIR* dir = opendir(fullPath.c_str());

                file     = File(nullptr, dir, entryPath, m_rootPath);
            }
            else
            {
                FILE* fd = fopen(fullPath.c_str(), mode);

                if (nullptr != fd)
                {
                    file = File(fd, nullptr, entryPath, m_rootPath);
                }
            }
        }
    }

    return file;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
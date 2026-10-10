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
 * @file   FS.h
 * @brief  Arduino stuff for test
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef FS_H
#define FS_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <Arduino.h>
#include <time.h>
#include <memory>
#include <string>
#include <dirent.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

/******************************************************************************
 * Macros
 *****************************************************************************/

#define FILE_READ "rb"
#define FILE_WRITE "wb"
#define FILE_APPEND "ab"

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/* Like the Arduino, the filesystem types live in the fs namespace and are
 * pulled into the global namespace afterwards.
 */
namespace fs
{

enum SeekMode
{
    SeekSet = 0,
    SeekCur = 1,
    SeekEnd = 2
};

/**
 * Force the binary mode in the given file open mode.
 *
 * The filesystem on the target knows binary files only. On the host a file
 * which is opened in text mode gets its line endings translated and a 0x1a is
 * interpreted as end of file. That would corrupt every file which is not pure
 * text, e.g. the images served by the webserver. The webserver opens its files
 * with "r", therefore the mode is adapted here and not at the caller.
 *
 * @param[in] mode  File open mode, like the C library uses it. May be nullptr.
 *
 * @return File open mode, which opens the file in binary mode.
 */
std::string toBinaryMode(const char* mode);

class File
{
public:

    File(FILE* fd = nullptr) :
        m_fd(fd, fileDeleter),
        m_dir(nullptr, dirDeleter),
        m_path(),
        m_name(),
        m_rootPath()
    {
    }

    /**
     * Constructs a file resp. directory.
     *
     * @param[in] fd        File descriptor, nullptr in case of a directory.
     * @param[in] dir       Directory descriptor, nullptr in case of a file.
     * @param[in] path      Path of the file, like the target uses it.
     * @param[in] rootPath  Root path of the filesystem on the host.
     */
    File(FILE* fd, DIR* dir, const std::string& path, const std::string& rootPath) :
        m_fd(fd, fileDeleter),
        m_dir(dir, dirDeleter),
        m_path(path),
        m_name(),
        m_rootPath(rootPath)
    {
        size_t pos = m_path.find_last_of('/');

        if (std::string::npos == pos)
        {
            m_name = m_path;
        }
        else
        {
            m_name = m_path.substr(pos + 1U);
        }
    }

    ~File()
    {
    }

    size_t write(uint8_t data);
    size_t write(const uint8_t* buf, size_t size);
    int    available();

    int    read()
    {
        int data = -1;

        if (nullptr != m_fd)
        {
            uint8_t byte = 0U;

            if (1U == fread(&byte, 1U, 1U, m_fd.get()))
            {
                data = byte;
            }
        }

        return data;
    }

    int    peek();
    void   flush();

    size_t read(uint8_t* buf, size_t size)
    {
        size_t readBytesCount = 0U;

        if ((nullptr != m_fd) &&
            (nullptr != buf))
        {
            readBytesCount = fread(buf, 1, size, m_fd.get());
        }

        return readBytesCount;
    }

    size_t readBytes(char* buffer, size_t length)
    {
        return read(reinterpret_cast<uint8_t*>(buffer), length);
    }

    bool seek(uint32_t pos, SeekMode mode)
    {
        bool isSuccessful = false;

        if (nullptr != m_fd)
        {
            isSuccessful = (0 == fseek(m_fd.get(), pos, mode));
        }

        return isSuccessful;
    }

    bool seek(uint32_t pos)
    {
        return seek(pos, SeekSet);
    }

    size_t position() const
    {
        size_t currPos = 0U;

        /* A directory has no file handle, therefore the position is always 0. */
        if (nullptr != m_fd)
        {
            long pos = ftell(m_fd.get());

            if (0 <= pos)
            {
                currPos = static_cast<size_t>(pos);
            }
        }

        return currPos;
    }

    size_t size() const;

    void   close()
    {
        m_fd.reset();
        m_dir.reset();
    }

    operator bool() const
    {
        return (nullptr != m_fd) || (nullptr != m_dir);
    }

    time_t getLastWrite();

    /**
     * Get the name of the file resp. directory, without any path.
     *
     * @return Name
     */
    const char* name() const
    {
        return m_name.c_str();
    }

    /**
     * Get the full path of the file resp. directory, like the target uses it.
     *
     * @return Full path
     */
    const char* path() const
    {
        return m_path.c_str();
    }

    /**
     * Is it a directory?
     *
     * @return If it is a directory, it will return true otherwise false.
     */
    boolean isDirectory(void)
    {
        return (nullptr != m_dir);
    }

    /**
     * Get the next file resp. directory inside this directory.
     *
     * @param[in] mode  File mode, used to open the next file.
     *
     * @return Next file. If there is none, the file will be invalid.
     */
    File openNextFile(const char* mode = FILE_READ);

    /**
     * Start the directory iteration from the beginning.
     */
    void rewindDirectory(void)
    {
        if (nullptr != m_dir)
        {
            rewinddir(m_dir.get());
        }
    }

private:

    /* The handle is shared, because a File is copied around by value, e.g. by
     * the webserver. Every copy would close the handle on its own, which is a
     * double free. The Arduino counterpart shares it the same way.
     */
    std::shared_ptr<FILE> m_fd;  /**< File handle, nullptr in case of a directory. */
    std::shared_ptr<DIR>  m_dir; /**< Directory handle, nullptr in case of a file. */

    /**
     * Close a file handle. Used as deleter of the shared pointer.
     *
     * @param[in] fd    File handle. May be nullptr.
     */
    static void fileDeleter(FILE* fd)
    {
        if (nullptr != fd)
        {
            (void)fclose(fd);
        }
    }

    /**
     * Close a directory handle. Used as deleter of the shared pointer.
     *
     * @param[in] dir   Directory handle. May be nullptr.
     */
    static void dirDeleter(DIR* dir)
    {
        if (nullptr != dir)
        {
            (void)closedir(dir);
        }
    }

    std::string m_path;     /**< Path of the file, like the target uses it. */
    std::string m_name;     /**< Name of the file, without any path. */
    std::string m_rootPath; /**< Root path of the filesystem on the host. */

    /**
     * Get the path of the file on the host.
     *
     * @param[in] path  Path of the file, like the target uses it.
     *
     * @return Path on the host
     */
    std::string toHostPath(const std::string& path) const
    {
        std::string hostPath = m_rootPath;

        /* Avoid a double separator, because the target paths start with one. */
        if ((false == path.empty()) &&
            ('/' != path[0]))
        {
            hostPath += "/";
        }

        hostPath += path;

        return hostPath;
    }
};

class FS
{
public:

    FS() :
        m_rootPath(".")
    {
    }

    ~FS()
    {
    }

    /**
     * Mount the filesystem below the given root path.
     *
     * The target uses absolute paths like "/configuration/1.json". On the host
     * they are mapped below the root path, so nothing is written to the root of
     * the drive. By default the current working directory is used.
     *
     * @param[in] rootPath  Root path of the filesystem. May be nullptr.
     *
     * @return If successful mounted, it will return true otherwise false.
     */
    bool begin(const char* rootPath = ".")
    {
        bool isSuccessful = false;

        if (nullptr == rootPath)
        {
            /* Guard: invalid root path. */
        }
        else
        {
            m_rootPath   = rootPath;
            isSuccessful = true;
        }

        return isSuccessful;
    }

    /**
     * Unmount the filesystem.
     */
    void end()
    {
        m_rootPath = ".";
    }

    File open(const char* path, const char* mode = FILE_READ)
    {
        std::string fullPath = buildPath(path);
        struct stat info;
        File        file;

        if ((0 == stat(fullPath.c_str(), &info)) &&
            (0 != (info.st_mode & S_IFDIR)))
        {
            DIR* dir = opendir(fullPath.c_str());

            file     = File(nullptr, dir, (nullptr == path) ? "" : path, m_rootPath);
        }
        else
        {
            std::string binaryMode = toBinaryMode(mode);
            FILE*       fd         = fopen(fullPath.c_str(), binaryMode.c_str());

            if (nullptr != fd)
            {
                file = File(fd, nullptr, (nullptr == path) ? "" : path, m_rootPath);
            }
        }

        return file;
    }

    File open(const String& path, const char* mode = FILE_READ)
    {
        return open(path.c_str(), mode);
    }

    bool exists(const char* path)
    {
        std::string fullPath = buildPath(path);
        bool        itExists = false;
        FILE*       fd       = fopen(fullPath.c_str(), "rb");

        if (nullptr != fd)
        {
            itExists = true;
            fclose(fd);
        }

        return itExists;
    }

    bool exists(const String& path)
    {
        return exists(path.c_str());
    }

    bool remove(const char* path)
    {
        std::string fullPath = buildPath(path);

        return (0 == std::remove(fullPath.c_str()));
    }

    bool remove(const String& path)
    {
        return remove(path.c_str());
    }

    bool rename(const char* pathFrom, const char* pathTo)
    {
        std::string fullPathFrom = buildPath(pathFrom);
        std::string fullPathTo   = buildPath(pathTo);

        return (0 == std::rename(fullPathFrom.c_str(), fullPathTo.c_str()));
    }

    bool rename(const String& pathFrom, const String& pathTo)
    {
        return rename(pathFrom.c_str(), pathTo.c_str());
    }

    bool mkdir(const char* path)
    {
        std::string fullPath = buildPath(path);
        int         result   = 0;

#ifdef _WIN32
        result = _mkdir(fullPath.c_str());
#else
        result = ::mkdir(fullPath.c_str(), 0777);
#endif

        return (0 == result);
    }

    bool mkdir(const String& path)
    {
        return mkdir(path.c_str());
    }

    bool rmdir(const char* path)
    {
        std::string fullPath = buildPath(path);
        int         result   = 0;

#ifdef _WIN32
        result = _rmdir(fullPath.c_str());
#else
        result = ::rmdir(fullPath.c_str());
#endif

        return (0 == result);
    }

    bool rmdir(const String& path)
    {
        return rmdir(path.c_str());
    }

    /**
     * Get the total size of the filesystem.
     * The host filesystem has no fixed size, therefore a virtual size is
     * reported, like a typical LittleFS partition has.
     *
     * @return Total size in byte
     */
    size_t totalBytes() const
    {
        return VIRTUAL_TOTAL_BYTES;
    }

    /**
     * Get the used size of the filesystem.
     * The host filesystem has no fixed size, therefore nothing is used.
     *
     * @return Used size in byte
     */
    size_t usedBytes() const
    {
        return 0U;
    }

private:

    /** Virtual filesystem size in byte, reported to the user. */
    static const size_t VIRTUAL_TOTAL_BYTES = 1024U * 1024U;


    std::string         m_rootPath; /**< Root path, which every path is relative to. */

    /**
     * Build the full path by considering the root path.
     *
     * @param[in] path  Path, as the target would use it. May be nullptr.
     *
     * @return Full path
     */
    std::string buildPath(const char* path) const
    {
        std::string fullPath = m_rootPath;

        if (nullptr == path)
        {
            /* Guard: invalid path. */
        }
        else
        {
            /* Avoid a double separator, because the target paths start with one. */
            if ('/' != path[0])
            {
                fullPath += "/";
            }

            fullPath += path;
        }

        return fullPath;
    }
};

} /* namespace fs */

using fs::File;
using fs::FS;
using fs::SeekMode;

/* The enumerators of a unscoped enum belong to the enclosing namespace, so they
 * are pulled in separately. The application uses them unqualified.
 */
using fs::SeekCur;
using fs::SeekEnd;
using fs::SeekSet;

extern fs::FS NativeFS;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* FS_H */

/** @} */
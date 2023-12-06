/* MIT License
 *
 * Copyright (c) 2019 - 2023 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Arduino stuff for test
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup test
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

/******************************************************************************
 * Macros
 *****************************************************************************/

#define FILE_READ       "r"
#define FILE_WRITE      "w"
#define FILE_APPEND     "a"

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

enum SeekMode {
    SeekSet = 0,
    SeekCur = 1,
    SeekEnd = 2
};

class File
{
public:
    File(FILE* fd) :
        m_fd(fd)
    {
    }

    ~File()
    {
    }

    size_t write(uint8_t data);
    size_t write(const uint8_t *buf, size_t size);
    int available();
    
    int read()
    {
        int data = -1;

        if (nullptr != m_fd)
        {
            int8_t byte = 0;

            if (1 != fread(&byte, 1, 1, m_fd))
            {
                data = -1;
            }
        }

        return data;
    }

    int peek();
    void flush();
    
    size_t read(uint8_t* buf, size_t size)
    {
        return fread(buf, 1, size, m_fd);
    }

    size_t readBytes(char *buffer, size_t length)
    {
        return read((uint8_t*)buffer, length);
    }

    bool seek(uint32_t pos, SeekMode mode)
    {
        return (0 == fseek(m_fd, pos, mode));
    }

    bool seek(uint32_t pos)
    {
        return seek(pos, SeekSet);
    }

    size_t position() const
    {
        return ftell(m_fd);
    }

    size_t size() const;

    void close()
    {
        fclose(m_fd);
        m_fd = nullptr;
    }

    operator bool() const
    {
        return (nullptr != m_fd);
    }

    time_t getLastWrite();
    const char* name() const;

    boolean isDirectory(void);
    File openNextFile(const char* mode = FILE_READ);
    void rewindDirectory(void);

private:

    FILE*   m_fd;

    File();
};

class FS
{
public:
    FS()
    {
    }

    ~FS()
    {
    }

    File open(const char* path, const char* mode = FILE_READ)
    {
        FILE*   fd = fopen(path, mode);

        return File(fd);
    }

    File open(const String& path, const char* mode = FILE_READ)
    {
        return open(path.c_str(), mode);
    }

    bool exists(const char* path)
    {
        bool    itExists    = false;
        FILE*   fd          = fopen("path", "r");

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

    bool remove(const char* path);
    bool remove(const String& path);

    bool rename(const char* pathFrom, const char* pathTo);
    bool rename(const String& pathFrom, const String& pathTo);

    bool mkdir(const char *path);
    bool mkdir(const String &path);

    bool rmdir(const char *path);
    bool rmdir(const String &path);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* FS_H */

/** @} */
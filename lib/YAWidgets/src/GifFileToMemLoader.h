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
 * @file   GifFileToMemLoader.h
 * @brief  GIF file to memory loader
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup GFX
 *
 * @{
 */

#ifndef GIF_FILE_TO_MEM_LOADER_H
#define GIF_FILE_TO_MEM_LOADER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IGifLoader.h"
#include <TypedAllocator.hpp>
#include <PsAllocator.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * GIF file loader, which keeps the file in the memory and closes the file itself
 * immediately.
 */
class GifFileToMemLoader : public IGifLoader
{
public:

    /**
     * Construct the GIF file loader.
     */
    GifFileToMemLoader() :
        IGifLoader(),
        m_allocator(),
        m_fileSize(0U),
        m_fileBuffer(nullptr),
        m_pos(0U)
    {
    }

    /**
     * Construct the GIF file loader by copying another loader.
     *
     * @param[in] other Loader to copy.
     */
    GifFileToMemLoader(const GifFileToMemLoader& other) :
        IGifLoader(),
        m_allocator(other.m_allocator),
        m_fileSize(other.m_fileSize),
        m_fileBuffer(nullptr),
        m_pos(other.m_pos)
    {
        if (false == copyFileBuffer(other))
        {
            close();
        }
    }

    /**
     * Destroy the GIF file loader.
     */
    ~GifFileToMemLoader() override
    {
        close();
    }

    /**
     * Assignment operator.
     *
     * @param[in] other Loader to copy.
     *
     * @return Reference to this loader.
     */
    GifFileToMemLoader& operator=(const GifFileToMemLoader& other)
    {
        if (this != &other)
        {
            close();

            m_allocator = other.m_allocator;
            m_fileSize  = other.m_fileSize;

            if (false == copyFileBuffer(other))
            {
                close();
            }
            else
            {
                m_pos = other.m_pos;
            }
        }

        return *this;
    }

    /**
     * Open a GIF file, load it to memory and closes it again.
     *
     * @param[in] fs        Filesystem to use
     * @param[in] fileName  Name of the GIF file.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool open(FS& fs, const String& fileName) final
    {
        bool isSuccessful = false;

        if (nullptr == m_fileBuffer)
        {
            File fd = fs.open(fileName);

            if (true == fd)
            {
                m_fileSize   = fd.size();
                m_fileBuffer = m_allocator.allocateArray(m_fileSize);

                if (nullptr == m_fileBuffer)
                {
                    m_fileSize = 0U;
                }
                else if (m_fileSize != fd.read(m_fileBuffer, m_fileSize))
                {
                    close();
                }
                else
                {
                    isSuccessful = true;
                }

                fd.close();
            }
        }

        return isSuccessful;
    }

    /**
     * Close the GIF loader and release any memory.
     */
    void close() final
    {
        if (nullptr != m_fileBuffer)
        {
            m_allocator.deallocateArray(m_fileBuffer);
            m_fileBuffer = nullptr;
            m_fileSize   = 0U;
            m_pos        = 0U;
        }
    }

    /**
     * Read data from GIF.
     *
     * @param[in] buffer    Buffer to fill.
     * @param[in] size      Buffer size in bytes.
     *
     * @return If successful read, it will return true otherwise false.
     */
    bool read(void* buffer, size_t size) final
    {
        bool isSuccessful = false;

        if ((nullptr != m_fileBuffer) &&
            (m_fileSize >= (m_pos + size)))
        {
            memcpy(buffer, &m_fileBuffer[m_pos], size);
            m_pos        += size;

            isSuccessful  = true;
        }

        return isSuccessful;
    }

    /**
     * Get file position.
     *
     * @return File position
     */
    size_t position() final
    {
        return m_pos;
    }

    /**
     * Set file position.
     *
     * @param[in] position  File position to set
     * @param[in] mode      The seek mode.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool seek(size_t position, SeekMode mode) final
    {
        bool isSuccessful = false;

        if (SeekSet == mode)
        {
            if (m_fileSize >= position)
            {
                m_pos        = position;

                isSuccessful = true;
            }
        }
        else if (SeekCur == mode)
        {
            if (m_fileSize >= (m_pos + position))
            {
                m_pos        += position;

                isSuccessful  = true;
            }
        }
        else
        {
            if (m_fileSize >= position)
            {
                m_pos        = m_fileSize - position;

                isSuccessful = true;
            }
        }

        return isSuccessful;
    }

    /**
     * If file is opened, it will return true otherwise false.
     *
     * @return File status
     */
    operator bool() const final
    {
        return (nullptr != m_fileBuffer);
    }

private:

    /**
     * Data allocator type for file buffer.
     */
    typedef TypedAllocator<uint8_t, PsAllocator> DataAllocator;

    DataAllocator                                m_allocator;  /**< Memory allocator. */
    size_t                                       m_fileSize;   /**< File size in byte. */
    uint8_t*                                     m_fileBuffer; /**< File buffer. */
    size_t                                       m_pos;        /**< Current read position in byte. */

    /**
     * Copy file buffer from another loader with own memory allocator.
     * If there is already a file buffer, it will be closed first.
     *
     * Position won't be changed.
     *
     * @param[in] other Loader to copy.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool copyFileBuffer(const GifFileToMemLoader& other)
    {
        bool isSuccessful = false;

        if (nullptr != m_fileBuffer)
        {
            m_allocator.deallocateArray(m_fileBuffer);
            m_fileBuffer = nullptr;
            m_fileSize   = 0U;
        }

        m_fileSize = other.m_fileSize;

        if ((nullptr != other.m_fileBuffer) &&
            (0U < m_fileSize))
        {
            m_fileBuffer = m_allocator.allocateArray(m_fileSize);

            if (nullptr == m_fileBuffer)
            {
                m_fileSize = 0U;
            }
            else
            {
                memcpy(m_fileBuffer, other.m_fileBuffer, m_fileSize);
                isSuccessful = true;
            }
        }

        return isSuccessful;
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* GIF_FILE_TO_MEM_LOADER_H */

/** @} */
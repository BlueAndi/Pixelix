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
 * @file   ImageFileLoader.h
 * @brief  Image file loader
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup GFX
 *
 * @{
 */

#ifndef IMAGE_FILE_LOADER_H
#define IMAGE_FILE_LOADER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IImageLoader.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Image file loader.
 */
class ImageFileLoader : public IImageLoader
{
public:

    /**
     * Construct the image file loader.
     */
    ImageFileLoader() :
        IImageLoader(),
        m_fd()
    {
    }

    /**
     * Construct the image file loader by copy.
     *
     * @param[in] loader    Image file loader to copy.
     */
    ImageFileLoader(const ImageFileLoader& loader) :
        IImageLoader(),
        m_fd(loader.m_fd)
    {
    }

    /**
     * Destroy the image file loader.
     */
    ~ImageFileLoader() override
    {
        close();
    }

    /**
     * Assignment operator.
     *
     * @param[in] loader    Image file loader to copy.
     *
     * @return Reference to this loader.
     */
    ImageFileLoader& operator=(const ImageFileLoader& loader)
    {
        if (this != &loader)
        {
            close();

            m_fd = loader.m_fd;
        }

        return *this;
    }

    /**
     * Open an image file.
     *
     * @param[in] fs        Filesystem to use.
     * @param[in] fileName  Name of the image file.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool open(FS& fs, const String& fileName) final
    {
        bool isSuccessful = false;

        if (false == m_fd)
        {
            m_fd         = fs.open(fileName);

            isSuccessful = m_fd;
        }

        return isSuccessful;
    }

    /**
     * Close the image file.
     */
    void close() final
    {
        if (true == m_fd)
        {
            m_fd.close();
        }
    }

    /**
     * Read data from image.
     *
     * @param[in] buffer    Buffer to fill.
     * @param[in] size      Buffer size in bytes.
     *
     * @return If successful read, it will return true otherwise false.
     */
    bool read(void* buffer, size_t size) final
    {
        bool isSuccessful = false;

        if (true == m_fd)
        {
            uint8_t* u8Buffer = static_cast<uint8_t*>(buffer);

            if (size == m_fd.read(u8Buffer, size))
            {
                isSuccessful = true;
            }
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
        return m_fd.position();
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
        return m_fd.seek(position, mode);
    }

    /**
     * If file is opened, it will return true otherwise false.
     *
     * @return File status
     */
    operator bool() const final
    {
        return m_fd;
    }

private:

    File m_fd; /**< File descriptor */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* IMAGE_FILE_LOADER_H */

/** @} */
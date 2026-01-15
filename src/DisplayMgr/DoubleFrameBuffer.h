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
 * @file   DoubleFrameBuffer.h
 * @brief  Double frame buffer
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup DISPLAY_MGR
 *
 * @{
 */

#ifndef DOUBLE_FRAME_BUFFER_H
#define DOUBLE_FRAME_BUFFER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <YAGfxBitmap.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This class provides a double buffered framebuffer.
 * It contains two framebuffers, which can be used for double buffering.
 * The framebuffers memory is allocated dynamically.
 */
class DoubleFrameBuffer
{
public:

    /**
     * Construct the double framebuffer.
     */
    DoubleFrameBuffer() :
        m_framebuffers(),
        m_selectedIndex(0U)
    {
        /* Nothing to do */
    }

    /**
     * Destruct the double framebuffer.
     */
    ~DoubleFrameBuffer()
    {
        /* Nothing to do */
    }

    /**
     * Create framebuffers.
     *
     * @param[in] width     Width in pixels
     * @param[in] height    Height in pixels
     *
     * @return If successful, it will return true otherwise false.
     */
    bool create(uint16_t width, uint16_t height);

    /**
     * Release framebuffers.
     */
    void release();

    /**
     * Get the selected framebuffer.
     *
     * @return Selected framebuffer
     */
    YAGfxDynamicBitmap& getSelectedFramebuffer()
    {
        return m_framebuffers[m_selectedIndex];
    }

    /**
     * Select the next framebuffer.
     * This will switch between the two framebuffers.
     */
    void selectNextFramebuffer()
    {
        m_selectedIndex = (m_selectedIndex + 1) % FB_MAX;
    }

    /**
     * Get previous framebuffer.
     *
     * @return Previous framebuffer
     */
    YAGfxDynamicBitmap& getPreviousFramebuffer()
    {
        size_t previousIndex = (m_selectedIndex + 1) % FB_MAX;
        return m_framebuffers[previousIndex];
    }

private:

    /**
     * Max. number of frame buffers.
     */
    static const size_t FB_MAX = 2U;

    YAGfxDynamicBitmap  m_framebuffers[FB_MAX]; /**< Two framebuffers, which can be used for double buffering. */
    size_t              m_selectedIndex;        /**< Index of the selected framebuffer. */

    /**
     * Copy consturctor is not allowed.
     *
     * @param[in] other  Other instance, which to copy
     */
    DoubleFrameBuffer(const DoubleFrameBuffer& other)            = delete;

    /**
     * Assignment operator is not allowed.
     *
     * @param[in] other  Other instance, which to assign
     *
     * @return Reference to this instance
     */
    DoubleFrameBuffer& operator=(const DoubleFrameBuffer& other) = delete;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* DOUBLE_FRAME_BUFFER_H */

/** @} */
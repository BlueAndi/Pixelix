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
 * @brief  Distributed Display Protocol plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "DDPPlugin.h"

#include <Logging.h>
#include <Util.h>
#include <WiFi.h>

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

void DDPPlugin::start(uint16_t width, uint16_t height)
{
    String  manufacturer    = "BlueAndi & Friends"; /* Do-It-Yourself project */
    String  model           = "Pixelix";            /* Use project name */
    String  version         = "0.1.0";              /* From library.json */
    String  mac             = WiFi.macAddress();

    if (false == m_framebuffer.create(width, height))
    {
        LOG_ERROR("Failed to create framebuffer (%u x %u).", width, height);
    }
    else if (false == m_server.begin(manufacturer, model, version, mac))
    {
        LOG_ERROR("Failed to start DDP server.");
    }
    else
    {
        m_server.pause();
        m_server.registerDDPCallback(
            [this](DDPServer::Format format, uint32_t offset, uint8_t bitsPerPixel, uint8_t* payload, uint16_t payloadSize, bool isFinal)
            {
                this->onData(format, offset, bitsPerPixel, payload, payloadSize, isFinal);
            }
        );

        m_server.notifyUpState();
    }
}

void DDPPlugin::stop()
{
    m_server.notifyDownState();

    m_server.registerDDPCallback(nullptr);
    m_server.end();
    m_framebuffer.release();
}

void DDPPlugin::active(YAGfx& gfx)
{
    /* Clear display */
    gfx.fillScreen(ColorDef::BLACK);

    m_server.resume();
}

void DDPPlugin::inactive()
{
    m_server.pause();
}

void DDPPlugin::update(YAGfx& gfx)
{
    MutexGuard<Mutex> guard(m_mutex);

    if (true == m_isUpdated)
    {
        gfx.drawBitmap(0U, 0U, m_framebuffer);
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void DDPPlugin::onData(DDPServer::Format format, uint32_t offset, uint8_t bitsPerPixelElement, uint8_t* payload, uint16_t payloadSize, bool isFinal)
{
    MutexGuard<Mutex> guard(m_mutex);

    /* xlights <= v202301 sends FORMAT_UNDEFINED with 1-bit per pixel element which is
     * necessary to be interpreted as FORMAT_RGB with 8-bit per pixel element.
     */
    if ((DDPServer::FORMAT_UNDEFINED == format) &&
        (1U == bitsPerPixelElement))
    {
        /* Workaround! */
        format              = DDPServer::FORMAT_RGB;
        bitsPerPixelElement = 8U;
    }
    /* If the format is FORMAT_UNDEFINED and the bit per pixel element is undefined,
     * we assume its 8-bit per pixel element.
     */
    else if ((DDPServer::FORMAT_UNDEFINED == format) &&
             (0U == bitsPerPixelElement))
    {
        format              = DDPServer::FORMAT_RGB;
        bitsPerPixelElement = 8U;
    }
    else
    {
        ;
    }

    if ((nullptr != payload) &&
        (DDPServer::FORMAT_RGB == format) &&
        (8U == bitsPerPixelElement))
    {
        uint16_t    srcIdx          = 0U;
        int16_t     x               = (offset % m_framebuffer.getWidth());
        int16_t     y               = (offset / m_framebuffer.getWidth());
        uint8_t     bytePerPixel    = (bitsPerPixelElement * 3U) / 8U; /* RGB = 3 base colors */

        while((payloadSize > srcIdx) && (m_framebuffer.getHeight() > y))
        {
            Color       color;
            uint32_t    colorCode = 0U;
            uint8_t     byteIdx = 0U;

            while((bytePerPixel > byteIdx) && (payloadSize > srcIdx))
            {
                colorCode <<= 8U;
                colorCode |= payload[srcIdx];
                
                ++srcIdx;                
                ++byteIdx;
            }

            color.set(colorCode);

            m_framebuffer.drawPixel(x, y, color);

            ++x;
            if (m_framebuffer.getWidth() <= x)
            {
                x = 0;

                ++y;
            }
        }

        m_isUpdated = isFinal;
    }
    else
    {
        LOG_WARNING("Unsupported DDP frame with format %d and bits per pixel element %u.", format, bitsPerPixelElement);
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

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
 * @file   SDLInterface.cpp
 * @brief  SDL Interface for native LCD Simulation
 * @author Norbert Schulz <github@schulznorbert.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <SDL3/SDL.h>

#include "SDLInterface.h"
#include "Logging.h"

#include <array>
/******************************************************************************
 * Macros
 *****************************************************************************/

static const int32_t WINDOW_DEFAULT_WIDTH     = 1024U; /**< Default window width in pixels. */
static const int32_t WINDOW_DEFAULT_HEIGHT_8  = 240U;  /**< Default window height in pixels. */
static const int32_t WINDOW_DEFAULT_HEIGHT_16 = 360U;  /**< Default window height in pixels. */
static const int32_t WINDOW_DEFAULT_HEIGHT_32 = 512U;  /**< Default window height in pixels. */
static const int32_t WINDOW_DEFAULT_HEIGHT_64 = 1024U; /**< Default window height in pixels. */

static const int32_t WINDOW_BUTTONBAR_HEIGHT  = 100U; /**< Vertical space for buttons */

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/** Image entry for the SDL interface. */
struct ImageEntry
{
    SDLInterface::ImageId id;             /**< Image identifier. */
    const char*           path = nullptr; /**< Path to the image file. */
};
/******************************************************************************
 * Prototypes
 *****************************************************************************/

/**
 * @brief   Gets the default window height based on the number of pixel rows.
 * @param   pixelrows The number of pixel rows.
 * @return  The default window height in pixels.
 */
static uint32_t getDefaultWindowHeight(uint32_t pixelrows);

/******************************************************************************
 * Local Variables

 *****************************************************************************/
/**
 * @brief   Image entry for the SDL interface.
 * @details Contains the path and surface of a registered image.
 */
std::array<ImageEntry, static_cast<std::size_t>(SDLInterface::ImageId::IMG_ID_COUNT)> gImages = {
    { { SDLInterface::ImageId::IMG_ID_WINDOW_ICON, "data/favicon.png" },
        { SDLInterface::ImageId::IMG_ID_ABOUT_LOGO, "data/images/LogoSmall.png" } }
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

SDLInterface::~SDLInterface()
{
    shutdown();
}

bool SDLInterface::initialize(int width, int height)
{
    if (m_sdl_initialized)
    {
        return m_window != nullptr && m_renderer != nullptr;
    }

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        LOG_WARNING("SDL_Init failed: %s", SDL_GetError());
        return false;
    }
    m_sdl_initialized = true;

    m_window          = SDL_CreateWindow(
        "Pixelix LED Grid Simulation",
        WINDOW_DEFAULT_WIDTH, /* Width is same for all layouts.*/
        getDefaultWindowHeight(height),
        SDL_WINDOW_RESIZABLE);
    if (m_window == nullptr)
    {
        LOG_WARNING("SDL_CreateWindow failed: %s", SDL_GetError());
        shutdown();
        return false;
    }

    /* Load all images from registry to SDL surfaces.*/
    for (auto imgEntry : gImages)
    {
        auto surface = SDL_LoadPNG(imgEntry.path);
        if (surface == nullptr)
        {
            LOG_WARNING("SDL_LoadPNG failed for %s: %s", imgEntry.path, SDL_GetError());
        }
        else
        {
            m_images[imgEntry.id] = surface;

            if (imgEntry.id == SDLInterface::ImageId::IMG_ID_WINDOW_ICON)
            {
                /* Favicon is used as the window icon. */
                SDL_SetWindowIcon(m_window, surface);
            }
        }
    }

    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (m_renderer == nullptr)
    {
        LOG_WARNING("SDL_CreateRenderer failed: %s", SDL_GetError());
        shutdown();
        return false;
    }

    SDL_SetRenderVSync(m_renderer, 1);
    return true;
}

void SDLInterface::shutdown()
{
    for (auto image : m_images)
    {
        if (image.second != nullptr)
        {
            SDL_DestroySurface(image.second);
        }
    }
    m_images.clear();

    if (m_renderer != nullptr)
    {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }

    if (m_window != nullptr)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    if (m_sdl_initialized)
    {
        SDL_Quit();
        m_sdl_initialized = false;
    }
}

SDL_Surface* SDLInterface::getImageSurface(ImageId id) const
{
    SDL_Surface* surface = nullptr;

    auto         it      = m_images.find(id);
    if (it != m_images.end())
    {
        surface = it->second;
    }

    return surface;
}

void SDLInterface::beginUpdate()
{
    if (!m_sdl_initialized)
    {
        return;
    }

    SDL_SetRenderDrawColor(m_renderer, 30, 30, 30, 255);
    SDL_RenderClear(m_renderer);
}

void SDLInterface::finishUpdate()
{
    if (!m_sdl_initialized)
    {
        return;
    }

    SDL_RenderPresent(m_renderer);
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

static uint32_t getDefaultWindowHeight(uint32_t pixelrows)
{
    uint32_t height = 0U;

    switch (pixelrows)
    {
    case 8U:
        height = WINDOW_DEFAULT_HEIGHT_8;
        break;

    case 16U:
        height = WINDOW_DEFAULT_HEIGHT_16;
        break;

    case 32U:
        height = WINDOW_DEFAULT_HEIGHT_32;
        break;

    case 64U:
        height = WINDOW_DEFAULT_HEIGHT_64;
        break;

    default:
        height = WINDOW_DEFAULT_HEIGHT_8;
        break;
    }

    return height + WINDOW_BUTTONBAR_HEIGHT;
}

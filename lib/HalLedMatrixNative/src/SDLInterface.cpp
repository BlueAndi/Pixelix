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

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/**
 * @brief   Image entry for the SDL interface.
 * @details Contains the path and surface of a registered image.
 */
static const std::array<ImageEntry, static_cast<std::size_t>(SDLInterface::ImageId::IMG_ID_COUNT)> gImages = {
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
    bool result = false;

    if (true == m_sdl_initialized)
    {
        result = ((nullptr != m_window) && (nullptr != m_renderer));
    }
    else
    {
        if (false == SDL_Init(SDL_INIT_VIDEO))
        {
            LOG_WARNING("SDL_Init failed: %s", SDL_GetError());
            result = false;
        }
        else
        {
            m_sdl_initialized = true;

            m_window          = SDL_CreateWindow(
                "Pixelix LED Grid Simulation",
                static_cast<int>(width),
                static_cast<int>(height),
                SDL_WINDOW_RESIZABLE);

            if (nullptr == m_window)
            {
                LOG_WARNING("SDL_CreateWindow failed: %s", SDL_GetError());
                shutdown();
                result = false;
            }
            else
            {
                /* Load all images from registry to SDL surfaces.*/
                for (const ImageEntry& imgEntry : gImages)
                {
                    auto surface = SDL_LoadPNG(imgEntry.path);
                    if (nullptr == surface)
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
                if (nullptr == m_renderer)
                {
                    LOG_WARNING("SDL_CreateRenderer failed: %s", SDL_GetError());
                    shutdown();
                    result = false;
                }
                else
                {
                    result = true;
                    SDL_SetRenderVSync(m_renderer, 1);
                }
            }
        }
    }

    return result;
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

    if (nullptr != m_renderer)
    {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }

    if (nullptr != m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    if (false != m_sdl_initialized)
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

void SDLInterface::beginUpdate() const
{
    if (true == m_sdl_initialized)
    {
        SDL_SetRenderDrawColor(m_renderer, 30, 30, 30, 255);
        SDL_RenderClear(m_renderer);
    }
}

void SDLInterface::finishUpdate() const
{
    if (true == m_sdl_initialized)
    {
        SDL_RenderPresent(m_renderer);
    }
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

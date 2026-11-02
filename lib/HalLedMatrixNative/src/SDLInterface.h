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
 * @file   SDLInterface.h
 * @brief  SDL Interface for native LCD Simulation
 * @author Norbert Schulz <github@schulznorbert.de>
 *
 * @addtogroup LED Grid Simulator
 * @{
 */

#pragma once

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <SDL3/SDL.h>

#include <map>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * @brief Manages the SDL video subsystem
 * @details Initializes SDL resources on request and releases them during
 *          shutdown or destruction. This class cannot be copied or copy-assigned.
 */
class SDLInterface
{
public:

    /** Identifies an image loaded and owned by this interface. */
    enum class ImageId
    {
        IMG_ID_WINDOW_ICON, /**< Window icon image. */
        IMG_ID_ABOUT_LOGO,  /**< About dialog logo image. */
        IMG_ID_COUNT
    };

    /**
     * @brief Constructs an SDL interface without initializing SDL.
     */
    SDLInterface() = default;

    /**
     * @brief Shuts down the interface and releases its SDL resources.
     */
    virtual ~SDLInterface();

    /**
     * @brief Initializes SDL and creates the window and renderer.
     * @param[in] width The window width in screen pixels.
     * @param[in] height The window height in screen pixels.
     * @return true if initialization succeeds; otherwise, false.
     */
    bool initialize(int width, int height);

    /**
     * @brief Releases the renderer, window, and SDL resources.
     * @details Safe to call more than once or after a failed initialization.
     */
    void shutdown();

    /**
     * @brief Begins the SDL UI update cycle.
     */
    void beginUpdate() const;

    /**
     * @brief Finishes the SDL UI update cycle.
     */
    void finishUpdate() const;

    /**
     * @brief Gets the SDL window managed by this interface.
     * @return A non-owning pointer to the window, or nullptr if unavailable.
     */
    SDL_Window* getWindow() const
    {
        return m_window;
    }

    /**
     * @brief Gets the SDL renderer managed by this interface.
     * @return A non-owning pointer to the renderer, or nullptr if unavailable.
     */
    SDL_Renderer* getRenderer() const
    {
        return m_renderer;
    }

    /**
     * @brief Gets a registered image surface.
     * @param[in] id Image identifier.
     * @return A non-owning pointer to the image surface, or nullptr if unavailable.
     */
    SDL_Surface* getImageSurface(ImageId id) const;

private:

    SDL_Window*                     m_window          = nullptr;
    SDL_Renderer*                   m_renderer        = nullptr;

    bool                            m_sdl_initialized = false;

    std::map<ImageId, SDL_Surface*> m_images; /**< Map of registered images. */

    /** @brief Copy construction is disabled because this class owns SDL resources. */
    SDLInterface(const SDLInterface&)            = delete;

    /** @brief Copy assignment is disabled because this class owns SDL resources. */
    SDLInterface& operator=(const SDLInterface&) = delete;
};

/** @} */
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
 * @file   LedGridSim.h
 * @brief  ImGui/SDL3 Interface for native LED Grid Simulation
 * @author Norbert Schulz <github@schulznorbert.de>
 *
 * @addtogroup LED Grid Simulator
 *
 * @{
 */
#pragma once

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <stdint.h>

class SDLInterface;

/**
 * @brief Initializes and manages the Dear ImGui SDL3 backends.
 * @details The referenced SDLInterface must outlive this object and must have
 *          successfully initialized its window and renderer before construction.
 */
class LedGridSim
{
public:

    /**
     * @brief Creates an ImGui context and initializes its SDL3 backends.
     * @param sdl_interface Initialized SDL interface providing the window and renderer.
     */
    explicit LedGridSim();

    /**
     * @brief Shuts down the interface and releases IMGUI resources.
     */
    ~LedGridSim();

    /**
     * @brief Initializes SDL and creates the window and renderer.
     * @param width The width of the Led Matrix in leds.
     * @param height The height of the Led Matrix in pixels.
     * @return true if initialization succeeds; otherwise, false.
     */
    bool initialize(int width, int height);

    /**
     * @brief Processes all pending SDL events and forwards them to ImGui.
     * @return true if processing should continue; false if a quit event was received.
     */
    bool dispatchEvents();

    /**
     * @brief Updates the ImGui UI.
     */
    void update(const void* frameBuffer);

    LedGridSim(const LedGridSim&)            = delete;
    LedGridSim& operator=(const LedGridSim&) = delete;

    /**
     * @brief Reports whether the context and both SDL3 backends initialized successfully.
     * @return true if this interface is ready for ImGui use; otherwise, false.
     */
    bool isInitialized() const;

protected:

    /**
     * @brief Begins a fullscreen ImGui window that covers the entire viewport.
     */
    void beginFullscreenWindow();

    /**
     * @brief Renders a button bar at the bottom of the ImGui window.
     */
    void renderButtonBar();

    /**
     * @brief Renders the main display area of the ImGui window.
     */
    void renderDisplay(const void* framebuffer);

    /**
     * @brief Renders the menu bar at the top of the ImGui window.
     */
    void renderMenuBar();

private:

    SDLInterface* m_sdl_interface;                        /**<Underlying SDL interface. */
    bool          m_context_created              = false; /**< Tracks whether this object created an ImGui context. */
    bool          m_platform_backend_initialized = false; /**< Tracks whether the SDL3 platform backend initialized. */
    bool          m_renderer_backend_initialized = false; /**< Tracks whether the SDL3 renderer backend initialized. */

    int16_t       m_framebuffer[32 * 8]; /**< Framebuffer for the LCD display. */
};

/** @} */
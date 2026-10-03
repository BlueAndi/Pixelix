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
 * @file   LedGridSim.cpp
 * @brief  ImGui/SDL3 Interface for native LED Grid Simulation
 * @author Norbert Schulz <github@schulznorbert.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "LedGridSim.h"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include "SDLInterface.h"

#include "Board.h"
#include "Version.h"

#include <array>
#include <stdint.h>
#include <string>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/**
 * Describes a single button in the simulator button bar.
 */
struct SimulatedButton
{
    const char* label;    /**< Label shown on the button. */
    ButtonId    id;       /**< Pixelix Id of the related button. */
    bool        sameLine; /**< Put this button on same line if true. "*/
};

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/**
 * @brief Adjusts a RGB channel value based on the specified brightness.
 *
 * The brightness is affecting the upper 50% of the channel value range.
 * A brightness of 0 means 50%, while a brightness of 255 means the 100%.
 * The simulated display is otherwise getting too dark at low brightness values.
 *
 * @param channel The original RGB channel value [0; 255].
 * @param brightness The brightness value [0; 255].
 * @return The adjusted RGB channel value [0; 255].
 */
static uint8_t adjustRgbChannel(uint8_t channel, uint8_t brightness);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Simulated Hardware buttons shown in the button bar, from left to right. */
static const std::array<SimulatedButton, 3U> gSimButtons = {
    { { "Left", BUTTON_ID_LEFT, false },
        { "Ok", BUTTON_ID_OK, true },
        { "Right", BUTTON_ID_RIGHT, true } }
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/
LedGridSim::LedGridSim() :
    m_sdl_interface(new SDLInterface())
{
}

bool LedGridSim::initialize(int width, int height)
{
    if (!m_sdl_interface->initialize(width, height))
    {
        SDL_Log("SDL initialization failed");
        return false;
    }

    SDL_Window*   window   = m_sdl_interface->getWindow();
    SDL_Renderer* renderer = m_sdl_interface->getRenderer();
    if (window == nullptr || renderer == nullptr)
    {
        SDL_Log("Cannot initialize ImGui: SDL window or renderer is unavailable");
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    m_context_created = true;
    ImGui::StyleColorsDark();

    if (!ImGui_ImplSDL3_InitForSDLRenderer(window, renderer))
    {
        SDL_Log("ImGui SDL3 platform backend initialization failed");
        return false;
    }
    m_platform_backend_initialized = true;

    if (!ImGui_ImplSDLRenderer3_Init(renderer))
    {
        SDL_Log("ImGui SDL renderer backend initialization failed");
        return false;
    }
    m_renderer_backend_initialized = true;

    SDL_Surface* logoSurface       = m_sdl_interface->getImageSurface(SDLInterface::ImageId::IMG_ID_ABOUT_LOGO);
    if (logoSurface != nullptr)
    {
        m_logo_texture = SDL_CreateTextureFromSurface(renderer, logoSurface);
        if (m_logo_texture == nullptr)
        {
            SDL_Log("SDL_CreateTextureFromSurface failed for the About logo: %s", SDL_GetError());
        }
    }

    return m_renderer_backend_initialized;
}

LedGridSim::~LedGridSim()
{
    if (m_logo_texture != nullptr)
    {
        SDL_DestroyTexture(m_logo_texture);
        m_logo_texture = nullptr;
    }

    if (m_renderer_backend_initialized)
    {
        ImGui_ImplSDLRenderer3_Shutdown();
    }

    if (m_platform_backend_initialized)
    {
        ImGui_ImplSDL3_Shutdown();
    }

    if (m_context_created)
    {
        ImGui::DestroyContext();
    }

    m_sdl_interface->shutdown();
    delete m_sdl_interface;
    m_sdl_interface = nullptr;
}

bool LedGridSim::isInitialized() const
{
    return m_context_created && m_platform_backend_initialized && m_renderer_backend_initialized;
}

bool LedGridSim::dispatchEvents()
{
    bool continueRunning = true;

    if (!isInitialized())
    {
        return false;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL3_ProcessEvent(&event);
        if (event.type == SDL_EVENT_QUIT)
        {
            continueRunning = false;
        }
    }
    return continueRunning;
}

void LedGridSim::update(const YAGfxBitmap& bitmap)
{
    if (!isInitialized())
    {
        return;
    }

    m_sdl_interface->beginUpdate();

    // --- ImGui frame ---
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    this->beginFullscreenWindow();

    this->renderMenuBar();
    this->renderDisplay(bitmap);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    this->renderButtonBar();

    ImGui::End();

    // --- Render ---
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_sdl_interface->getRenderer());

    m_sdl_interface->finishUpdate();
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/


void LedGridSim::beginFullscreenWindow()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_MenuBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

    ImGui::Begin("PixelixSimulatorRoot", nullptr, flags);

    ImGui::PopStyleVar(3);
}

void LedGridSim::renderButtonBar()
{
    const float buttonWidth  = 100.0f;
    const float buttonHeight = 30.0f;
    const float spacing      = ImGui::GetStyle().ItemSpacing.x;

    const float buttonCount  = static_cast<float>(gSimButtons.size());

    float       totalWidth   = (buttonWidth * buttonCount) + (spacing * (buttonCount - 1.0f));
    float       availWidth   = ImGui::GetContentRegionAvail().x;
    float       offsetX      = (availWidth - totalWidth) * 0.5f;

    ButtonDrv*  buttonDrv    = dynamic_cast<ButtonDrv*>(&Board::getInstance().getButtonDrv());

    if (offsetX > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

    for (const SimulatedButton& button : gSimButtons)
    {
        if (button.sameLine)
        {
            ImGui::SameLine();
        }

        ImGui::Button(button.label, ImVec2(buttonWidth, buttonHeight));

        if (ImGui::IsItemActivated())
        {
            buttonDrv->updateButton(button.id, BUTTON_STATE_PRESSED);
        }
        if ((ImGui::IsItemDeactivated()))
        {
            buttonDrv->updateButton(button.id, BUTTON_STATE_RELEASED);
        }
    }
}
void LedGridSim::renderDisplay(const YAGfxBitmap& bitmap)
{
    ImVec2 avail          = ImGui::GetContentRegionAvail();

    // Reserve space at the bottom for the button bar (computed by caller ideally,
    // but here we just reserve a fixed height for simplicity)
    float buttonBarHeight = ImGui::GetFrameHeightWithSpacing() + 20.0f;
    float availHeight     = avail.y - buttonBarHeight;
    if (availHeight < 10.0f)
        availHeight = 10.0f;

    float aspectRatio  = (float)32 / (float)8;

    // Fit within (avail.x, availHeight) while preserving aspect ratio
    float targetWidth  = avail.x;
    float targetHeight = targetWidth / aspectRatio;

    if (targetHeight > availHeight)
    {
        targetHeight = availHeight;
        targetWidth  = targetHeight * aspectRatio;
    }

    // Center horizontally
    float offsetX = (avail.x - targetWidth) * 0.5f;
    if (offsetX > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

    const ImVec2 displayPos = ImGui::GetCursorScreenPos();
    const float  cellWidth  = targetWidth / 32.0f;
    const float  cellHeight = targetHeight / 8.0f;
    const float  cellInset  = (cellWidth < cellHeight ? cellWidth : cellHeight) * 0.12f;
    ImDrawList*  drawList   = ImGui::GetWindowDrawList();

    drawList->AddRectFilled(
        displayPos,
        ImVec2(displayPos.x + targetWidth, displayPos.y + targetHeight),
        IM_COL32(24, 27, 26, 255));

    if (m_power)
    {
        uint16_t bitmap_width  = bitmap.getWidth();
        uint16_t bitmap_height = bitmap.getHeight();

        if (CONFIG_LED_MATRIX_WIDTH < bitmap_width)
        {
            bitmap_width = CONFIG_LED_MATRIX_WIDTH;
        }

        if (CONFIG_LED_MATRIX_HEIGHT < bitmap_height)
        {
            bitmap_height = CONFIG_LED_MATRIX_HEIGHT;
        }

        for (int y = 0; y < bitmap_height; ++y)
        {
            for (int x = 0; x < bitmap_width; ++x)
            {
                const ImVec2 cellMin(displayPos.x + x * cellWidth, displayPos.y + y * cellHeight);
                const ImVec2 cellMax(cellMin.x + cellWidth, cellMin.y + cellHeight);

                const Color  pixel = bitmap.getColor(x, y);
                const ImU32  color = IM_COL32(
                    adjustRgbChannel(pixel.getRed(), m_brightness),
                    adjustRgbChannel(pixel.getGreen(), m_brightness),
                    adjustRgbChannel(pixel.getBlue(), m_brightness),
                    SDL_ALPHA_OPAQUE);

                drawList->AddRectFilled(
                    ImVec2(cellMin.x + cellInset, cellMin.y + cellInset),
                    ImVec2(cellMax.x - cellInset, cellMax.y - cellInset),
                    color,
                    5.0f);
            }
        }
    }
    /* Save the space of the pixel area from placing other widgets */
    ImGui::Dummy(ImVec2(targetWidth, targetHeight));
}

void LedGridSim::renderMenuBar()
{
    bool showAboutDialog = false;

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("About"))
        {
            showAboutDialog = true;
            ImGui::EndMenu();
        }

        ImGui::SameLine();
        std::string webpageUrl("http://localhost:");
        webpageUrl += std::to_string(CONFIG_WEBSERVER_PORT);

        ImGui::TextLinkOpenURL("Open Web Page", webpageUrl.c_str());

        /* Reserve space and push "Help" to the right edge */
        float helpWidth = ImGui::CalcTextSize("FPS: 100.00").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        ImGui::SameLine(ImGui::GetWindowWidth() - helpWidth - 10.0f);

        ImGui::Text("FPS: %.1f", this->getPower() ? ImGui::GetIO().Framerate : 0.0f);

        ImGui::EndMenuBar();
    }

    if (showAboutDialog)
    {
        ImGui::OpenPopup("About");
        showAboutDialog = false; // only trigger once
    }

    // Centered modal popup
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (m_logo_texture != nullptr)
        {
            const ImTextureID textureId   = static_cast<ImTextureID>(reinterpret_cast<intptr_t>(m_logo_texture));
            const float       logoOffsetX = (ImGui::GetContentRegionAvail().x - m_logo_texture->w) * 0.5f;
            /* Center the logo horizontally */
            if (logoOffsetX > 0.0f)
            {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + logoOffsetX);
            }
            ImGui::Image(ImTextureRef(textureId), ImVec2(m_logo_texture->w, m_logo_texture->h));
        }
        else
        {
            ImGui::Text(".:PIXELIX:.");
        }
        ImGui::Separator();


        ImGui::Text("Home:    ");
        ImGui::SameLine();
        ImGui::TextLinkOpenURL("https://github.com/BlueAndi/Pixelix");

        ImGui::Text("Version:  %s", Version::getSoftwareVersion());
        ImGui::Text("Branch:   %s", Version::getSoftwareBranchName());
        ImGui::Text("Revision: %s", Version::getSoftwareRevisionShort());
        ImGui::Separator();
        ImGui::Text("Copyright (c) 2019 - 2026 Andreas Merkle");

        ImGui::Spacing();

        const float closeButtonWidth   = 120.0f;
        const float closeButtonOffsetX = (ImGui::GetContentRegionAvail().x - closeButtonWidth) * 0.5f;
        if (closeButtonOffsetX > 0.0f)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + closeButtonOffsetX);
        }

        if (ImGui::Button("Close", ImVec2(closeButtonWidth, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

static uint8_t adjustRgbChannel(uint8_t channel, uint8_t brightness)
{
    const uint32_t MAX_CHANNEL_VALUE = 255U;
    const uint32_t scaling           = MAX_CHANNEL_VALUE + static_cast<uint32_t>(brightness);

    return static_cast<uint8_t>(
        (static_cast<uint32_t>(channel) * scaling + MAX_CHANNEL_VALUE) / (2 * MAX_CHANNEL_VALUE));
};
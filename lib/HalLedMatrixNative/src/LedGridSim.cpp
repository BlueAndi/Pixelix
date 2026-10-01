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

    return m_renderer_backend_initialized;
}

LedGridSim::~LedGridSim()
{
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

void LedGridSim::update(const void* frameBuffer)
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
    this->renderDisplay(frameBuffer);

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
            printf("Button '%s' pressed\n", button.label);
        }
        if ((ImGui::IsItemDeactivated()))
        {
            buttonDrv->updateButton(button.id, BUTTON_STATE_RELEASED);
            printf("Button '%s' released\n", button.label);
        }
    }
}
void LedGridSim::renderDisplay(const void* framebuffer)
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

    for (int y = 0; y < 8; ++y)
    {
        for (int x = 0; x < 32; ++x)
        {
            const ImVec2   cellMin(displayPos.x + x * cellWidth, displayPos.y + y * cellHeight);
            const ImVec2   cellMax(cellMin.x + cellWidth, cellMin.y + cellHeight);

            const uint32_t pixel = reinterpret_cast<const uint32_t*>(framebuffer)[y * 32 + x];
            const uint8_t  red   = (pixel >> 16) & 0xFF;
            const uint8_t  green = (pixel >> 8) & 0xFF;
            const uint8_t  blue  = pixel & 0xFF;
            const ImU32    color = IM_COL32(red, green, blue, 255);

            drawList->AddRectFilled(
                ImVec2(cellMin.x + cellInset, cellMin.y + cellInset),
                ImVec2(cellMax.x - cellInset, cellMax.y - cellInset),
                color,
                5.0f);
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
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

        /* Reserve space and push "Help" to the right edge */
        float helpWidth = ImGui::CalcTextSize("Help").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        ImGui::SameLine(ImGui::GetWindowWidth() - helpWidth - 10.0f);

        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About"))
            {
                showAboutDialog = true;
            }
            ImGui::EndMenu();
        }
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
        ImGui::Text("Pixelix Simulation");
        ImGui::Separator();
        ImGui::Text("Version %s", Version::getSoftwareVersion());
        ImGui::Text("Branch: %s", Version::getSoftwareBranchName());
        ImGui::Text("Revision: %s", Version::getSoftwareRevisionShort());

        ImGui::Spacing();

        if (ImGui::Button("Close", ImVec2(120, 0)))
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
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Norbert Schulz <github@schulznorbert.de>

/*******************************************************************************
    DESCRIPTION
*******************************************************************************/
/**
 * @file   LedGridSim.cpp
 * @brief  ImGui Interface for native LED Grid Simulation
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
#include "Version.h"

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

    float       totalWidth   = (buttonWidth * 3) + (spacing * 2);
    float       availWidth   = ImGui::GetContentRegionAvail().x;
    float       offsetX      = (availWidth - totalWidth) * 0.5f;

    if (offsetX > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

    if (ImGui::Button("Left", ImVec2(buttonWidth, buttonHeight)))
    {
        // handle left button press
    }
    ImGui::SameLine();
    if (ImGui::Button("Ok", ImVec2(buttonWidth, buttonHeight)))
    {
        // handle middle button press
    }
    ImGui::SameLine();
    if (ImGui::Button("Right", ImVec2(buttonWidth, buttonHeight)))
    {
        // handle right button press
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
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
#include "Logging.h"
#include "Version.h"

#include <array>
#include <stdint.h>
#include <string>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

static const uint16_t MAX_GRID_RESOLUTION       = 64U;   /**< Maximum supported Led dimension. */
static const uint32_t DEFAULT_WINDOW_WIDTH      = 1024U; /**< Default main window width. */

static const uint32_t RESERVED_MENUBAR_HEIGHT   = 28U; /**< space for title bar and menu bar. */
static const uint32_t RESERVED_BUTTONBAR_HEIGHT = 50U; /**< space for button bar on bottom. */

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
 * @brief Adjusts an RGB channel value based on the specified brightness.
 *
 * The brightness is affecting the upper 50% of the channel value range.
 * A brightness of 0 means 50%, while a brightness of 255 means the 100%.
 * The simulated display is otherwise getting too dark at low brightness values.
 *
 * @param[in] channel The original RGB channel value in the range [0; 255].
 * @param[in] brightness The brightness value in the range [0; 255].
 * @return The adjusted RGB channel value in the range [0; 255].
 */
static uint8_t adjustRgbChannel(uint8_t channel, uint8_t brightness);

/**
 * @brief   Calculate the default main window height based on LED configuration.
 *
 * @param[in] windowWidth The desired window width in screen pixels.
 * @param[in] ledsX The simulated pixel grid X resolution.
 * @param[in] ledsY The simulated pixel grid Y resolution.

 * @return  The default window height in screen pixels.
 */
static uint32_t getDefaultWindowHeight(uint32_t windowWidth, uint32_t ledsX, uint32_t ledsY);

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
    m_sdlInterface()
{
}

LedGridSim::~LedGridSim()
{
    shutdown();
}

bool LedGridSim::initialize(uint16_t width, uint16_t height)
{
    bool result = false;

    if ((0U == height) || (0U == width) || (MAX_GRID_RESOLUTION < height) || (MAX_GRID_RESOLUTION < width))
    {
        LOG_WARNING("Unexpected LED grid dimensions %d:%d. Falling back to 32:8", width, height);
        m_width  = 32U;
        m_height = 8U;
    }
    else
    {
        m_width  = width;
        m_height = height;
    }
    m_aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);

    if (false == m_sdlInterface.initialize(
                     static_cast<int>(DEFAULT_WINDOW_WIDTH),
                     static_cast<int>(getDefaultWindowHeight(DEFAULT_WINDOW_WIDTH, m_width, m_height))))
    {
        LOG_WARNING("SDL initialization failed");
    }
    else
    {
        SDL_Window*   window   = m_sdlInterface.getWindow();
        SDL_Renderer* renderer = m_sdlInterface.getRenderer();
        if ((nullptr == window) || (nullptr == renderer))
        {
            LOG_WARNING("Cannot initialize ImGui: SDL window or renderer is unavailable");
        }
        else
        {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            m_contextCreated = true;
            ImGui::StyleColorsDark();

            if (false == ImGui_ImplSDL3_InitForSDLRenderer(window, renderer))
            {
                LOG_WARNING("ImGui SDL3 platform backend initialization failed");
                shutdown();
            }
            else
            {
                m_platformBackendInitialized = true;

                if (false == ImGui_ImplSDLRenderer3_Init(renderer))
                {
                    LOG_WARNING("ImGui SDL renderer backend initialization failed");
                    shutdown();
                }
                else
                {
                    m_rendererBackendInitialized = true;

                    SDL_Surface* logoSurface     = m_sdlInterface.getImageSurface(SDLInterface::ImageId::IMG_ID_ABOUT_LOGO);
                    if (nullptr != logoSurface)
                    {
                        m_logoTexture = SDL_CreateTextureFromSurface(renderer, logoSurface);
                        if (nullptr == m_logoTexture)
                        {
                            LOG_WARNING("SDL_CreateTextureFromSurface failed for the About logo: %s", SDL_GetError());
                            /* No shutdown here, we just don't have an app icon. */
                        }
                    }

                    m_buttonDrv = dynamic_cast<ButtonDrv*>(&Board::getInstance().getButtonDrv());
                    if (nullptr == m_buttonDrv)
                    {
                        LOG_ERROR("Unexpected ButtonDrv class. Buttons will not work.");
                    }
                    result = true;
                }
            }
        }
    }
    return result;
}

void LedGridSim::shutdown()
{
    if (nullptr != m_logoTexture)
    {
        SDL_DestroyTexture(m_logoTexture);
        m_logoTexture = nullptr;
    }

    if (true == m_rendererBackendInitialized)
    {
        ImGui_ImplSDLRenderer3_Shutdown();
        m_rendererBackendInitialized = false;
    }

    if (true == m_platformBackendInitialized)
    {
        ImGui_ImplSDL3_Shutdown();
        m_platformBackendInitialized = false;
    }

    if (true == m_contextCreated)
    {
        ImGui::DestroyContext();
        m_contextCreated = false;
    }

    m_sdlInterface.shutdown();
}

bool LedGridSim::isInitialized() const
{
    return m_contextCreated && m_platformBackendInitialized && m_rendererBackendInitialized;
}

bool LedGridSim::dispatchEvents() const
{
    bool continueRunning = true;

    if (true == isInitialized())
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (SDL_EVENT_QUIT == event.type)
            {
                continueRunning = false;
            }
        }
    }
    else
    {
        continueRunning = false;
    }

    return continueRunning;
}

/**
 * @brief Updates the ImGui UI.
 * @param[in] bitmap The bitmap to display in the ImGui LED grid window.
 */
void LedGridSim::update(const YAGfxBitmap& bitmap) const
{
    if (true == isInitialized())
    {
        m_sdlInterface.beginUpdate();

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
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_sdlInterface.getRenderer());

        m_sdlInterface.finishUpdate();
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/


void LedGridSim::beginFullscreenWindow() const
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

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0F);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0F);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

    ImGui::Begin("PixelixSimulatorRoot", nullptr, flags);

    ImGui::PopStyleVar(3);
}

void LedGridSim::renderButtonBar() const
{
    const float buttonWidth  = 100.0F;
    const float buttonHeight = 30.0F;
    const float spacing      = ImGui::GetStyle().ItemSpacing.x;

    const float buttonCount  = static_cast<float>(gSimButtons.size());

    float       totalWidth   = (buttonWidth * buttonCount) + (spacing * (buttonCount - 1.0F));
    float       availWidth   = ImGui::GetContentRegionAvail().x;
    float       offsetX      = (availWidth - totalWidth) * 0.5F;

    if (nullptr != m_buttonDrv)
    {
        if (0.0F < offsetX)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
        }

        for (const SimulatedButton& button : gSimButtons)
        {
            if (true == button.sameLine)
            {
                ImGui::SameLine();
            }

            ImGui::Button(button.label, ImVec2(buttonWidth, buttonHeight));

            if (true == ImGui::IsItemActivated())
            {
                m_buttonDrv->updateButton(button.id, BUTTON_STATE_PRESSED);
            }
            if ((true == ImGui::IsItemDeactivated()))
            {
                m_buttonDrv->updateButton(button.id, BUTTON_STATE_RELEASED);
            }
        }
    }
}


void LedGridSim::renderDisplay(const YAGfxBitmap& bitmap) const
{
    ImVec2 avail          = ImGui::GetContentRegionAvail();

    // Reserve space at the bottom for the button bar (computed by caller ideally,
    // but here we just reserve a fixed height for simplicity)
    float buttonBarHeight = ImGui::GetFrameHeightWithSpacing() + 20.0F;
    float availHeight     = avail.y - buttonBarHeight;
    if (10.0F > availHeight)
        availHeight = 10.0F;


    // Fit within (avail.x, availHeight) while preserving aspect ratio
    float targetWidth  = avail.x;
    float targetHeight = targetWidth / m_aspectRatio;

    if (targetHeight > availHeight)
    {
        targetHeight = availHeight;
        targetWidth  = targetHeight * m_aspectRatio;
    }

    // Center horizontally
    float offsetX = (avail.x - targetWidth) * 0.5F;
    if (0.0F < offsetX)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

    const ImVec2 displayPos = ImGui::GetCursorScreenPos();
    const float  cellWidth  = targetWidth / m_width;
    const float  cellHeight = targetHeight / m_height;
    const float  cellInset  = (cellWidth < cellHeight ? cellWidth : cellHeight) * 0.12F;
    ImDrawList*  drawList   = ImGui::GetWindowDrawList();

    drawList->AddRectFilled(
        displayPos,
        ImVec2(displayPos.x + targetWidth, displayPos.y + targetHeight),
        IM_COL32(24, 27, 26, 255));

    if (true == m_power)
    {
        uint16_t bitmap_width  = bitmap.getWidth();
        uint16_t bitmap_height = bitmap.getHeight();

        if (m_width < bitmap_width)
        {
            bitmap_width = m_width;
        }

        if (m_height < bitmap_height)
        {
            bitmap_height = m_height;
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
                    5.0F);
            }
        }
    }
    /* Save the space of the pixel area from placing other widgets */
    ImGui::Dummy(ImVec2(targetWidth, targetHeight));
}

void LedGridSim::renderMenuBar() const
{
    bool showAboutDialog = false;

    if (true == ImGui::BeginMenuBar())
    {
        if (true == ImGui::MenuItem("About"))
        {
            showAboutDialog = true;
        }

        ImGui::SameLine();
        std::string webpageUrl("http://localhost:");
        webpageUrl += std::to_string(CONFIG_WEBSERVER_PORT);

        ImGui::TextLinkOpenURL("Open Web Page", webpageUrl.c_str());

        /* Reserve space and push "FPS" to the right edge */
        float helpWidth = ImGui::CalcTextSize("FPS: 100.00").x + ImGui::GetStyle().FramePadding.x * 2.0F;
        ImGui::SameLine(ImGui::GetWindowWidth() - helpWidth - 10.0F);

        ImGui::Text("FPS: %.1f", this->getPower() ? ImGui::GetIO().Framerate : 0.0F);

        ImGui::EndMenuBar();
    }

    if (true == showAboutDialog)
    {
        ImGui::OpenPopup("About");
    }

    // Centered modal popup
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (true == ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (nullptr != m_logoTexture)
        {
            const ImTextureID textureId   = static_cast<ImTextureID>(reinterpret_cast<intptr_t>(m_logoTexture));
            const float       logoOffsetX = (ImGui::GetContentRegionAvail().x - m_logoTexture->w) * 0.5F;
            /* Center the logo horizontally */
            if (0.0F < logoOffsetX)
            {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + logoOffsetX);
            }
            ImGui::Image(ImTextureRef(textureId), ImVec2(m_logoTexture->w, m_logoTexture->h));
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

        const float closeButtonWidth   = 120.0F;
        const float closeButtonOffsetX = (ImGui::GetContentRegionAvail().x - closeButtonWidth) * 0.5F;
        if (0.0F < closeButtonOffsetX)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + closeButtonOffsetX);
        }

        if (true == ImGui::Button("Close", ImVec2(closeButtonWidth, 0)))
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
}


static uint32_t getDefaultWindowHeight(uint32_t windowWidth, uint32_t ledsX, uint32_t ledsY)
{
    /* Calculate pixel grid height based on given windowWidth and resolution aspect ratio. */
    uint32_t height  = windowWidth;
    height          *= ledsY;
    height          /= ledsX;

    /* Add extra space for window title/menu bar and button bar */
    height          += RESERVED_MENUBAR_HEIGHT + RESERVED_BUTTONBAR_HEIGHT;

    return height;
}

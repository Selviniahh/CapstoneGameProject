#include <iostream>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <SDL3/SDL.h>
#include <cstring>
#include "Engine.h"
#include "../Core/ComponentBase.h"
#include "../Managers/AssetManager.h"
#include "../Managers/GameState.h"
#include "../Managers/InputManager.h"
#include "../Managers/TypeRegistry.h"
#include "../../Utils/Math.h"
#include "UI/EngineUI.h"

bool Engine::CurrentGameFocus = false;
bool Engine::PreviousGameFocus = false;

bool Engine::AbsoluteOrientationOpen = false;
bool Engine::RelativeOrientationOpen = false;
bool Engine::PropertiesOpen = false;

using namespace ETG::Globals;
using namespace ETG;

void Engine::Initialize()
{
    //Initialize ImGui with the SDL3 window and renderer
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    if (!ImGui_ImplSDL3_InitForSDLRenderer(Window->getNativeWindow(), Window->getNativeRenderer()))
        throw std::runtime_error("Cannot initialize ImGUI with the given Window");
    if (!ImGui_ImplSDLRenderer3_Init(Window->getNativeRenderer()))
        throw std::runtime_error("Cannot initialize the ImGUI SDL_Renderer backend");

    GameState::GetInstance().SetEngine(this);

    GameState::GetInstance().SetEngineUISize(&windowSize);
    windowSize = {400, (float)ETG::RenderWindow::LogicalSize.y};
    std::cout << std::unitbuf;

    LoadFont();
}

void Engine::Update()
{
    //Start a new ImGui frame
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();

    //ImGui draws entirely in the fixed logical canvas (RenderWindow::LogicalSize); SDL's
    //permanently-active logical presentation is what maps that onto the real, possibly-HiDPI
    //window. Force both so ImGui doesn't also apply its own (unrelated) DPI scale, and so its
    //DisplaySize matches the logical canvas that GameManager converts mouse events into.
    ImGuiIO& io = ImGui::GetIO();
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    io.DisplaySize = ImVec2((float)ETG::RenderWindow::LogicalSize.x, (float)ETG::RenderWindow::LogicalSize.y);

    ImGui::NewFrame();

    // Pin the panel to the logical canvas's right edge and full height every frame, so it
    // always stays docked there instead of drifting once the user drags/resizes it
    // (ImGuiCond_FirstUseEver only applied the position/size on the very first frame).
    const float windowW = (float)ETG::RenderWindow::LogicalSize.x;
    const float windowH = (float)ETG::RenderWindow::LogicalSize.y;
    ImGui::SetNextWindowPos(ImVec2(windowW - windowSize.x, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(windowSize.x, windowH), ImGuiCond_Always);

    ImGui::Begin("Details Pane", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    UpdateDetailsPanel();

    PreviousGameFocus = CurrentGameFocus;
    CurrentGameFocus = !(ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
                         ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) ||
                         io.WantCaptureMouse ||
                         io.WantCaptureKeyboard);

    //The end
    ImGui::End();
}

bool Engine::IsGameWindowFocused()
{
    // First check if ImGui wants to capture mouse input
    const ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse || io.WantCaptureKeyboard)
        return false;

    // Check if the game has focus; if not, ignore input.
    if (!CurrentGameFocus)
        return false;

    // Determine if focus was just gained
    if (CurrentGameFocus && !Engine::PreviousGameFocus)
        InputManager::LeftClickRequired = true;

    // Process events to check for mouse release
    if (InputManager::LeftClickRequired && InputManager::GameEvent.type == SDL_EVENT_MOUSE_BUTTON_UP &&
        InputManager::GameEvent.button.button == SDL_BUTTON_LEFT)
    {
        InputManager::LeftClickRequired = false;
    }

    // If LeftClickRequired is still true, wait for release before processing input
    if (InputManager::LeftClickRequired)
    {
        return false;
    }
    return true;
}

void Engine::Draw()
{
    //Render
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), Window->getNativeRenderer());
}

//Probably the way I am making selection is wrong. Fix it with the convenient way ImGUI handled before

void Engine::UpdateDetailsPanel()
{
    //NOTE: Open the Game Objects pane by default
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);

    //Game object pane
    if (ImGui::CollapsingHeader("Hierarchy", ImGuiTreeNodeFlags_None))
    {
        const float hierarchyHeight = Math::CalculatePercentageOfValue((float)ScreenSize.y, 25);
        // constexpr float hierarchyHeight = 300;

        // Create a child window with scrolling enabled
        ImGui::BeginChild("HierarchyScrollingRegion", ImVec2(0, hierarchyHeight), true, ImGuiWindowFlags_HorizontalScrollbar);

        // Assuming Scene is the root object
        GameObjectBase* sceneObj = GameState::GetInstance().GetSceneObj();
        DisplayHierarchy(sceneObj);

        ImGui::EndChild(); // End the scrollable region
    }

    //NOTE: Open the details pane by default
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);

    //Show details of selected objects
    if (ImGui::CollapsingHeader("Details", ImGuiTreeNodeFlags_None))
    {
        //Open the pane by default once
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);

        DisplayProperties();
    }
}


void Engine::DisplayHierarchy(GameObjectBase* object)
{
    ImGui::PushID(object);

    //Is current object has any children prepare to make it expandible node tree instead of just selectable.
    bool currObjHasChildren = false;
    for (const auto* sceneObj : GameState::GetInstance().GetSceneObjs())
    {
        if (sceneObj->Owner == object)
        {
            currObjHasChildren = true;
            break;
        }
    }

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (object == GameState::GetInstance().GetSceneObj()) flags |= ImGuiTreeNodeFlags_DefaultOpen; //Default expand the scene objects
    if (SelectedObj == object) flags |= ImGuiTreeNodeFlags_Selected;

    //Create tree node if the object has children. If not create single selectable widget
    bool isOpen;
    currObjHasChildren
        ? isOpen = ImGui::TreeNodeEx(object->GetObjectName().c_str(), flags)
        : isOpen = ImGui::Selectable(object->GetObjectName().c_str(), object == SelectedObj);

    //Handle selection
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        SelectedObj = object;
    }

    //Find all the children of current object and Draw all of them
    if (isOpen)
    {
        for (const auto& sceneObj : GameState::GetInstance().GetSceneObjs())
        {
            if (sceneObj->Owner == object)
            {
                DisplayHierarchy(sceneObj);
            }
        }

        if (currObjHasChildren) ImGui::TreePop();
    }

    ImGui::PopID();
}

void Engine::DisplayProperties() const
{
    // Check if the selected object is still valid
    if (SelectedObj && SelectedObj->IsValid())
    {
        //Display the Absolute and Relative orientation only if the SelectedObj is not inherited from ComponentBase
        if (!dynamic_cast<ComponentBase*>(SelectedObj))
        {
            if (ImGui::TreeNode("Absolute Orientation"))
            {
                ImGuiSetAbsoluteOrientation(SelectedObj);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Relative Orientation"))
            {
                ImGuiSetRelativeOrientation(SelectedObj);
                ImGui::TreePop();
            }
        }

        //Use stored state for properties tree node
        ImGui::SetNextItemOpen(PropertiesOpen);
        if (ImGui::TreeNode("Properties"))
        {
            PropertiesOpen = true;
            TypeRegistry::ProcessObject(SelectedObj);

            if (SelectedObj->IsGameObjectUISpecified && ImGui::TreeNode("Current Object"))
            {
                SelectedObj->PopulateSpecificWidgets();
                ImGui::TreePop();
            }

            ImGui::TreePop();
        }
        else
            PropertiesOpen = false;
    }
}

void Engine::LoadFont()
{
    const ImGuiIO io = ImGui::GetIO();
    io.Fonts->Clear();

    //Load through SDL's IO layer instead of AddFontFromFileTTF (stdio), so it also works from APK assets on Android.
    //ImGui frees the buffer with its own allocator, so the bytes are copied into an ImGui-owned block.
    const std::vector<unsigned char> fontData = ETG::AssetManager::LoadBytes("Fonts/SegoeUI.ttf");
    if (fontData.empty()) throw std::runtime_error("Failed to load font Fonts/SegoeUI.ttf");

    void* imguiOwned = IM_ALLOC(fontData.size());
    std::memcpy(imguiOwned, fontData.data(), fontData.size());
    SegoeFont = io.Fonts->AddFontFromMemoryTTF(imguiOwned, static_cast<int>(fontData.size()), 18.f);
    if (SegoeFont == nullptr) throw std::runtime_error("Failed to build ImGui font from Fonts/SegoeUI.ttf");
    //NOTE: With ImGui 1.92+ and the SDL_Renderer3 backend, the font atlas texture is managed automatically
}

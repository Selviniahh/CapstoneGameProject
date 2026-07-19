#include <SDL3/SDL.h>
#include <imgui_impl_sdl3.h>
#include "GameManager.h"
#include "../../Engine/Managers/AssetManager.h"
#include "../../Engine/Managers/DebugTexts.h"
#include "../../Engine/Managers/InputManager.h"
#include "../../Engine/Managers/SpriteBatch.h"
#include "../../Engine/Managers/Globals.h"
#include "../../Engine/Core/Components/CollisionComponent.h"
#include "../../Engine/Core/Scene/Scene.h"
#include "../UI/UserInterface.h"
#include "../Levels/SpawnInitialLevel.h"
#include "RegisterGameTypes.h"

using namespace ETG::Globals;

ETG::GameManager::~GameManager() = default;

ETG::GameManager::GameManager()
{
    Initialize();
}

void ETG::GameManager::Initialize()
{
    //Always launch fullscreen at the desktop's native resolution
    if (!SDL_WasInit(SDL_INIT_VIDEO)) 
        SDL_InitSubSystem(SDL_INIT_VIDEO);

    //Resolve the Resources root before anything tries to load an asset
    AssetManager::Initialize();
    
    int desktopWidth = 1280, desktopHeight = 720;
    if (const SDL_DisplayMode* desktopMode = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay()))
    {
        desktopWidth = desktopMode->w;
        desktopHeight = desktopMode->h;
    }
    Window = std::make_shared<ETG::RenderWindow>(static_cast<unsigned>(desktopWidth), static_cast<unsigned>(desktopHeight), "Enter The Gungeon Clone (SDL3)", true);
    Window->requestFocus();
    Window->setFramerateLimit(Globals::FPS);
    GameState::GetInstance().SetRenderWindow(Window.get());

    //Initialize GameState instance before anything
    GameState::GetInstance();
    GameState::GetInstance().SetGameManager(this);

    //Scene is the ownership root every factory-created object attaches to, so it has to exist before
    //anything else. It's still owned by WorldObjects; it's pushed into the list below in update order.
    auto scene = std::make_unique<Scene>();
    scene->SetObjectNameToSelfClassName();
    GameState::GetInstance().SetSceneObj(scene.get());


    //NOTE: Secondly EngineUI needs to be initialized
    EngineUI.Initialize();

    //Reflection type list is game content; the engine only provides the registry machinery
    RegisterGameTypes();

    Globals::Initialize(Window);
    InputManager::InitializeDebugText();

    //Scene joins WorldObjects first, then the level content is spawned through the same pending-spawn
    //queue as runtime spawns and flushed immediately so everything is in place before the first Update.
    WorldObjects.push_back(std::move(scene));
    SpawnInitialLevel::Spawn(*this);
    FlushPendingSpawns();

    UI = CreateGameObjectDefault<UserInterface>();

    //Always initialize debug text last
    DebugText = std::make_unique<class DebugText>();
}

//Pause is decided in the main loop: while the window is unfocused, main never calls Update/Draw
void ETG::GameManager::Update()
{
    EngineUI.Update();
    Globals::Update();
    InputManager::Update();

    //Objects spawned last frame join the list before anyone updates, so they never draw un-updated
    FlushPendingSpawns();

    for (const auto& obj : WorldObjects)
        obj->Update();
    UI->Update();

    //Deallocate everything marked with MarkForDestroy during this frame
    SweepDestroyedObjects();
}

void ETG::GameManager::Draw()
{
    Window->clear({1,255,255,255});

    //NOTE: Draw the main game scene with Custom view. These draws will be drawn zoomed.
    //Everything is drawn against the fixed logical canvas (RenderWindow::LogicalSize);
    //SDL's permanently-active logical presentation letterboxes it onto the real window,
    //regardless of how the window is resized.
    Window->setView(Globals::MainView);

    GlobSpriteBatch.begin();
    for (const auto& obj : WorldObjects) 
        obj->Draw();
    GlobSpriteBatch.end(*Window);

    //NOTE: Switch to the default (un-zoomed) view for overlays (UI). These draws will be drawn in screen coords.
    //NOTE: Which means, even though The view zoomed or moved, these draws will always stay persistent in initial given coords
    Window->setView(Window->getDefaultView());

    ETG::GlobSpriteBatch.begin();
    UI->Draw();
    ETG::GlobSpriteBatch.end(*Window);

    //NOTE: non batch draws here.
    // DebugText->Draw(*Window);
    EngineUI.Draw();

    //Display the frame after everything is set to be drawn
    Window->display();
}

void ETG::GameManager::FlushPendingSpawns()
{
    for (auto& obj : PendingSpawns)
        WorldObjects.push_back(std::move(obj));
    PendingSpawns.clear();
}

void ETG::GameManager::SweepDestroyedObjects()
{
    bool anyDestroyed = false;
    for (auto it = WorldObjects.begin(); it != WorldObjects.end();)
    {
        if ((*it)->IsPendingDestroy())
        {
            UnregisterGameObject(it->get());
            it = WorldObjects.erase(it); //unique_ptr deallocates the object here
            anyDestroyed = true;
        }
        else
        {
            ++it;
        }
    }

    //Components attached to a destroyed object registered themselves under their own names and died
    //with their owner. Purge those now-dangling registry entries so the hierarchy panel stays safe.
    if (anyDestroyed)
    {
        auto& sceneObjs = GameState::GetInstance().GetSceneObjs();
        std::erase_if(sceneObjs, [](const GameObjectBase* obj) { return !GameClass::IsValid(obj); });
    }
}

void ETG::GameManager::ProcessEvents()
{
    //Reset per-frame input accumulators
    InputManager::MouseWheelDelta = 0.f;

    SDL_Event event;
    while (Window->pollEvent(event))
    {
        InputManager::GameEvent = event;

        if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) HasFocus = false;
        if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED)
        {
            HasFocus = true;
            //The game was paused while unfocused; restart the tick clock so the paused duration
            //doesn't hit the simulation as one giant DeltaTime.
            Globals::ResetTick();
        }
        if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
        {
            Globals::Font.reset();
            Window->close();
            return;
        }

        // Handle window resize
        if (event.type == SDL_EVENT_WINDOW_RESIZED)
        {
            // Update the global screen size (real window pixels; not used for layout anymore
            // since everything now draws against the fixed logical canvas)
            ScreenSize = {static_cast<unsigned>(event.window.data1), static_cast<unsigned>(event.window.data2)};
        }

        //Accumulate mouse wheel movement for this frame (used for gun switching)
        if (event.type == SDL_EVENT_MOUSE_WHEEL)
        {
            InputManager::MouseWheelDelta += event.wheel.y;
        }

        //Poll and process events for ImGUI. Convert coordinates into the logical/render
        //coordinate space first, so ImGui's hit-testing lines up with the permanently-active
        //logical presentation (ImGui itself draws in that same logical space, see Engine::Update).
        SDL_Event imguiEvent = event;
        SDL_ConvertEventToRenderCoordinates(Window->getNativeRenderer(), &imguiEvent);
        ImGui_ImplSDL3_ProcessEvent(&imguiEvent);
    }
}

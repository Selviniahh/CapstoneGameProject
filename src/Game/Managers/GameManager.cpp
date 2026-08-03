#include <SDL3/SDL.h>
#include <imgui_impl_sdl3.h>
#include "GameManager.h"
#include "../../Engine/Managers/AssetManager.h"
#include "../../Engine/Managers/DebugTexts.h"
#include "../../Engine/Managers/InputManager.h"
#include "../../Engine/Managers/SpriteBatch.h"
#include "../../Engine/Managers/RenderContext.h"
#include "../../Engine/Managers/Time.h"
#include "../../Engine/Core/Components/CollisionComponent.h"
#include "../../Engine/Core/Scene/Scene.h"
#include "../UI/UserInterface.h"
#include "../Levels/SpawnInitialLevel.h"
#include "RegisterGameTypes.h"

using namespace ETG::RenderContext;

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
    Window->setVSyncEnabled(true);

    //Scene is the ownership root every factory-created object attaches to, so it has to exist before
    //anything else (constructing it publishes it as Scene::GetSelf() via SingleInstance). It's still
    //owned by WorldObjects; it's pushed into the list below in update order.
    auto scene = std::make_unique<Scene>();
    scene->SetObjectNameToSelfClassName();


    //NOTE: Secondly EngineUI needs to be initialized
    EngineUI.Initialize();

    //Reflection type list is game content; the engine only provides the registry machinery
    RegisterGameTypes();

    RenderContext::Initialize(Window);
    Time::Initialize();
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
    Time::Update();
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
    //GraphicsDevice letterboxes it onto the real window, regardless of how the window is
    //resized or what aspect ratio it ends up with.
    Window->setView(RenderContext::MainView);

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
        std::erase_if(Scene::Get()->SceneObjs, [](const GameObjectBase* obj) { return !GameClass::IsValid(obj); });
    }
}

namespace
{
    //Rewrite an event's mouse coordinates from window points into the fixed logical canvas.
    //SDL's own SDL_ConvertEventToRenderCoordinates did this while SDL_Renderer owned the
    //letterboxing; the letterbox is ours now (GraphicsDevice::GetViewportRect), so this is too.
    void ConvertEventToLogicalCoordinates(SDL_Event& event)
    {
        const auto& window = ETG::RenderContext::Window;
        if (!window) return;

        const auto toLogical = [&window](const float x, const float y) { return window->mapWindowPointToLogical({x, y}); };

        switch (event.type)
        {
        case SDL_EVENT_MOUSE_MOTION:
            {
                //Relative motion is a delta, so it only takes the scale, not the offset
                const ETG::Vector2f origin = toLogical(0.f, 0.f);
                const ETG::Vector2f moved = toLogical(event.motion.xrel, event.motion.yrel);
                const ETG::Vector2f position = toLogical(event.motion.x, event.motion.y);
                event.motion.x = position.x;
                event.motion.y = position.y;
                event.motion.xrel = moved.x - origin.x;
                event.motion.yrel = moved.y - origin.y;
                break;
            }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                const ETG::Vector2f position = toLogical(event.button.x, event.button.y);
                event.button.x = position.x;
                event.button.y = position.y;
                break;
            }
        case SDL_EVENT_MOUSE_WHEEL:
            {
                const ETG::Vector2f position = toLogical(event.wheel.mouse_x, event.wheel.mouse_y);
                event.wheel.mouse_x = position.x;
                event.wheel.mouse_y = position.y;
                break;
            }
        default:
            break;
        }
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
            Time::ResetTick();
        }
        if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
        {
            DebugText::Font.reset();
            Window->close();
            return;
        }

        // Handle window resize
        if (event.type == SDL_EVENT_WINDOW_RESIZED || event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
        {
            // Update the global screen size (real window pixels; not used for layout anymore
            // since everything now draws against the fixed logical canvas)
            ScreenSize = {static_cast<unsigned>(event.window.data1), static_cast<unsigned>(event.window.data2)};

            //The backbuffer follows the window; the logical canvas is re-letterboxed into it
            Window->handleResize();
        }

        //Accumulate mouse wheel movement for this frame (used for gun switching)
        if (event.type == SDL_EVENT_MOUSE_WHEEL)
        {
            InputManager::MouseWheelDelta += event.wheel.y;
        }

        //Poll and process events for ImGUI. Convert coordinates into the logical canvas first, so
        //ImGui's hit-testing lines up with the letterboxed presentation (ImGui itself draws in
        //that same logical space, see Engine::Update).
        SDL_Event imguiEvent = event;
        ConvertEventToLogicalCoordinates(imguiEvent);
        ImGui_ImplSDL3_ProcessEvent(&imguiEvent);
    }
}

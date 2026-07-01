#include <SDL3/SDL.h>
#include <imgui_impl_sdl3.h>
#include "GameManager.h"
#include "DebugTexts.h"
#include "InputManager.h"
#include "SpriteBatch.h"
#include "Globals.h"
#include "../Guns/AK-47/AK47.h"
#include "../Core/Components/CollisionComponent.h"
#include "../Core/Scene/Scene.h"
#include "../Characters/Hero.h"
#include "../UI/UserInterface.h"
#include "../Enemy/BulletMan/BulletMan.h"
#include "../Items/Active/DoubleShoot.h"
#include "../Items/Passive/PlatinumBullets.h"
#include "../Guns/SawedOff/SawedOff.h"
#include "../Guns/Magnum/Magnum.h"


SDL_Event ETG::GameManager::GameEvent{};
using namespace ETG::Globals;

ETG::GameManager::~GameManager() = default;

ETG::GameManager::GameManager()
{
    Initialize();
}

void ETG::GameManager::Initialize()
{
    //During development for different resolution and size monitors, Window mode will be half of host's window size
    if (!SDL_WasInit(SDL_INIT_VIDEO)) SDL_InitSubSystem(SDL_INIT_VIDEO);
    int desktopWidth = 1280, desktopHeight = 720;
    if (const SDL_DisplayMode* desktopMode = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay()))
    {
        desktopWidth = desktopMode->w;
        desktopHeight = desktopMode->h;
    }
    Window = std::make_shared<ETG::RenderWindow>(static_cast<unsigned>(desktopWidth / 1.2), static_cast<unsigned>(desktopHeight / 1.2), "Enter The Gungeon Clone (SDL3)");
    Window->requestFocus();
    Window->setFramerateLimit(Globals::FPS);
    GameState::GetInstance().SetRenderWindow(Window.get());

    //Initialize GameState instance before anything and initialize SceneObj vector
    GameState::GetInstance();
    GameState::GetInstance().SetSceneObjs(SceneObjects);

    //What's going on at here is only applicable for Scene object.
    Scene = std::make_unique<class Scene>();
    Scene->SetObjectNameToSelfClassName();
    GameState::GetInstance().SetSceneObj(Scene.get());


    //NOTE: Secondly EngineUI needs to be initialized
    EngineUI.Initialize();

    Globals::Initialize(Window);
    InputManager::InitializeDebugText();

    Hero = ETG::CreateGameObjectDefault<class Hero>(ETG::Vector2f{10,10});

    UI = ETG::CreateGameObjectDefault<UserInterface>();

    //Always initialize debug text last
    DebugText = std::make_unique<class DebugText>();

    BulletMan = ETG::CreateGameObjectDefault<class BulletMan>(ETG::Vector2f{50,50});
    PlatinumBullets = ETG::CreateGameObjectDefault<class PlatinumBullets>();
    DoubleShoot = ETG::CreateGameObjectDefault<class DoubleShoot>();
    Ak47 = ETG::CreateGameObjectDefault<class AK47>(ETG::Vector2f{-100,100});
    SawedOff = ETG::CreateGameObjectDefault<class SawedOff>(ETG::Vector2f{-150,100});
    Magnum = ETG::CreateGameObjectDefault<class Magnum>(ETG::Vector2f{-200,100});


    //TODO: Work on safely destroying and error resolution for accessing destroyed object
    // DestroyGameObject(Hero);


}

void ETG::GameManager::Update()
{
    if (HasFocus)
    {
        EngineUI.Update();
        Globals::Update();
        InputManager::Update();
        Hero->Update();
        UI->Update();
        Ak47->Update();
        SawedOff->Update();
        Magnum->Update();
        Scene->Update();
        BulletMan->Update();
        PlatinumBullets->Update();
        DoubleShoot->Update();
    }

}

void ETG::GameManager::Draw()
{
    if (!HasFocus) return;
    Window->clear({1,255,255,255});

    //NOTE: Draw the main game scene with Custom view. These draws will be drawn zoomed
    Window->setView(Globals::MainView);

    GlobSpriteBatch.begin();
    Hero->Draw();
    Scene->Draw();
    BulletMan->Draw();
    PlatinumBullets->Draw();
    DoubleShoot->Draw();
    Ak47->Draw();
    SawedOff->Draw();
    Magnum->Draw();
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

void ETG::GameManager::ProcessEvents()
{
    //Reset per-frame input accumulators
    InputManager::MouseWheelDelta = 0.f;

    SDL_Event event;
    while (Window->pollEvent(event))
    {
        GameEvent = event;

        if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) HasFocus = false;
        if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED) HasFocus = true;
        if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
        {
            Globals::Font.reset();
            Window->close();
            return;
        }

        // Handle window resize
        if (event.type == SDL_EVENT_WINDOW_RESIZED)
        {
            // Update the global screen size
            ScreenSize = {static_cast<unsigned>(event.window.data1), static_cast<unsigned>(event.window.data2)};

            // Optionally update the default view if you rely on it
            Window->setView(Window->getDefaultView());

            //TODO: When the window is resized, I need to recalculate every position of every UI object.
            // Recalculate UI positions based on the new screen size.
            // UI->Initialize(); // or UI.UpdatePositions(); if you separate the logic.
        }

        //Accumulate mouse wheel movement for this frame (used for gun switching)
        if (event.type == SDL_EVENT_MOUSE_WHEEL)
        {
            InputManager::MouseWheelDelta += event.wheel.y;
        }

        //Poll and process events for ImGUI
        ImGui_ImplSDL3_ProcessEvent(&event);
    }
}

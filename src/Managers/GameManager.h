#pragma once
#include <unordered_map>
#include <SDL3/SDL_events.h>
#include "Globals.h"
#include "../Engine/Engine.h"

namespace ETG
{
    class DebugText;
    class Scene;
    class Hero;
    class UserInterface;
    class PlatinumBullets;
    class BulletMan;
    class DoubleShoot;
    class AK47;
    class SawedOff;
    class Magnum;


    class GameManager
    {
    public:
        GameManager();
        ~GameManager();
        void Initialize();
        void ProcessEvents();
        [[nodiscard]] bool WindowHasFocus() const { return HasFocus; }

        //I might delete this later on
        static bool IsRunning() { return Globals::Window->isOpen(); }
        void Update();
        void Draw();

    private:
        std::unique_ptr<Hero> Hero;
        std::unique_ptr<UserInterface> UI;
        std::unique_ptr<Scene> Scene;
        std::unique_ptr<BulletMan> BulletMan;

        std::unique_ptr<PlatinumBullets> PlatinumBullets;
        std::unique_ptr<DoubleShoot> DoubleShoot;
        std::unique_ptr<AK47> Ak47;
        std::unique_ptr<SawedOff> SawedOff;
        std::unique_ptr<Magnum> Magnum;

        Engine EngineUI{};

        bool HasFocus = true;
        std::unique_ptr<DebugText> DebugText;

    public:
        //Hold only scene objects. Used for displaying details panel
        std::unordered_map<std::string,GameObjectBase*> SceneObjects;

        static SDL_Event GameEvent;
    };
}

#pragma once

#include <unordered_map>
#include "Globals.h"
#include "../Engine/Engine.h"
#include "../Items/ActiveItem.h"
#include "../Items/PassiveItem.h"


namespace ETG
{
    class BulletMan;
}

namespace ETG
{
    class DebugText;
    class Scene;
    class Hero;
    class UserInterface;

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
        std::unique_ptr<ActiveItem> ActiveItem;
        std::unique_ptr<PassiveItem> PassiveItem;

    private:
        std::unique_ptr<Hero> Hero;
        std::unique_ptr<UserInterface> UI;
        std::unique_ptr<Scene> Scene;
std::unique_ptr<BulletMan> BulletMan;
        
        Engine EngineUI{};

        bool HasFocus = true;
        std::unique_ptr<DebugText> DebugText;
        // Active and Passive Items

        void SpawnItems();
        void SpawnActiveItem(const std::string& texturePath, const sf::Vector2f& position);
        void SpawnPassiveItem(const std::string& texturePath, const sf::Vector2f& position);
        void HandleItemEquip();
        void UpdateItems();

    public:
        //Hold only scene objects. Used for displaying details panel
        std::unordered_map<std::string, GameObjectBase*> SceneObjects;

        static sf::Event GameEvent;
    };
}

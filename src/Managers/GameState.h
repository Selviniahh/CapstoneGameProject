#pragma once
#include <unordered_map>
#include "../Core/Scene/Scene.h"
#include "../Engine/Engine.h"
#include "../Items/Active/ActiveItemBase.h"
#include "../Items/Passive/PassiveItemBase.h"

namespace ETG
{
    class Hero;
    class Scene;
    class GameObjectBase;
    class PassiveItemBase;
    class GameManager;


    class GameState
    {
    public:
        static GameState& GetInstance()
        {
            static GameState instance;
            return instance;
        }

        [[nodiscard]] Hero* GetHero() const { return MainHero; }
        [[nodiscard]] GameManager* GetGameManager() const { return GameManagerPtr; } //Spawn runtime objects through this
        [[nodiscard]] std::unordered_map<std::string, GameObjectBase*>& GetSceneObjs() const { return *SceneObjs; } //Original method for fast lookups
        [[nodiscard]] std::vector<GameObjectBase*>& GetOrderedSceneObjs() {return OrderedSceneObjs;} //New, for ordered hierarchy pane 
        [[nodiscard]] ETG::Vector2f* GetEngineUISize() const { return EngineUISize; }
        [[nodiscard]] Engine* GetEngine() const { return Engine; }
        [[nodiscard]] Scene* GetSceneObj() const { return SceneObj; }
        [[nodiscard]] ETG::RenderWindow* GetRenderWindow() const { return Window; }
        [[nodiscard]] std::vector<PassiveItemBase*>& GetEquippedPassiveItems() { return EquippedPassiveItems; }
        [[nodiscard]] std::vector<ActiveItemBase*>& GetEquippedActiveItems() { return EquippedActiveItems; }

        void SetHero(Hero* hero) { MainHero = hero; }
        void SetGameManager(GameManager* gameManager) { GameManagerPtr = gameManager; }
        void SetSceneObjs(std::unordered_map<std::string, GameObjectBase*>& sceneObj) { SceneObjs = &sceneObj; }
        void SetEngineUISize(ETG::Vector2f* size) { EngineUISize = size; }
        void SetEngine(Engine* engine) { Engine = engine; }
        void SetSceneObj(Scene* sceneObj) { SceneObj = sceneObj; }
        void SetRenderWindow(ETG::RenderWindow* window) { Window = window; }

        //TODO: Who should own equipped items class? Hero cannot because hero also don't know which item it's interacted. UI or GameManager might but for now I will let this class to own equipped items.
        //NOTE: For now since equipped items at least knows hero, I set owner from scene to Hero when collided with hero. For now setting equipped item's owner to hero makes sense but we'll see in the future 
        // void SetEquippedPassiveItems(const std::vector<PassiveItemBase*>& eqPassiveItem) { EquippedPassiveItems = eqPassiveItem; }

    private:
        GameState() = default;
        Hero* MainHero = nullptr;
        GameManager* GameManagerPtr = nullptr;

        //Game objects (non-owning pointers)
        std::unordered_map<std::string, GameObjectBase*>* SceneObjs = nullptr;
        std::vector<GameObjectBase*> OrderedSceneObjs; //Ordered for display

        Scene* SceneObj = nullptr;
        ETG::RenderWindow* Window;

        Engine* Engine;
        
        //Owned objects
        ETG::Vector2f* EngineUISize{};
        std::vector<PassiveItemBase*> EquippedPassiveItems{};
        std::vector<ActiveItemBase*> EquippedActiveItems{};
    };
}

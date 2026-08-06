#pragma once
#include <functional>
#include <memory>
#include <vector>
#include "../../Engine/Managers/RenderContext.h"
#include "../../Engine/Editor/Engine.h"
#include "../../Engine/Core/Factory.h"

namespace ETG
{
    class DebugText;
    class UserInterface;

    //The initialization, Update and Draw cycle of all objects are handled here. 
    class GameManager
    {
    public:
        GameManager();
        ~GameManager();
        void Initialize();
        void ProcessEvents();
        [[nodiscard]] bool WindowHasFocus() const { return HasFocus; }

        [[nodiscard]] bool IsRunning() const { return RenderContext::Window && RenderContext::Window->isOpen(); }
        void Update();
        void Draw();

        //Spawn a world object into the central scene list at runtime. Safe to call mid-frame (even from
        //another object's Update): the object is queued and joins the list at the start of the next Update.
        //Objects marked with MarkForDestroy() are swept and deallocated at the end of each Update.
        //What gets spawned into the world once the engine is up. Left empty - which is what the game itself does -
        //Initialize spawns the game level (SpawnInitialLevel). A different host sets this BEFORE constructing the
        //GameManager and gets a world containing exactly what it spawned instead: that is how the interactive
        //gameplay tests (Test/Interactive) build their own environment without anyone editing the game's level.
        //
        //NOTE: static on purpose. It has to be set before the constructor runs, because the constructor is what
        //spawns the level
        inline static std::function<void(GameManager&)> LevelSpawnOverride{};

        template <typename T, typename... Args>
        T* SpawnGameObject(Args&&... args)
        {
            //Çağıranın argümanları lvalue mı rvalue mı verdiyse aynı şekilde CreateGameObjectDefault fonksiyonuna aktarmak
            //Ayrıca std::move doğrudan kopyalamayı ortadan kaldırmaz. Sadece nesneyi rvalue’ya çevirerek move constructor’ın seçilebilmesini  sağlar.
            auto obj = CreateGameObjectDefault<T>(std::forward<Args>(args)...);
            T* rawPtr = obj.get();
            PendingSpawns.push_back(std::move(obj)); //unique ptr sahiplik kopyalanamaz sadece tasinabilir
            return rawPtr;
        }

    private:
        //The HUD reads the live hero (its gun, its items), so it can only exist while there is one. Called at the
        //top of every Update: it builds the UI once a hero with a gun is in the world and drops it the moment that
        //hero is gone.
        //
        //NOTE: the game never notices this - its level spawns a hero before the first frame, so the UI is built on
        //that frame and never dropped. It matters for a host whose world is empty at startup and whose hero is
        //replaced at runtime (the interactive gameplay tests): the UI used to capture Hero::Get() once, in its
        //constructor, which is a dangling pointer the moment a different hero takes over
        void EnsureGameUI();

        //When an object needs to be spawns, it gets added to PendingSpawns. This function transfers those objects into WorldObjects and refreshes PendingSpawns
        void FlushPendingSpawns();
        
        void SweepDestroyedObjects();

        //Central owning list of every world object. Update/Draw iterate this list in order;
        //vector order is update order, draw order is resolved by SpriteBatch depth sorting.
        std::vector<std::unique_ptr<GameObjectBase>> WorldObjects;
        std::vector<std::unique_ptr<GameObjectBase>> PendingSpawns;

        //UI is an overlay: it draws with the default (un-zoomed) view in its own batch, so it stays outside WorldObjects
        std::unique_ptr<UserInterface> UI;

        Engine EngineUI{};

        bool HasFocus = true;
        std::unique_ptr<DebugText> DebugText;
    };
}

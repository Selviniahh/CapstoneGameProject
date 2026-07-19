#pragma once
#include <memory>
#include <vector>
#include <SDL3/SDL_events.h>
#include "Globals.h"
#include "../Engine/Engine.h"
#include "../Core/Factory.h"

namespace ETG
{
    class DebugText;
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

        //Spawn a world object into the central scene list at runtime. Safe to call mid-frame (even from
        //another object's Update): the object is queued and joins the list at the start of the next Update.
        //Objects marked with MarkForDestroy() are swept and deallocated at the end of each Update.
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

    public:
        static SDL_Event GameEvent;
    };
}

#pragma once
#include <functional>
#include <vector>
#include "../GameObjectBase.h"
#include "../SingleInstance.h"

namespace ETG
{
    //The active scene (Scene::GetSelf()): every factory-created object attaches to it
    //and registers itself into its SceneObjs list.
    class Scene : public GameObjectBase, public SingleInstance<Scene>
    {
    public:
        Scene();
        ~Scene() override = default;
        void Initialize() override;
        void Update() override;
        void Draw() override;
        void PopulateSpecificWidgets() override;

        //Single scene registry (non-owning pointers); insertion-ordered so the hierarchy panel stays stable
        std::vector<GameObjectBase*> SceneObjs;

        //Game-specific editor widgets (spawn buttons etc.) are injected from the game side,
        //so the engine's Scene never has to know concrete game types.
        std::function<void()> PopulateGameWidgets;

        BOOST_DESCRIBE_CLASS(Scene,(GameObjectBase),
            (),
            (),
            ())
    };
}
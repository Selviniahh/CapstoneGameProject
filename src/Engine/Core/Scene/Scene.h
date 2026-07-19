#pragma once
#include <functional>
#include "../GameObjectBase.h"

namespace ETG
{
    class Scene : public GameObjectBase
    {
    public:
        Scene();
        ~Scene() override = default;
        void Initialize() override;
        void Update() override;
        void Draw() override;
        void PopulateSpecificWidgets() override;

        //Game-specific editor widgets (spawn buttons etc.) are injected from the game side,
        //so the engine's Scene never has to know concrete game types.
        std::function<void()> PopulateGameWidgets;

        BOOST_DESCRIBE_CLASS(Scene,(GameObjectBase),
            (),
            (),
            ())
    };
}
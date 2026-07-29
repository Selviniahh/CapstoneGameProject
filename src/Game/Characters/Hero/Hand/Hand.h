#pragma once
#include "../../../../Engine/Core/GameObjectBase.h"

namespace ETG
{
    class Hand : public GameObjectBase
    {
    public:
        Hand();

        ETG::Vector2f GunOffset{0,0}; //2 ,2
        ETG::Vector2f HandOffset{0,0}; //-2 -1

        //The owning character rewrites this every frame - a hand is in front of the body from the front and
        //behind it from the back - so it has to be reachable from outside. Same trick GunBase uses for Rotation
        using GameObjectBase::Depth;

        void Initialize() override;
        void Draw() override;
        void Update() override;

        BOOST_DESCRIBE_CLASS(Hand, (GameObjectBase), (HandOffset, GunOffset), (), ())
    };
    
}

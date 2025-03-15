#pragma once
#include "../../Core/GameObjectBase.h"

namespace ETG
{
    class Hand : public GameObjectBase
    {
    public:
        Hand(const std::string& handTexture);

        sf::Vector2f GunOffset{0, 0}; //2 ,2 
        sf::Vector2f HandOffset{0, 0}; //-2 -1

        void Initialize() override;
        void Draw() override;
        void Update() override;

        BOOST_DESCRIBE_CLASS(Hand, (GameObjectBase), (HandOffset, GunOffset), (), ())
    };
}

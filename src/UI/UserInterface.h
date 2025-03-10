#pragma once
#include <SFML/Graphics/Texture.hpp>
#include "../Core/GameObjectBase.h"

namespace ETG
{
    class UserInterface : public GameObjectBase
    {
    private:
        sf::Texture Frame;
        sf::Texture AmmoBar;
        sf::Texture AmmoDisplay;
        sf::Texture Gun;
        sf::Vector2f GunPosition;
        sf::Vector2f AmmoBarPosition;
        std::vector<sf::Vector2i> AmmoArr{};
        int lastAmmoCount = 8;
        bool IsReloaded = true;
        bool RemoveLast = true;
        
        sf::Vector2f FrameOffsetPerc{4,3.5};
        float AmmoBarOffsetPercX = 2.f;

        sf::Vector2f GameScreenSize;

    public:
        UserInterface();
        void Initialize() override;
        void Update() override;
        void Draw() override;

        BOOST_DESCRIBE_CLASS(UserInterface,(GameObjectBase),
            (IsReloaded),
            (),
            ()
            )
    };

}

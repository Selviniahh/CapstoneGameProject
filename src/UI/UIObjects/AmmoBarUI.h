#pragma once
#include "../../Core/GameObjectBase.h"
#include <SFML/Graphics/Texture.hpp>
#include <vector>

namespace ETG
{
    class GunBase;

    class AmmoBarUI : public GameObjectBase
    {
    private:
        // Textures
        std::shared_ptr<sf::Texture> ammoBarTexture;

    public:
        AmmoBarUI();
        ~AmmoBarUI() override = default;

        void Initialize() override;
        void Update() override;
        void Draw() override;


        // Update the ammo display based on current gun state
        void FlipTexture(bool horizontally = true, bool vertically = true);

        BOOST_DESCRIBE_CLASS(AmmoBarUI, (GameObjectBase),(),(),(ammoBarTexture))
    };
}

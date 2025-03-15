#pragma once
#include <memory>
#include "../../../Core/Components/BaseAnimComp.h"

namespace sf
{
    class Texture;
}

namespace ETG
{
    enum class EnemyStateEnum;
    class BulletMan;

    class BulletManAnimComp : public BaseAnimComp<EnemyStateEnum>
    {
    public:
        BulletManAnimComp();
        ~BulletManAnimComp() override;
        void SetAnimations() override;
        void Initialize() override;
        void Update() override;

        ETG::BulletMan* BulletMan{};
        std::shared_ptr<sf::Texture> Texture; // Add texture field to match BaseAnimComp pattern
    };

}


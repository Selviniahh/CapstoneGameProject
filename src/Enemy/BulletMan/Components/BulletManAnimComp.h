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

    public:
        BulletMan* BulletMan{};
        std::shared_ptr<ETG::Texture> Texture; // Add texture field to match BaseAnimComp pattern

        BOOST_DESCRIBE_CLASS(BulletManAnimComp, (BaseAnimComp<EnemyStateEnum>),
            (BulletMan, Texture), (), ())
    };

}


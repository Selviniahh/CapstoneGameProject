#pragma once
#include "ActiveItemBase.h"

namespace ETG
{
    class Hero;
    class CollisionComponent;

    class DoubleShoot : public ActiveItemBase
    {
    public:
        DoubleShoot();
        ~DoubleShoot() override = default;
        void RequestUsage() override;

        void Initialize() override;
        void Update() override;
        void Draw() override;
        void ApplyPerk(const Hero* hero);

        std::unique_ptr<CollisionComponent> CollisionComp;

        static constexpr float DEFAULT_COOLDOWN = 15.0f;
        static constexpr float DEFAULT_ACTIVE_TIME = 10.0f;

        int ShootCount = 2;
        float SpreadAmount = 1.0f;


        BOOST_DESCRIBE_CLASS(DoubleShoot, (ActiveItemBase),(TotalCooldownTime, TotalConsumeTime, ConsumeTimer,CoolDownTimer, ActiveItemState, ShootCount, SpreadAmount),
                             (ItemDescription),
                             ())
    };
}

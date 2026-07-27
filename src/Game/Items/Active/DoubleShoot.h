#pragma once
#include "ActiveItemBase.h"
#include "../../Modifiers/Gun/IGunModifier.h"

namespace ETG
{
    class Hero;
    class GunBase;

    //The item IS the gun modifier: while it is being consumed it registers itself with the gun the hero is
    //holding and shapes every shot itself, so what the item does and what the effect does live in one file
    class DoubleShoot : public ActiveItemBase, public IGunModifier
    {
    public:
        DoubleShoot();
        ~DoubleShoot() override = default;
        void RequestUsage() override;

        void Initialize() override;
        void Update() override;
        void Draw() override;

        //<---------- IGunModifier ---------->
        void ModifyShot(ShotParams& shot) override;

        int ShootCount = 2;
        float SpreadAmount = 1.0f;

        //Which gun the effect was handed to. The hero may switch weapons mid-effect, and the modifier has to come
        //off the gun it was put on rather than whatever is in hand when the timer runs out
        GunBase* AffectedGun{};

        BOOST_DESCRIBE_CLASS(DoubleShoot, (ActiveItemBase),(TotalCooldownTime, TotalConsumeTime, ConsumeTimer,CoolDownTimer, ActiveItemState, ShootCount, SpreadAmount),
                             (ItemDescription),
                             ())
    };
}

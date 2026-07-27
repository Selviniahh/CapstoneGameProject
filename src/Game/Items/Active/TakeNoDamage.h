#pragma once
#include "ActiveItemBase.h"

namespace ETG
{
    class Hero;
    class CollisionComponent;
    
    class TakeNoDamage : public ActiveItemBase
    {
    public:
        TakeNoDamage();
        ~TakeNoDamage() override = default;
        void RequestUsage() override;
        
        void Initialize() override;
        void Update() override;
        void Draw() override;

        std::unique_ptr<CollisionComponent> CollisionComp;

        static constexpr float DEFAULT_COOLDOWN = 30.f;
        static constexpr float DEFAULT_ACTIVE_TIME = 8.f;

        //What the granted InvulnerabilityModifier is built with. The item only states the policy; turning a bullet
        //around is Hero's job, since the hero is the one holding the projectile when it arrives
        bool DeflectProjectiles = true;

        //The base flips Consuming -> Cooldown on its own, so the item has to remember it still owes the hero a
        //RemoveModifier. Without this, Update would re-run the removal on every frame of the cooldown
        bool IsEffectActive{};

        BOOST_DESCRIBE_CLASS(TakeNoDamage, (ActiveItemBase),(TotalCooldownTime, TotalConsumeTime, ConsumeTimer,CoolDownTimer, ActiveItemState, DeflectProjectiles, IsEffectActive),
                             (ItemDescription),
                             ())
    };
}





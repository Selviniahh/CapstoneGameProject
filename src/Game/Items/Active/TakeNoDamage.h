#pragma once
#include "ActiveItemBase.h"
#include "../../Modifiers/Hero/IHeroModifier.h"

namespace ETG
{
    class Hero;
    class CollisionComponent;

    //The item IS the hero modifier: while it is being consumed it registers itself with the hero and answers the
    //damage hook itself, so what the item does and what the effect does live in one file
    class TakeNoDamage : public ActiveItemBase, public IHeroModifier
    {
    public:
        TakeNoDamage();
        ~TakeNoDamage() override = default;
        void RequestUsage() override;

        void Initialize() override;
        void Update() override;
        void Draw() override;

        //<---------- IHeroModifier ---------->
        bool ReflectProjectile(Hero& hero, ProjectileBase* projectile) override;

        //Who is carrying this item, when that is someone the damage hook can be attached to
        [[nodiscard]] Hero* GetHolder() const;


        //Whether blocked shots are sent back at whoever fired them, or simply deleted
        bool DeflectProjectiles = true;

        
        BOOST_DESCRIBE_CLASS(TakeNoDamage, (ActiveItemBase),(TotalCooldownTime, TotalConsumeTime, ConsumeTimer,CoolDownTimer, ActiveItemState, DeflectProjectiles, IsEffectActive),
                             (ItemDescription),
                             ())
    };
}

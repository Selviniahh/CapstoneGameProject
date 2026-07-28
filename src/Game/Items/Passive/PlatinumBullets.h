#pragma once
#include <random>

#include "PassiveItemBase.h"
#include "../../../Engine/Core/Components/CollisionComponent.h" // Include the full definition

namespace ETG
{
    class CollisionComponent;
    class Character;

    class PlatinumBullets : public PassiveItemBase
    {
    public:
        PlatinumBullets();
        ~PlatinumBullets() override = default;

        void Initialize() override;
        void Update() override;
        void Draw() override;

        //Attaches the fire rate modifier. FireRate is the delay between shots, so "faster" is a smaller number and
        //the modifier is negative
        void ApplyGunPerk(GunBase& gun) override;

        std::unique_ptr<CollisionComponent> CollisionComp;
        float FireRateIncreasePerc = 20;

        //Whoever picked the item up. Set on pickup rather than resolved through Hero::Get(), so an enemy carrying
        //this buffs its own guns
        Character* Holder{};

        BOOST_DESCRIBE_CLASS(PlatinumBullets, (PassiveItemBase), (FireRateIncreasePerc), (), ())

    private:
        //NOTE: this used to be Perk(), which wrote `gun->FireRate = gun->BaseFireRate - 20%` into the single gun the
        //hero happened to be holding - so the perk was lost on a weapon switch, a second fire rate item overwrote it
        //outright, and Update() had to recompute the whole thing from the base value on every frame the percentage
        //changed, because a plain assignment left nothing to undo. It applies to every gun now, once each
        void ApplyToAllGuns();

        bool IsPickedUp = false; //Only a collected item is allowed to modify its holder's guns
        float PreviousFireRatePerc = 20; // Store previous value to detect changes
    };
}


#pragma once
#include <random>

#include "PassiveItemBase.h"
#include "../../../Engine/Core/Components/CollisionComponent.h" // Include the full definition

namespace ETG
{
    class CollisionComponent;
    class Hero;

    class PlatinumBullets : public PassiveItemBase
    {
    public:
        PlatinumBullets();
        ~PlatinumBullets() override = default;

        void Initialize() override;
        void Update() override;
        void Draw() override;
        void Perk(const Hero* hero) const;

        std::unique_ptr<CollisionComponent> CollisionComp;
        float FireRateIncreasePerc = 20;
        Hero* Hero{};

        BOOST_DESCRIBE_CLASS(PlatinumBullets, (PassiveItemBase), (FireRateIncreasePerc), (), ())
    private:
        float PreviousFireRatePerc = 20; // Store previous value to detect changes
    };    
}


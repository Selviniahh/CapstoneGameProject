#pragma once
#include "../EnemyBase.h"
#include "../../Characters/Hero.h"
#include "../../Core/Components/BaseMoveComp.h"
#include "../../Guns/Magnum/Magnum.h"

namespace ETG
{
    class BulletManAnimComp;

    class BulletMan : public EnemyBase
    {
    public:
        explicit BulletMan(const sf::Vector2f& position);
        ~BulletMan() override;
        void Initialize() override;
        void MoveTowardsHero();
        void Update() override;
        void Draw() override;


        EnemyStateEnum BulletManState{EnemyStateEnum::Idle}; // Initialize with default state
        Direction BulletManDir{Direction::Right}; // Initialize with default direction\

        float DistanceRange = 100;
        int stateTransitionCounter = 0;
        const int STATE_TRANSITION_DELAY = 5; // frames to wait before allowing state change
        sf::Vector2f HandOffsetForBulletMan{0.f, 0.f};
        
        // Shooting timing control
        float shootTimer = 0.0f;
        const float shootInterval = 1.5f; // Shoot every 1.5 seconds

        std::unique_ptr<BulletManAnimComp> AnimationComp;
        std::unique_ptr<Magnum> Magnum;
        std::unique_ptr<BaseMoveComp> MoveComp;
        std::unique_ptr<class Hand> Hand;

        BOOST_DESCRIBE_CLASS(BulletMan, (EnemyBase), (DistanceRange, HandOffsetForBulletMan, BulletManState, BulletManDir, shootTimer, shootInterval),(),())
    };
}
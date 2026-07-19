#pragma once
#include <boost/describe/class.hpp>
#include "../../../Engine/Platform/Platform.h"

#include "../../../Engine/Core/Components/BaseMoveComp.h"

namespace ETG
{
    class Hero;
    class EnemyBase;

    class EnemyMoveCompBase : public BaseMoveComp
    {
    public:
        explicit EnemyMoveCompBase(float maxSpeed = 100.0f, float acceleration = 5000.0f, float deceleration = 8000.0f);
        ~EnemyMoveCompBase() override;

        void Initialize() override;
        void Update() override;

        // AI movement parameters
        float DetectionRadius = 200.0f;     // How far enemy can detect the Hero
        float StopDistance = 150.0f;        // How close enemy gets before stopping
        float MovementSpeed = 40.0f;        // Default movement speed
        
    protected:
        // Enemy AI movement implementation
        virtual void UpdateAIMovement();

        // Get the direction to the hero
        ETG::Vector2f GetDirectionToHero() const;
        float GetDistanceToHero() const;
        
        Hero* Hero; // Static pointer to the hero for all enemies
        
        // The owner as an EnemyBase
        EnemyBase* OwnerEnemy = nullptr;
        
        BOOST_DESCRIBE_CLASS(EnemyMoveCompBase, (BaseMoveComp),
                         (DetectionRadius, StopDistance, MovementSpeed), (), ())
    };
}
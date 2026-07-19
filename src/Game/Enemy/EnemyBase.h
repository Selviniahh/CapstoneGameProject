#pragma once
#include "../Characters/Hero.h"
#include "../../Engine/Core/GameObjectBase.h"
#include "../../Engine/Core/Events/EventDelegate.h"
#include "../Managers/Enum/StateFlags.h"
#include "Components/EnemyMoveCompBase.h"

namespace ETG
{
    class ProjectileBase;
    class Hero;
    class Hand;
    class RogueSpecial;
    class BaseHealthComp;
    class CollisionComponent;
    class EnemyMoveCompBase;

    class EnemyBase : public GameObjectBase
    {
    protected:
        EnemyBase(); // Add default constructor
        ~EnemyBase() override;
        void Initialize() override;
        void Update() override;
        void Draw() override;

    public:
        // Projectile collision handlFing
        virtual void HandleHitForce(const ProjectileBase* projectile);

        [[nodiscard]] inline bool CanMove() const { return !HasAnyFlag(StateFlags, EnemyStateFlag::PreventMovement); }
        [[nodiscard]] inline bool CanShoot() const { return !HasAnyFlag(StateFlags, EnemyStateFlag::PreventShooting) && Hero->GetState() != HeroStateEnum::Die; }
        [[nodiscard]] inline bool CanFlipAnims() const { return !HasAnyFlag(StateFlags, EnemyStateFlag::PreventAnimFlip); }

        // Force handling helpers that delegate to MoveComp
        // [[nodiscard]] bool IsBeingForced() const { return MoveComp ? MoveComp->IsBeingForced : false; }
        // [[nodiscard]] bool IsDead() const { return EnemyState == EnemyStateEnum::Die; }
        void SetState(const EnemyStateEnum& state);
        [[nodiscard]] EnemyStateEnum GetState() const { return EnemyState; }

        std::unique_ptr<CollisionComponent> CollisionComp;
        std::unique_ptr<EnemyMoveCompBase> MoveComp;
        std::unique_ptr<BaseHealthComp> HealthComp;

        float KnockBackMagnitudeForDeath = 75;
        float KnockBackDurationForDeath = 1.0f;

        Direction EnemyDir{Direction::Right};
    private:
        EnemyStateEnum EnemyState{EnemyStateEnum::Idle};

    protected:
        std::unique_ptr<Hand> Hand;
        Hero* Hero;
        EnemyStateFlag StateFlags{EnemyStateFlag::StateIdle};

        //I am not sure is this the right way. Force should take time based on the hero's gun's fire rate. However, it also should be faster so that the enemy can stop force and have some time to shoot
        //before another projectile is fired. After some tests, I decided to divide Gun->FireRate / 3
        float ForceDurationDivider = 3.f;

        BOOST_DESCRIBE_CLASS(EnemyBase, (GameObjectBase), (Hero), (ForceDurationDivider), (EnemyState, EnemyDir))
    };
}

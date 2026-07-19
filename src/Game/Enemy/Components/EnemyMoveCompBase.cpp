#include "EnemyMoveCompBase.h"
#include "../../../Engine/Core/GameObjectBase.h"
#include "../../Managers/GameState.h"
#include "../../Characters/Hero.h"
#include "../../../Utils/Math.h"
#include "../EnemyBase.h"
#include "../../../Utils/DirectionUtils.h"

namespace ETG
{
    EnemyMoveCompBase::EnemyMoveCompBase(const float maxSpeed, const float acceleration, const float deceleration) : BaseMoveComp(maxSpeed, acceleration, deceleration)
    {
        IsGameObjectUISpecified = true;
        Hero = GameState::GetInstance().GetHero();
    }

    EnemyMoveCompBase::~EnemyMoveCompBase() = default;

    void EnemyMoveCompBase::Initialize()
    {
        BaseMoveComp::Initialize();

        // Get EnemyBase pointer from owner
        if (Owner)
        {
            OwnerEnemy = dynamic_cast<EnemyBase*>(Owner);
        }
    }

    void EnemyMoveCompBase::Update()
    {
        BaseMoveComp::Update(); // This will handle force updates

        if (!OwnerEnemy || !OwnerEnemy->CanMove()) return;

        UpdateAIMovement();
    }

    void EnemyMoveCompBase::UpdateAIMovement()
    {
        if (!OwnerEnemy || !Hero->IsValid() || !OwnerEnemy->CanMove()) return;

        // Calculate direction and distance to hero
        const ETG::Vector2f directionToHero = GetDirectionToHero();
        const float distanceToHero = GetDistanceToHero();

        // Update enemy's direction
        OwnerEnemy->EnemyDir = DirectionUtils::GetDirectionToHero(Hero, OwnerEnemy->GetPosition());

        // Don't change state if currently shooting
        if (OwnerEnemy->GetState() == EnemyStateEnum::Shooting)
        {
            // Still update movement if needed, but don't change state
            if (distanceToHero > StopDistance)
            {
                MaxSpeed = MovementSpeed;
                ETG::Vector2f position = OwnerEnemy->GetPosition();
                UpdateMovement(directionToHero, position);
                OwnerEnemy->SetPosition(position);
            }
            else
            {
                // Gradually slow down if within stop distance
                Velocity *= 0.9f;
            }
            return;
        }

        // Move if the hero is in range and not too close
        if (distanceToHero > StopDistance)
        {
            // Set the state to running when moving
            OwnerEnemy->SetState(EnemyStateEnum::Run);
            MaxSpeed = MovementSpeed;

            // Update movement
            ETG::Vector2f position = OwnerEnemy->GetPosition();
            UpdateMovement(directionToHero, position);
            OwnerEnemy->SetPosition(position);
        }
        else
        {
            // Either at proper distance or in cooldown, just idle
            OwnerEnemy->SetState(EnemyStateEnum::Idle);

            // Gradually slow down
            Velocity *= 0.9f;
        }
    }

    ETG::Vector2f EnemyMoveCompBase::GetDirectionToHero() const
    {
        if (!Hero->IsValid() || !OwnerEnemy) return {0, 0};

        return Math::Normalize(Hero->GetPosition() - OwnerEnemy->GetPosition());
    }

    float EnemyMoveCompBase::GetDistanceToHero() const
    {
        if (!Hero || !OwnerEnemy) return 0;

        return Math::Length(Hero->GetPosition() - OwnerEnemy->GetPosition());
    }
}

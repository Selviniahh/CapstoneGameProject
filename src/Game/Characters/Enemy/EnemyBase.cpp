#include "EnemyBase.h"
#include <limits>
#include "../../../Utils/Math.h"
#include "../Hero/Hand/Hand.h"
#include "../Hero/Hero.h"
#include "../../../Engine/Core/Components/CollisionComponent.h"
#include "../../../Engine/Managers/RenderContext.h"
#include "../../../Engine/Core/Factory.h"
#include "../../../Engine/Core/Components/BaseHealthComp.h"
#include "../../Projectile/ProjectileBase.h"
#include "Components/EnemyMoveCompBase.h"
#include "../../Guns/Base/GunBase.h"

namespace ETG
{
    EnemyBase::EnemyBase() : Hero(Hero::Get())
    {
        CollisionComp = ETG::CreateGameObjectAttached<CollisionComponent>(this);
        CollisionComp->CollisionRadius = 4.f;
        CollisionComp->CollisionVisualizationColor = ETG::Color::Magenta;
        CollisionComp->SetCollisionEnabled(true);

        MoveComp = ETG::CreateGameObjectAttached<EnemyMoveCompBase>(this);
        MoveComp->Initialize();

        // Register force event handlers for the move component
        MoveComp->OnForceStart.AddListener([this]()
        {
            this->SetState(EnemyStateEnum::Hit);
        });

        MoveComp->OnForceEnd.AddListener([this]()
        {
            // Reset to idle state when force ends
            if (GetState() == EnemyStateEnum::Hit)
                SetState(EnemyStateEnum::Idle);
        });

        HealthComp = ETG::CreateGameObjectAttached<BaseHealthComp>(this, 30.f);

        EnemyBase::Initialize();
    }

    EnemyBase::~EnemyBase() = default;

    void EnemyBase::Initialize()
    {
        CollisionComp->OnCollisionEnter.AddListener([this](const CollisionEventData& eventData)
        {
            // Check if we collided with a projectile
            if (eventData.Other->IsA<ProjectileBase>())
            {
                auto* projectile = eventData.Other->As<ProjectileBase>();

                //If collision is with our own or another enemy's projectile, ignore it
                if (projectile->Owner->Owner->IsA<EnemyBase>())
                    return;

                HealthComp->ApplyDamage(projectile->Damage, projectile->Force, projectile);
            }
        });


        HealthComp->OnDeath.AddListener([this](const GameObjectBase* instigator)
        {
            // If dead, ignore the damage
            if (GetState() == EnemyStateEnum::Die) return;

            // Set enemy state to die
            SetState(EnemyStateEnum::Die);
            Depth = std::numeric_limits<float>::max(); //set depth to max value so that it will be drawn bottom of everything
            const ETG::Vector2f knockbackDir = Math::Normalize(Position - instigator->GetPosition());
            MoveComp->ApplyForce(knockbackDir, KnockBackMagnitudeForDeath, KnockBackDurationForDeath);

            //Clear the delegates to not let any interaction
            MoveComp->OnForceStart.Clear();
            MoveComp->OnForceEnd.Clear();
            HealthComp->OnDamageTaken.Clear();
            CollisionComp->SetCollisionEnabled(false);
        });

        //In the future, enemy will take damage from explosive environment %
        HealthComp->OnDamageTaken.AddListener([this](const float damage, const float forceMagnitude, const GameObjectBase* instigator)
        {
            HandleHitForce(instigator->As<ProjectileBase>());
        });
    }

    void EnemyBase::Update()
    {
        MoveComp->Update();
        HealthComp->Update();
        CollisionComp->Update();

        GameObjectBase::Update();
    }

    void EnemyBase::Draw()
    {
        GameObjectBase::Draw();
    }

    void EnemyBase::SetState(const EnemyStateEnum& state)
    {
        EnemyState = state;

        switch (state)
        {
        case EnemyStateEnum::Idle: StateFlags = EnemyStateFlag::StateIdle;
            break;
        case EnemyStateEnum::Run: StateFlags = EnemyStateFlag::StateRun;
            break;
        case EnemyStateEnum::Shooting: StateFlags = EnemyStateFlag::StateShooting;
            break;
        case EnemyStateEnum::Hit: StateFlags = EnemyStateFlag::StateHit;
            break;
        case EnemyStateEnum::Die: StateFlags = EnemyStateFlag::StateDie;
            break;
        default: break;
        }
    }

    //Before calling this function, we already ensured that the projectile is not owned by any enemy
    void EnemyBase::HandleHitForce(const ProjectileBase* projectile)
    {
        const auto* projectileOwnerGun = projectile->Owner->As<GunBase>();

        // Check if this is a hero projectile
        // Calculate force direction (from projectile to enemy)
        const ETG::Vector2f forceDirection = Math::Normalize(this->Position - projectile->GetPosition());

        // Get force from projectile
        const float forceMagnitude = projectile->Force;

        auto* Gun = projectile->Owner->As<GunBase>();
        
        // Apply the force
        MoveComp->ApplyForce(forceDirection, forceMagnitude,  Gun->ForceDuration);
    }
}
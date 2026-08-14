#include "EnemyBase.h"
#include <limits>
#include "../../../Utils/Math.h"
#include "../Hero/Hand/Hand.h"
#include "../Hero/Hero.h"
#include "../../../Engine/Core/Components/CollisionComponent.h"
#include "../../../Engine/Managers/RenderContext.h"
#include "../../../Engine/Core/Factory.h"
#include "../../../Engine/Core/Components/BaseHealthComp.h"
#include "../../../Engine/Core/Components/ShaderEffectComponent.h"
#include "../../Projectile/ProjectileBase.h"
#include "Components/EnemyMoveCompBase.h"
#include "../../Guns/Base/GunBase.h"

namespace ETG
{
    EnemyBase::EnemyBase() : Hero(Hero::Get())
    {
        CollisionComp = ETG::CreateGameObjectAttached<CollisionComponent>(this);
        CollisionComp->CollisionVisualizationColor = ETG::Color::Magenta;

        //Only projectiles. The hero watches for us rather than the other way round, and items waiting on the
        //floor watch for us, so an enemy that walks over one still picks it up
        CollisionComp->Layer = CollisionLayer::Enemy;
        CollisionComp->Mask = CollisionLayer::Projectile;

        CollisionComp->SetCollisionEnabled(true);

        MoveComp = ETG::CreateGameObjectAttached<EnemyMoveCompBase>(this);
        MoveComp->Initialize();

        HealthComp = ETG::CreateGameObjectAttached<BaseHealthComp>(this, 30.f);

        ShaderEffectComp = ETG::CreateGameObjectAttached<ShaderEffectComponent>(this);

        //Not Initialize(): that one runs again from every concrete enemy's constructor, and binding from there
        //registered a second copy of every listener. A constructor runs exactly once per object, so binding here
        //cannot be duplicated no matter how many times Initialize is called
        BindEvents();
    }

    EnemyBase::~EnemyBase() = default;

    //Deliberately empty. Everything that used to live here is event binding, which is now the constructor's job -
    //see BindEvents. What belongs here is per-type tuning that is safe to re-run, the way BulletMan::Initialize
    //sets its movement numbers
    void EnemyBase::Initialize()
    {
    }

    void EnemyBase::BindEvents()
    {
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

            //Cleared before the force below and not after it, which is the order this used to be in: ApplyForce
            //broadcasts OnForceStart, whose listener sets the state to Hit, so the death force was undoing the
            //SetState(Die) two lines above it and the guard at the top of this listener had nothing to catch
            MoveComp->OnForceStart.Clear();
            MoveComp->OnForceEnd.Clear();
            HealthComp->OnDamageTaken.Clear();
            CollisionComp->SetCollisionEnabled(false);

            // Set enemy state to die
            SetState(EnemyStateEnum::Die);
            Depth = std::numeric_limits<float>::max(); //set depth to max value so that it will be drawn bottom of everything
            ETG::Vector2f knockbackDir;
            
            if (const auto* proj = instigator->As<ProjectileBase>())
                knockbackDir = Math::RadianToDirection(Math::AngleToRadian(proj->GetRotation()));
            else 
                knockbackDir = Math::Normalize(Position - instigator->GetPosition());
            
            std::cout << "During Death: " << knockbackDir.x << " " << knockbackDir.y << std::endl;

            MoveComp->ApplyForce(knockbackDir, KnockBackMagnitudeForDeath, KnockBackDurationForDeath);
        });

        //In the future, enemy will take damage from explosive environment %
        HealthComp->OnDamageTaken.AddListener([this](const float damage, const float forceMagnitude, const GameObjectBase* instigator)
        {
            //The flash goes first and is unconditional: it is the one piece of feedback every hit gets,
            //however the enemy reacts to it otherwise. Retriggering is the component's problem, which is
            //what lets an automatic weapon land ten of these without them piling up
            ShaderEffectComp->PlayHitFlash();

            if (KnockBackOnHit)
                HandleHitForce(instigator->As<ProjectileBase>());
        });
    }

    void EnemyBase::Update()
    {
        //Ahead of everything else on purpose. A flash started by a bullet that landed in the collision pass -
        //which is CollisionSystem's, at the end of the frame, after every object's Update has run - has to
        //survive until this object's draw properties are published, so it is only aged from the tick after the
        //one it started in
        ShaderEffectComp->Update();

        MoveComp->Update();
        HealthComp->Update();

        GameObjectBase::Update();
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
        // Check if this is a hero projectile
        // Calculate force direction (from projectile to enemy)
        // "A'dan B'ye giden vektör" = B - A.
        const ETG::Vector2f  forceDirection = Math::RadianToDirection(Math::AngleToRadian(projectile->GetRotation()));

        std::cout << forceDirection.x << " " << forceDirection.y << std::endl;

        // Get force from projectile
        const float forceMagnitude = projectile->Force;

        auto* Gun = projectile->Owner->As<GunBase>();
        
        // Apply the force
        MoveComp->ApplyForce(forceDirection, forceMagnitude,  Gun->ForceDuration);
    }

    void EnemyBase::Draw()
    {
        GameObjectBase::Draw();
        
        if (!IsVisible) return;
        
        SpriteBatch::Draw(GetDrawProperties());
        if (CollisionComp) CollisionComp->Visualize(*RenderContext::Window);

        //Draw every equipped gun (the holstered ones only draw their projectiles), same as the hero
        for (GunBase* gun : EquippedGuns)
            if (gun) gun->Draw();

        Hand->Draw();
        OffHand->Draw();
    }
}

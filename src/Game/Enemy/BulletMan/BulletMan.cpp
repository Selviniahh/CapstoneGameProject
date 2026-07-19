#include "../../../Engine/Managers/Time.h"
#include "BulletMan.h"
#include <filesystem>
#include "../../../Engine/Platform/Platform.h"
#include "../../../Engine/Core/Factory.h"
#include "../../../Engine/Managers/SpriteBatch.h"
#include "../../../Utils/Math.h"
#include "Components/BulletManAnimComp.h"
#include "../../../Engine/Core/Components/CollisionComponent.h"
#include "../../Characters/Hand/Hand.h"
#include "../../Guns/Base/GunBase.h"
#include "../../Guns/Magnum/Magnum.h"
#include "../../../Engine/Core/Components/BaseHealthComp.h"
#include "../../../Engine/Managers/RenderContext.h"
#include "../../../Engine/Managers/AssetManager.h"

namespace ETG
{
    class Magnum;
}

ETG::BulletMan::BulletMan(const ETG::Vector2f& position)
{
    this->Position = position;
    Depth = 4;
    
    BulletMan::Initialize();

    Hand = ETG::CreateGameObjectAttached<class Hand>(this);

    // Initialize animation component
    AnimationComp = ETG::CreateGameObjectAttached<BulletManAnimComp>(this);
    AnimationComp->Initialize();
    AnimationComp->Update();

    Gun = ETG::CreateGameObjectAttached<Magnum>(this, Hand->GetRelativePosition());
    Gun->Initialize();
    Gun->ProjTexture->loadFromFile(AssetManager::Resolve("Projectiles/Enemy/8x8_enemy_projectile_001.png"));
}

ETG::BulletMan::~BulletMan() = default;

void ETG::BulletMan::Initialize()
{
    EnemyBase::Initialize();
    
    MoveComp->DetectionRadius = 200.0f;
    MoveComp->StopDistance = 150.0f;
    MoveComp->MovementSpeed = 40.0f;
    MoveComp->MaxSpeed = 100.0f;
    MoveComp->Acceleration = 10.0f;
    MoveComp->Deceleration = 5000.0f;
}

void ETG::BulletMan::Update()
{
    EnemyBase::Update();
    CollisionComp->Update();

    UpdateAnimations();
    UpdateHandAndGunPositions();
    UpdateShooting();
    UpdateVisibility();
}

void ETG::BulletMan::UpdateAnimations()
{
    // Update animation Flip sprites based on direction
    if (CanFlipAnims()) AnimationComp->FlipSpritesX(EnemyDir, *this);
    if (CanFlipAnims()) AnimationComp->FlipSpritesY<GunBase>(EnemyDir, *Gun);
    
    AnimationComp->Update();
}

void ETG::BulletMan::UpdateHandAndGunPositions() const
{
    //Set hand properties
    const ETG::Vector2f HandOffsetForHero = AnimationComp->IsFacingRight(EnemyDir) ? 
        ETG::Vector2f{8.f, 5.f} : ETG::Vector2f{-8.f, 5.f};
    Hand->SetPosition(Position + Hand->HandOffset + HandOffsetForHero);
    Hand->Update();

    if (Hand && Gun)
    {
        // Position the gun relative to the hand.
        const ETG::Vector2f handPos = Hand->GetPosition();
        Gun->SetPosition(handPos + Hand->GunOffset);

        // Aim the gun toward the hero.
        const float angle = Math::AngleBetween(handPos, Hero->GetPosition());
        Gun->Rotation = angle;
    }
    
    Gun->Update();
}

void ETG::BulletMan::UpdateShooting()
{
    // Decrement the attack timer
    if (attackCooldownTimer > 0)
    {
        attackCooldownTimer -= Time::FrameTick;
    }

    // Make actual shooting happen
    if (GetState() == EnemyStateEnum::Shooting)
    {
        Gun->PrepareShooting();
    }

    // If the gun is not shooting and the animation is finished, set the state to idle
    if (GetState() == EnemyStateEnum::Shooting && Gun->CurrentGunState != GunStateEnum::Shoot &&
        Gun->GetAnimationInterface()->GetAnimation()->IsAnimationFinished())
    {
        SetState(EnemyStateEnum::Idle);
    }
    
    // BulletMan-specific shooting logic needs to be called after checking state transitions
    BulletManShoot();
}

void ETG::BulletMan::UpdateVisibility() const
{
    Gun->IsVisible = CanFlipAnims();
    Hand->IsVisible = CanFlipAnims();
}

void ETG::BulletMan::BulletManShoot()
{
    // Don't shoot if being forced/hit/dead
    if (!CanShoot()) return;

    //If the gun is shooting, we have to set enemy's animation to be shooting as well
    if (Gun->CurrentGunState == GunStateEnum::Shoot && !Gun->GetAnimationInterface()->GetAnimation()->IsAnimationFinished())
    {
        SetState(EnemyStateEnum::Shooting);
        return;
    }

    if (attackCooldownTimer <= 0)
    {
        // In attack range and cooldown finished, enter shooting state
        SetState(EnemyStateEnum::Shooting);
        attackCooldownTimer = attackCooldown; // Reset cooldown
    }
}

void ETG::BulletMan::HandleHitForce(const ProjectileBase* projectile)
{
    EnemyBase::HandleHitForce(projectile);
    SetState(EnemyStateEnum::Hit);
}

void ETG::BulletMan::Draw()
{
    if (!IsVisible) return;
    EnemyBase::Draw();
    SpriteBatch::Draw(GetDrawProperties());
    if (CollisionComp) CollisionComp->Visualize(*RenderContext::Window);

    Gun->Draw();
    Hand->Draw();
}

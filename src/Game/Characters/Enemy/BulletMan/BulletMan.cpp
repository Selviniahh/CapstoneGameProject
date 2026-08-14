#include "../../../../Engine/Managers/Time.h"
#include "BulletMan.h"
#include <filesystem>
#include "../../../../Engine/Platform/Platform.h"
#include "../../../../Engine/Core/Factory.h"
#include "../../../../Engine/Managers/SpriteBatch.h"
#include "../../../../Utils/Math.h"
#include "Components/BulletManAnimComp.h"
#include "../../../../Engine/Core/Components/CollisionComponent.h"
#include "../../Hero/Hand/Hand.h"
#include "../../../Guns/Base/GunBase.h"
#include "../../../Guns/Magnum/Magnum.h"
#include "../../../../Engine/Core/Components/BaseHealthComp.h"
#include "../../../../Engine/Managers/RenderContext.h"
#include "../../../../Engine/Managers/AssetManager.h"
#include "../../../../Utils/DirectionUtils.h"

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
    OffHand = ETG::CreateGameObjectAttached<class Hand>(this);

    // Initialize animation component
    AnimationComp = ETG::CreateGameObjectAttached<BulletManAnimComp>(this);
    AnimationComp->Initialize();
    AnimationComp->Update();

    Gun = ETG::CreateGameObjectAttached<Magnum>(this, Hand->GetRelativePosition());
    Gun->Initialize();
    //Reassign instead of loading in place: ProjTexture is shared through the texture cache, loading
    //into it would overwrite the hero's Magnum projectile texture too.
    Gun->ProjTexture = AssetManager::LoadTexture("Projectiles/Enemy/8x8_enemy_projectile_001.png");

    //Hands the weapon to Character's inventory, which is what makes CurrentGun / GetCurrentHoldingGun work for an
    //enemy the same way they do for the hero - so an active item picked up by this enemy finds a gun to modify
    EquipGun(Gun.get());
}

ETG::BulletMan::~BulletMan() = default;

void ETG::BulletMan::Initialize()
{
    EnemyBase::Initialize();

    EnemyMoveCompBase* moveComp = GetMoveComp();
    moveComp->DetectionRadius = 200.0f;
    moveComp->StopDistance = 150.0f;
    moveComp->MovementSpeed = 40.0f;
    moveComp->MaxSpeed = 100.0f;
    moveComp->Acceleration = 10.0f;
    moveComp->Deceleration = 5000.0f;
}

void ETG::BulletMan::Update()
{
    EnemyBase::Update();

    UpdateAnimations();

    //Same three steps the hero runs, in the same order: place the hand, aim from where the hand ended up, then move
    //the gun onto it
    UpdateHoldPoint();
    UpdateAim();
    UpdateGuns();

    UpdateShooting();
    UpdateHandAndGunVisibility();
}

void ETG::BulletMan::UpdateAnimations()
{
    // Update animation Flip sprites based on direction. The held gun is not flipped here: it follows the hand it
    // is in, which Character::UpdateGuns decides further down this same tick
    if (CanFlipAnims()) AnimationComp->FlipSpritesX(CurrentDir, *this);

    AnimationComp->Update();
}

//The enemy's counterpart of the hero's mouse angle: it aims at whatever it is hunting
void ETG::BulletMan::UpdateAim()
{
    if (!Hand || !Hero) return;

    //Aims from the hold point rather than from the hand: the hands now sit on the gun, which is aimed by this very
    //angle, so reading one of them back here would be a loop feeding on its own output
    AimAngle = Math::AngleBetween(HoldPoint, Hero->GetPosition());
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
        Gun->GetAnimationInterface()->GetAnimation()->IsFinished())
    {
        SetState(EnemyStateEnum::Idle);
    }

    // BulletMan-specific shooting logic needs to be called after checking state transitions
    BulletManShoot();
}

void ETG::BulletMan::BulletManShoot()
{
    // Don't shoot if being forced/hit/dead
    if (!CanShoot()) return;

    //If the gun is shooting, we have to set enemy's animation to be shooting as well
    if (Gun->CurrentGunState == GunStateEnum::Shoot && !Gun->GetAnimationInterface()->GetAnimation()->IsFinished())
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
    EnemyBase::Draw();
    
}
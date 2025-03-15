#include "BulletMan.h"
#include <filesystem>
#include <cmath>
#include "Components/BulletManAnimComp.h"
#include "../../Core/Factory.h"
#include "../../Managers/DebugTexts.h"
#include "../../Managers/SpriteBatch.h"
#include "../../Utils/Math.h"
#include "../../Utils/StrManipulateUtil.h"
#include "../../Characters/Hand/Hand.h"
#include "../../Managers/Globals.h"

ETG::BulletMan::BulletMan(const sf::Vector2f& position)
{
    this->Position = position;
    BulletManState = EnemyStateEnum::Idle; // Initialize state
    BulletManDir = Direction::Right; // Initialize direction
    Depth = 2; // Set depth like Hero does
    shootTimer = 0.0f; // Initialize shoot timer
}

ETG::BulletMan::~BulletMan() = default;

void ETG::BulletMan::Initialize()
{
    EnemyBase::Initialize();

    // Create hand first (needed for gun positioning)
    Hand = ETG::CreateGameObjectAttached<class Hand>(this, (std::filesystem::path(RESOURCE_PATH) / "Enemy" / "BulletMan" / "bullet_hand_001.png").generic_string());
    
    // Create animation component and gun
    AnimationComp = ETG::CreateGameObjectAttached<BulletManAnimComp>(this);
    Magnum = ETG::CreateGameObjectAttached<class Magnum>(this, Hand->GetRelativePosition());
    
    AnimationComp->Initialize();
    AnimationComp->Update();

    MoveComp = ETG::CreateGameObjectAttached<BaseMoveComp>(this, 50, 40, 40);

    // Don't load texture directly - let the animation component handle it
    EnemyBase::Initialize();
}

void ETG::BulletMan::Update()
{
    EnemyBase::Update(); // Start with base update

    // Move Component
    MoveComp->Update();
    MoveTowardsHero();

    // Update direction to hero
    BulletManDir = DirectionUtils::GetDirectionToHero(Hero, Position);

    // Debug texts on left window
    const auto DirectionSelfToHero = Math::Normalize(Hero->GetPosition() - Position);
    DebugTextManager::QueueText("DirectionSelfToHero " + std::to_string(DirectionSelfToHero.x) + "Y: " + std::to_string(DirectionSelfToHero.y));
    DebugTextManager::QueueText("Enemy Direction: " + std::string(EnumToString(BulletManDir)));

    // Update the state based on movement
    BulletManState = (MoveComp->IsMoving) ? EnemyStateEnum::Idle : EnemyStateEnum::Idle; // Keep Idle for now

    // Flip sprites
    AnimationComp->FlipSpritesY<class Magnum>(BulletManDir, *Magnum);
    AnimationComp->FlipSpritesX(BulletManDir, *this);

    // Update hand position
    HandOffsetForBulletMan = AnimationComp->IsFacingRight(BulletManDir)
        ? sf::Vector2f{9.f, 5.f}
        : sf::Vector2f{-8.f, 5.f};

    Hand->SetPosition(Position + Hand->HandOffset + HandOffsetForBulletMan);
    
    // Flip hand appropriately
    Hand->SetScale(sf::Vector2f(
        AnimationComp->IsFacingRight(BulletManDir) ? 1.0f : -1.0f,
        1.0f
    ));

    AnimationComp->Update();

    if (AnimationComp->CurrentTex)
    {
        Texture = AnimationComp->CurrentTex;
    }

    Hand->Update();

    // Update the gun
    Magnum->SetPosition(Hand->GetPosition() + Hand->GunOffset);
    
    // Calculate rotation to point at the hero
    sf::Vector2f dirToHero = Hero->GetPosition() - Magnum->GetPosition();
    float angleToHero = atan2f(dirToHero.y, dirToHero.x) * 180.0f / M_PI;
    Magnum->Rotation = angleToHero;
    
    // Shooting logic with animation coordination
    if (Magnum->CurrentGunState == GunStateEnum::Shoot)
    {
        // Gun is already in shooting animation
        // Check if the animation has finished
        if (Magnum->AnimationComp && static_cast<MagnumAnimComp*>(Magnum->AnimationComp.get())->IsAnimationFinished())
        {
            // Animation completed, actually fire the projectile
            Magnum->Shoot();
            DebugTextManager::QueueText("Enemy fired!");
            // Reset to idle state
            Magnum->CurrentGunState = GunStateEnum::Idle;
        }
    }
    else
    {
        // Only start shooting sequence if not already shooting
    shootTimer += ETG::Globals::FrameTick;
    if (shootTimer >= shootInterval)
    {
            // Reset timer and start shooting animation
        shootTimer = 0.0f;
            Magnum->CurrentGunState = GunStateEnum::Shoot; // This will trigger animation change
        }
    }
    
    Magnum->Update();

    EnemyBase::Update();
}

void ETG::BulletMan::Draw()
{
    EnemyBase::Draw();
    Magnum->Draw(); // Make sure to draw the gun
    Hand->Draw();
    SpriteBatch::Draw(GetDrawProperties());
}

void ETG::BulletMan::MoveTowardsHero()
{
    auto Dist = Hero->GetPosition() - Position;

    //If enemy is too close to hero, set velocity to 0 and return. 
    if (Math::Length(Dist) < DistanceRange)
    {
        MoveComp->Velocity = {0, 0};
        return;
    }

    Dist = Math::Normalize(Dist);
    MoveComp->UpdateMovement(Dist, Position);
}
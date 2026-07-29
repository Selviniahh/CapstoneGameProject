#include "BulletManAnimComp.h"
#include "../BulletMan.h"
#include "../BulletManDirections.h"

ETG::BulletManAnimComp::BulletManAnimComp()
{
    IsGameObjectUISpecified = true;
    BulletMan = nullptr;
    BulletManAnimComp::SetAnimations();
}

ETG::BulletManAnimComp::~BulletManAnimComp() = default;

void ETG::BulletManAnimComp::Initialize()
{
    BaseAnimComp<EnemyStateEnum>::Initialize();

    // Set initial state here
    if (Owner)
    {
        BulletMan = dynamic_cast<class BulletMan*>(Owner);
        if (BulletMan)
        {
            CurrentState = BulletMan->GetState();
            CurrentAnimStateKey = BulletManDirections::GetIdleEnum(BulletMan->CurrentDir);
        }
    }
}

void ETG::BulletManAnimComp::SetAnimations()
{
    BaseAnimComp::SetAnimations();

    // Idle Animation
    const auto idleAnims = std::vector<Animation>{
        Animation::CreateSpriteSheet("Enemy/BulletMan/Idle", "bullet_idle_back_001", "png", 0.15f),
        Animation::CreateSpriteSheet("Enemy/BulletMan/Idle", "bullet_idle_right_001", "png", 0.15f),
        Animation::CreateSpriteSheet("Enemy/BulletMan/Idle", "bullet_idle_right_001", "png", 0.15f),
    };
    AddAnimationsForState<BulletManIdleEnum>(EnemyStateEnum::Idle, Playback::Loop, idleAnims);

    // Run animation
    const auto runAnims = std::vector<Animation>{
        Animation::CreateSpriteSheet("Enemy/BulletMan/Run", "bullet_run_left_001", "png", 0.12f),
        Animation::CreateSpriteSheet("Enemy/BulletMan/Run", "bullet_run_left_back_001", "png", 0.12f),
        Animation::CreateSpriteSheet("Enemy/BulletMan/Run", "bullet_run_right_001", "png", 0.12f),
        Animation::CreateSpriteSheet("Enemy/BulletMan/Run", "bullet_run_right_back_001", "png", 0.12f),
    };
    AddAnimationsForState<BulletManRunEnum>(EnemyStateEnum::Run, Playback::Loop, runAnims);

    // Shooting animation
    auto shootingAnims = std::vector<Animation>{
        Animation::CreateSpriteSheet("Enemy/BulletMan/Shooting", "bullet_shooting_left_001", "png", 0.1f),
        Animation::CreateSpriteSheet("Enemy/BulletMan/Shooting", "bullet_shooting_right_001", "png", 0.1f),
    };
    AddAnimationsForState<BulletManShootingEnum>(EnemyStateEnum::Shooting, Playback::Once, shootingAnims);

    // Hit animation
    auto hitAnims = std::vector<Animation>{
        Animation::CreateSpriteSheet("Enemy/BulletMan/Hit", "bullet_hit_back_left_001", "png", 0.08f),
        Animation::CreateSpriteSheet("Enemy/BulletMan/Hit", "bullet_hit_back_right_001", "png", 0.08f),
        Animation::CreateSpriteSheet("Enemy/BulletMan/Hit", "bullet_hit_left_001", "png", 0.08f),
        Animation::CreateSpriteSheet("Enemy/BulletMan/Hit", "bullet_hit_right_001", "png", 0.08f),
    };
    AddAnimationsForState<BulletManHitEnum>(EnemyStateEnum::Hit, Playback::Once, hitAnims);

    // Death animation
    auto DeathAnims = std::vector<Animation>{
        Animation::CreateSpriteSheet("Enemy/BulletMan/Death", "bullet_death_back_south_001", "png", 0.08f),
        Animation::CreateSpriteSheet("Enemy/BulletMan/Death", "bullet_death_front_north_001", "png", 0.08f),
        Animation::CreateSpriteSheet("Enemy/BulletMan/Death", "bullet_death_left_back_001", "png", 0.08f),
        Animation::CreateSpriteSheet("Enemy/BulletMan/Death", "bullet_death_left_front_001", "png", 0.08f),
        Animation::CreateSpriteSheet("Enemy/BulletMan/Death", "bullet_death_left_side_001", "png", 0.08f),
        Animation::CreateSpriteSheet("Enemy/BulletMan/Death", "bullet_death_right_back_001", "png", 0.08f),
        Animation::CreateSpriteSheet("Enemy/BulletMan/Death", "bullet_death_right_front_001", "png", 0.08f),
        Animation::CreateSpriteSheet("Enemy/BulletMan/Death", "bullet_death_right_side_001", "png", 0.08f),
    };
    AddAnimationsForState<BulletManDeathEnum>(EnemyStateEnum::Die, Playback::Once, DeathAnims);
}

void ETG::BulletManAnimComp::Update()
{
    // Ensure we have a valid BulletMan pointer
    if (!BulletMan)
    {
        BulletMan = dynamic_cast<class BulletMan*>(Owner);
        if (!BulletMan) return; // Safety check
    }

    AnimationKey newKey;

    // Set key based on state (similar to HeroAnimComp approach)
    switch (BulletMan->GetState())
    {
    case EnemyStateEnum::Idle:
        newKey = BulletManDirections::GetIdleEnum(BulletMan->CurrentDir);
        break;

    case EnemyStateEnum::Run:
        newKey = BulletManDirections::GetRunEnum(BulletMan->CurrentDir);
        break;

    case EnemyStateEnum::Shooting:
        newKey = BulletManDirections::GetShootingEnum(BulletMan->CurrentDir);
        break;

    case EnemyStateEnum::Hit:
        newKey = BulletManDirections::GetHitEnum(BulletMan->CurrentDir);
        break;

    case EnemyStateEnum::Die:
        newKey = BulletManDirections::GetDeathEnum(BulletMan->CurrentDir);
        break;

    default:
        newKey = BulletManIdleEnum::Idle_Back; // Default fallback
        break;
    }

    // Update base animation component with current state and key
    BaseAnimComp<EnemyStateEnum>::Update(BulletMan->GetState(), newKey);

    //NOTE: the death animations are registered as one-shots, so they stop on their last frame by
    //themselves. This used to need a PlayOnlyLastFrame call here on every tick.
}

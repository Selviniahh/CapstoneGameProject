#include "BulletManAnimComp.h"
#include "../BulletMan.h"
#include "../../../Utils/DirectionUtils.h"

ETG::BulletManAnimComp::BulletManAnimComp()
{
    IsGameObjectUISpecified = false;
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
            CurrentState = BulletMan->BulletManState;
            CurrentAnimStateKey = DirectionUtils::GetBulletManIdleEnum(BulletMan->BulletManDir);
        }
    }
}

void ETG::BulletManAnimComp::SetAnimations()
{
    BaseAnimComp::SetAnimations();

    //Idle Animation
    const auto idleAnims = std::vector<Animation>{
        Animation::CreateSpriteSheet("Enemy/BulletMan/Idle", "bullet_idle_back_001", "png", 0.15f),
        Animation::CreateSpriteSheet("Enemy/BulletMan/Idle", "bullet_idle_right_001", "png", 0.15f),
        Animation::CreateSpriteSheet("Enemy/BulletMan/Idle", "bullet_idle_right_001", "png", 0.15f),
    };
    AddAnimationsForState<BulletManIdleEnum>(EnemyStateEnum::Idle,idleAnims);

    //Run animation
    const auto RunAnims = std::vector<Animation>{
        Animation::CreateSpriteSheet("Enemy/BulletMan/Run", "bullet_run_left_001", "png", 0.15f),
        Animation::CreateSpriteSheet("Enemy/BulletMan/Run", "bullet_run_left_back_001", "png", 0.15f),
        Animation::CreateSpriteSheet("Enemy/BulletMan/Run", "bullet_run_right_001", "png", 0.15f),
        Animation::CreateSpriteSheet("Enemy/BulletMan/Run", "bullet_run_right_back_001", "png", 0.15f),
    };
    AddAnimationsForState<BulletManRunEnum>(EnemyStateEnum::Run,RunAnims);
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

    // Set key based on state (identical to HeroAnimComp approach)
    if (BulletMan->BulletManState == EnemyStateEnum::Idle)
        newKey = DirectionUtils::GetBulletManIdleEnum(BulletMan->BulletManDir);
    
    else if (BulletMan->BulletManState == EnemyStateEnum::Run)
        newKey = DirectionUtils::GetBulletManRunEnum(BulletMan->BulletManDir);
    else
        newKey = BulletManIdleEnum::Idle_Back; // Default fallback

    // If key has changed, update and restart animation (just like HeroAnimComp)
    if (newKey != CurrentAnimStateKey)
    {
        CurrentAnimStateKey = newKey;

        //Restart the key
        auto& animManager = AnimManagerDict[CurrentState];
        if (animManager.AnimationDict.contains(CurrentAnimStateKey))
        {
            animManager.AnimationDict[CurrentAnimStateKey].Restart();
        }
    }

    // Update base animation component with current state and key (same as HeroAnimComp)
    BaseAnimComp<EnemyStateEnum>::Update(BulletMan->BulletManState, CurrentAnimStateKey);
}

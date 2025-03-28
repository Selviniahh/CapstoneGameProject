#include "HeroAnimComp.h"

#include "HeroMoveComp.h"
#include "InputComponent.h"
#include "../../Guns/Base/GunBase.h"
#include "../../Managers/GameState.h"
#include "../../Characters/Hero.h"

namespace ETG
{
    class RogueSpecial;
}

namespace ETG
{
    HeroAnimComp::HeroAnimComp()
    {
        HeroPtr = GameState::GetInstance().GetHero();
        IsGameObjectUISpecified = true;
        HeroAnimComp::SetAnimations();
        CurrentState = HeroPtr->CurrentHeroState;
    }

    void HeroAnimComp::SetAnimations()
    {
        BaseAnimComp::SetAnimations();

        //Run
        const auto runAnims = std::vector<Animation>{
            Animation::CreateSpriteSheet("Player/Run/Back", "rogue_run_back_hands_001", "png", RunAnimFrameInterval),
            Animation::CreateSpriteSheet("Player/Run/BackWard", "rogue_run_backward_001", "png", RunAnimFrameInterval),
            Animation::CreateSpriteSheet("Player/Run/Forward", "rogue_run_forward_hands_001", "png", RunAnimFrameInterval),
            Animation::CreateSpriteSheet("Player/Run/Front", "rogue_run_front_hands_001", "png", RunAnimFrameInterval),
        };
        AddAnimationsForState<HeroRunEnum>(HeroStateEnum::Run,runAnims);

        //Idle
        const auto idleAnims = std::vector<Animation>{
            Animation::CreateSpriteSheet("Player/Idle/Back", "rogue_idle_back_hand_left_001", "png", IdleAnimFrameInterval),
            Animation::CreateSpriteSheet("Player/Idle/BackWard", "rogue_idle_backwards_001", "png", IdleAnimFrameInterval),
            Animation::CreateSpriteSheet("Player/Idle/Front", "rogue_idle_front_hand_left_001", "png", IdleAnimFrameInterval),
            Animation::CreateSpriteSheet("Player/Idle/Right", "rogue_idle_hands_001", "png", IdleAnimFrameInterval),
        };
        const auto idleEnumValues = ConstructEnumVector<HeroIdleEnum>();
        AddAnimationsForState<HeroIdleEnum>(HeroStateEnum::Idle,idleAnims);
        
        //NOTE: Commented dash because dash not implemented yet.
        const auto dashAnims = std::vector<Animation>{
            Animation::CreateSpriteSheet("Player/Dash/Back", "rogue_dodge_back_001", "png", DashAnimFrameInterval), //Dash_Back 0.62
            Animation::CreateSpriteSheet("Player/Dash/BackWard", "rogue_dodge_left_back_001", "png", DashAnimFrameInterval), //Dash_Backward 0.62
            Animation::CreateSpriteSheet("Player/Dash/Front", "rogue_dodge_front_001", "png", DashAnimFrameInterval), //Dash_Front will take 0.62
            Animation::CreateSpriteSheet("Player/Dash/Right", "rogue_dodge_left_001", "png", DashAnimFrameInterval), //Dash_Left  Will take: 0.466159 seconds
            Animation::CreateSpriteSheet("Player/Dash/Right", "rogue_dodge_left_001", "png", DashAnimFrameInterval), //Dash_Right Will take: 0.466159 seconds
        };
        AddAnimationsForState<HeroDashEnum>(HeroStateEnum::Dash, dashAnims);
    }

    void HeroAnimComp::StartDash(HeroDashEnum direction)
    {
        if (IsDashing) return;
        
        IsDashing = true;
        DashTimer = 0.f; //Reset the timer
        CurrentDashDirection = direction;

        //Explicitly restart the animation to ensure it plays fully
        auto& anim = HeroPtr->AnimationComp->AnimManagerDict[HeroStateEnum::Dash].AnimationDict[direction];
        anim.FrameInterval = DashAnimFrameInterval;
        anim.Restart();
        HeroPtr->CurrentHeroState = HeroStateEnum::Dash;
        OnDashStart.Broadcast(direction);
    }

    //NOTE: IF we called this function in Update before the base Update, We would have to manually reset Dash animation at here
    void HeroAnimComp::EndDash()
    {
        if (!IsDashing) return;
        IsDashing = false;
        HeroPtr->MoveComp->StartDashCooldown();
        OnDashEnd.Broadcast();
    }

    bool HeroAnimComp::IsDashAnimFinished() const
    {
        if (!HeroPtr || !HeroPtr->AnimationComp) return true;
        return HeroPtr->AnimationComp->AnimManagerDict[HeroStateEnum::Dash].IsAnimationFinished();
    }

    void HeroAnimComp::Update()
    {
        AnimationKey newKey;

        if (HeroPtr->CurrentHeroState == HeroStateEnum::Idle)
        {
            newKey = DirectionUtils::GetHeroIdleDirectionEnum(HeroPtr->CurrentDirection);
            HeroPtr->AnimationComp->AnimManagerDict[HeroStateEnum::Idle].AnimationDict[newKey].FrameInterval = IdleAnimFrameInterval;
        }

        else if (HeroPtr->CurrentHeroState == HeroStateEnum::Run)
        {
            newKey = DirectionUtils::GetHeroRunEnum(HeroPtr->CurrentDirection);
            HeroPtr->AnimationComp->AnimManagerDict[HeroStateEnum::Run].AnimationDict[newKey].FrameInterval = RunAnimFrameInterval;
        }

        else if (HeroPtr->CurrentHeroState == HeroStateEnum::Dash)
        {
            newKey = CurrentDashDirection;
            DashTimer += Globals::FrameTick;
        }
        
        //NOTE: I know calling base here and then executing rest of the codes looks weird however, during state changes the base animation's `ChangeAnimStateIfRequired` will restart animation if state has ever changes
        //If the dash functionality below executed before this base, the dash animation will not restart so after executing once `IsDashAnimFinished` will always return true forever
        //One option is to extract  `ChangeAnimStateIfRequired` from base and call here however I believe in every anim state change, generally all animations needs to restart so I don't wanna move a generic function into it's child 
        BaseAnimComp<HeroStateEnum>::Update(HeroPtr->CurrentHeroState, newKey);

        //If Dash animation is complete, Stop Dash and set Current state to idle
        if (IsDashing && DashTimer >= MinDashDuration && IsDashAnimFinished())
        {
            EndDash();
        }
    }
}

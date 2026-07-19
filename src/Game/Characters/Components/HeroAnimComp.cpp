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
        CurrentState = HeroPtr->GetState();
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
        AddAnimationsForState<HeroRunEnum>(HeroStateEnum::Run, runAnims);

        //Idle
        const auto idleAnims = std::vector<Animation>{
            Animation::CreateSpriteSheet("Player/Idle/Back", "rogue_idle_back_hand_left_001", "png", IdleAnimFrameInterval),
            Animation::CreateSpriteSheet("Player/Idle/BackWard", "rogue_idle_backwards_001", "png", IdleAnimFrameInterval),
            Animation::CreateSpriteSheet("Player/Idle/Front", "rogue_idle_front_hand_left_001", "png", IdleAnimFrameInterval),
            Animation::CreateSpriteSheet("Player/Idle/Right", "rogue_idle_hands_001", "png", IdleAnimFrameInterval),
        };
        const auto idleEnumValues = ConstructEnumVector<HeroIdleEnum>();
        AddAnimationsForState<HeroIdleEnum>(HeroStateEnum::Idle, idleAnims);

        //Dash
        const auto dashAnims = std::vector<Animation>{
            Animation::CreateSpriteSheet("Player/Dash/Back", "rogue_dodge_back_001", "png", DashAnimFrameInterval), //Dash_Back 0.62
            Animation::CreateSpriteSheet("Player/Dash/BackWard", "rogue_dodge_left_back_001", "png", DashAnimFrameInterval), //Dash_Backward 0.62
            Animation::CreateSpriteSheet("Player/Dash/Front", "rogue_dodge_front_001", "png", DashAnimFrameInterval), //Dash_Front will take 0.62
            Animation::CreateSpriteSheet("Player/Dash/Right", "rogue_dodge_left_001", "png", DashAnimFrameInterval), //Dash_Left  Will take: 0.466159 seconds
            Animation::CreateSpriteSheet("Player/Dash/Right", "rogue_dodge_left_001", "png", DashAnimFrameInterval), //Dash_Right Will take: 0.466159 seconds
        };
        AddAnimationsForState<HeroDashEnum>(HeroStateEnum::Dash, dashAnims);

        //Hit (Because there's no hit animation, spin animation will be used)
        const auto spinAnim = std::vector<Animation>{
            Animation::CreateSpriteSheet("Player/Spin", "rogue_spin_001", "png", 0.10),
        };
        AddAnimationsForState<HeroHit>(HeroStateEnum::Hit, spinAnim);

        //Death 
        const auto DeathAnim = std::vector<Animation>{
            Animation::CreateSpriteSheet("Player/ShotDeath", "rogue_shot_death_001", "png", 0.10),
        };
        AddAnimationsForState<HeroDeath>(HeroStateEnum::Die, DeathAnim);
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
        HeroPtr->SetState(HeroStateEnum::Dash);
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

        switch (HeroPtr->GetState())
        {
        case HeroStateEnum::Idle:
            newKey = DirectionUtils::GetHeroIdleDirectionEnum(HeroPtr->CurrentDirection);
            HeroPtr->AnimationComp->AnimManagerDict[HeroStateEnum::Idle].AnimationDict[newKey].FrameInterval = IdleAnimFrameInterval;
            break;

        case HeroStateEnum::Run:
            newKey = DirectionUtils::GetHeroRunEnum(HeroPtr->CurrentDirection);
            HeroPtr->AnimationComp->AnimManagerDict[HeroStateEnum::Run].AnimationDict[newKey].FrameInterval = RunAnimFrameInterval;
            break;

        case HeroStateEnum::Dash:
            newKey = CurrentDashDirection;
            DashTimer += Globals::FrameTick;
            break;

        case HeroStateEnum::Hit:
            newKey = HeroHit::JustHit;
            break;

        case HeroStateEnum::Die:
            newKey = HeroDeath::Dead;
            break;

        default:
            break;
        }

        //NOTE: I know calling base here and then executing rest of the codes looks weird however, during state changes the base animation's `ChangeAnimStateIfRequired` will restart animation if state has ever changes
        //If the dash functionality below executed before this base, the dash animation will not restart so after executing once `IsDashAnimFinished` will always return true forever
        //One option is to extract  `ChangeAnimStateIfRequired` from base and call here however I believe in every anim state change, generally all animations needs to restart so I don't wanna move a generic function into it's child 
        BaseAnimComp<HeroStateEnum>::Update(HeroPtr->GetState(), newKey);

        //If Dash animation is complete, Stop Dash and set Current state to idle
        if (IsDashing && DashTimer >= MinDashDuration && IsDashAnimFinished())
        {
            EndDash();
        }

        //After hit, hit animation needs to be played once and then it will be set to idle
        if (HeroPtr->GetState() == HeroStateEnum::Hit && HeroPtr->AnimationComp->AnimManagerDict[HeroStateEnum::Hit].IsAnimationFinished())
        {
            //Restart current hit animation otherwise after first play, above IsAnimationFinished will always return true
            HeroPtr->AnimationComp->AnimManagerDict[HeroStateEnum::Hit].CurrentAnim->Restart();

            //After hit animation is finished, set idle state 
            HeroPtr->SetState(HeroStateEnum::Idle);
        }

        //NOTE: If hero is dead and animation is finished, set the animation to last frame. Hero's animation and state will never change ever again after death
        if (HeroPtr->GetState() == HeroStateEnum::Die && HeroPtr->AnimationComp->AnimManagerDict[HeroStateEnum::Die].IsAnimationFinished())
        {
            HeroPtr->AnimationComp->AnimManagerDict[HeroStateEnum::Die].CurrentAnim->PlayOnlyLastFrame();
        }
    }
}

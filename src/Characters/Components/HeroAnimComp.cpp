#include "HeroAnimComp.h"
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
            Animation::CreateSpriteSheet("Player/Run/Back", "rogue_run_back_hands_001", "png", 0.15f),
            Animation::CreateSpriteSheet("Player/Run/BackWard", "rogue_run_backward_001", "png", 0.15f),
            Animation::CreateSpriteSheet("Player/Run/Forward", "rogue_run_forward_hands_001", "png", 0.15f),
            Animation::CreateSpriteSheet("Player/Run/Front", "rogue_run_front_hands_001", "png", 0.15f),
        };
        AddAnimationsForState<HeroRunEnum>(HeroStateEnum::Run,runAnims);

        //Idle
        const auto idleAnims = std::vector<Animation>{
            Animation::CreateSpriteSheet("Player/Idle/Back", "rogue_idle_back_hand_left_001", "png", 0.15f),
            Animation::CreateSpriteSheet("Player/Idle/BackWard", "rogue_idle_backwards_001", "png", 0.15f),
            Animation::CreateSpriteSheet("Player/Idle/Front", "rogue_idle_front_hand_left_001", "png", 0.15f),
            Animation::CreateSpriteSheet("Player/Idle/Right", "rogue_idle_hands_001", "png", 0.15f),
        };
        AddAnimationsForState<HeroIdleEnum>(HeroStateEnum::Idle,idleAnims);
        
        //Dash
        const auto dashAnims = std::vector<Animation>{
            Animation::CreateSpriteSheet("Player/Dash/Back", "rogue_dodge_back_001", "png", 0.15f),
            Animation::CreateSpriteSheet("Player/Dash/BackWard", "rogue_dodge_left_back_001", "png", 0.15f),
            Animation::CreateSpriteSheet("Player/Dash/Front", "rogue_dodge_front_001", "png", 0.15f),
            Animation::CreateSpriteSheet("Player/Dash/Left", "rogue_dodge_left_001", "png", 0.15f),
            Animation::CreateSpriteSheet("Player/Dash/Right", "rogue_dodge_left_001", "png", 0.15f),
        };
        AddAnimationsForState<HeroDashEnum>(HeroStateEnum::Dash, dashAnims);
    }

    void HeroAnimComp::Update()
    {
        AnimationKey newKey;

        if (HeroPtr->CurrentHeroState == HeroStateEnum::Idle)
            newKey = DirectionUtils::GetHeroIdleDirectionEnum(HeroPtr->CurrentDirection);

        else if (HeroPtr->CurrentHeroState == HeroStateEnum::Run)
            newKey = DirectionUtils::GetHeroRunEnum(HeroPtr->CurrentDirection);

        else if (HeroPtr->CurrentHeroState == HeroStateEnum::Dash)
            newKey = InputComponent::GetDashDirectionEnum();

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

        BaseAnimComp<HeroStateEnum>::Update(HeroPtr->CurrentHeroState, CurrentAnimStateKey);
    }
}

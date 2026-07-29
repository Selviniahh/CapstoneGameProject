#include "../../../../Engine/Managers/Time.h"
#include "HeroAnimComp.h"

#include "HeroMoveComp.h"
#include "InputComponent.h"
#include "../../../Guns/Base/GunBase.h"
#include "../Hero.h"
#include "../HeroDirections.h"

namespace ETG
{
    class RogueSpecial;
}

namespace ETG
{
    HeroAnimComp::HeroAnimComp()
    {
        HeroPtr = Hero::Get();
        IsGameObjectUISpecified = true;
        HeroAnimComp::SetAnimations();
        SetKeyResolvers();
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
        AddAnimationsForState<HeroRunEnum>(HeroStateEnum::Run, Playback::Loop, runAnims);

        //Idle
        const auto idleAnims = std::vector<Animation>{
            Animation::CreateSpriteSheet("Player/Idle/Back", "rogue_idle_back_hand_left_001", "png", IdleAnimFrameInterval),
            Animation::CreateSpriteSheet("Player/Idle/BackWard", "rogue_idle_backwards_001", "png", IdleAnimFrameInterval),
            Animation::CreateSpriteSheet("Player/Idle/Front", "rogue_idle_front_hand_left_001", "png", IdleAnimFrameInterval),
            Animation::CreateSpriteSheet("Player/Idle/Right", "rogue_idle_hands_001", "png", IdleAnimFrameInterval),
        };
        AddAnimationsForState<HeroIdleEnum>(HeroStateEnum::Idle, Playback::Loop, idleAnims);

        //Dash
        auto dashAnims = std::vector<Animation>{
            Animation::CreateSpriteSheet("Player/Dash/Back", "rogue_dodge_back_001", "png", DashAnimFrameInterval), //Dash_Back 0.62
            Animation::CreateSpriteSheet("Player/Dash/BackWard", "rogue_dodge_left_back_001", "png", DashAnimFrameInterval), //Dash_Backward 0.62
            Animation::CreateSpriteSheet("Player/Dash/Front", "rogue_dodge_front_001", "png", DashAnimFrameInterval), //Dash_Front will take 0.62
            Animation::CreateSpriteSheet("Player/Dash/Right", "rogue_dodge_left_001", "png", DashAnimFrameInterval), //Dash_Left  Will take: 0.466159 seconds
            Animation::CreateSpriteSheet("Player/Dash/Right", "rogue_dodge_left_001", "png", DashAnimFrameInterval), //Dash_Right Will take: 0.466159 seconds
        };
        AddAnimationsForState<HeroDashEnum>(HeroStateEnum::Dash, Playback::Once, dashAnims);

        //Hit (Because there's no hit animation, spin animation will be used)
        auto spinAnim = std::vector<Animation>{
            Animation::CreateSpriteSheet("Player/Spin", "rogue_spin_001", "png", 0.10),
        };
        AddAnimationsForState<HeroHit>(HeroStateEnum::Hit, Playback::Once, spinAnim);

        //Death
        auto DeathAnim = std::vector<Animation>{
            Animation::CreateSpriteSheet("Player/ShotDeath", "rogue_shot_death_001", "png", 0.10),
        };
        AddAnimationsForState<HeroDeath>(HeroStateEnum::Die, Playback::Once, DeathAnim);
    }

    //NOTE: This replaces the switch that used to open Update(). Adding a state is now one registration next to its
    //animations instead of another case label. The Direction -> sub-enum helpers are unchanged, just wired up here
    void HeroAnimComp::SetKeyResolvers()
    {
        SetKeyResolver(HeroStateEnum::Idle, [this] { return AnimationKey{HeroDirections::GetIdleEnum(HeroPtr->CurrentDir)}; });
        SetKeyResolver(HeroStateEnum::Run, [this] { return AnimationKey{HeroDirections::GetRunEnum(HeroPtr->CurrentDir)}; });
        SetKeyResolver(HeroStateEnum::Dash, [this] { return AnimationKey{HeroPtr->CurrentDashDirection}; });
        SetKeyResolver(HeroStateEnum::Hit, [] { return AnimationKey{HeroHit::JustHit}; });
        SetKeyResolver(HeroStateEnum::Die, [] { return AnimationKey{HeroDeath::Dead}; });
    }

    void HeroAnimComp::ApplyFrameInterval(const HeroStateEnum& state, const AnimationKey& key)
    {
        float interval;
        switch (state)
        {
        case HeroStateEnum::Idle: interval = IdleAnimFrameInterval;
            break;
        case HeroStateEnum::Run: interval = RunAnimFrameInterval;
            break;
        case HeroStateEnum::Dash: interval = DashAnimFrameInterval;
            break;
        default: return; //Hit and Die run at the interval baked into their animation
        }

        auto& animations = AnimManagerDict[state].AnimationDict;
        const auto it = animations.find(key);
        if (it != animations.end()) it->second.FrameInterval = interval;
    }

    void HeroAnimComp::Update()
    {
        const HeroStateEnum state = HeroPtr->GetState();
        const AnimationKey key = ResolveKey(state);

        ApplyFrameInterval(state, key);

        //NOTE: Nothing after this call. The dash-finished, hit-finished and stay-dead rules that used to live down
        //here are transitions and node ticks in HeroStateMachine now. This is also why nothing restarts animations
        //by hand any more: BaseAnimComp restarts on its own whenever the key changes
        BaseAnimComp<HeroStateEnum>::Update(state, key);
    }
}

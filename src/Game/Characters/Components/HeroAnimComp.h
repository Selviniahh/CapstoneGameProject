#pragma once
#include "../../../Engine/Platform/Platform.h"
#include "../../../Engine/Core/Components/BaseAnimComp.h"
#include "../../Characters/Hero.h"
#include "../../../Engine/Core/Events/EventDelegate.h"

namespace ETG
{
    //NOTE: This class used to own the dash: it held IsDashing and a dash timer, set the hero's state, ended the dash
    //and started the move component's cooldown. All of that belongs to the state machine now. What is left is what
    //an animation component should have been doing all along: pick the right animation for whatever state we are in
    class HeroAnimComp : public BaseAnimComp<HeroStateEnum>
    {
    public:
        HeroAnimComp();

        //Override
        void Update() override;
        void SetAnimations() override;

    public:
        //Animation interval time for next frame. Lower is faster. Higher is slower.
        float DashAnimFrameInterval = 0.075;
        float IdleAnimFrameInterval = 0.15;
        float RunAnimFrameInterval = 0.15;

    private:
        //Registers "which sub-animation plays for this state", replacing the switch that used to be in Update()
        void SetKeyResolvers();

        //Keeps the editor-tweakable intervals live
        void ApplyFrameInterval(const HeroStateEnum& state, const AnimationKey& key);

        Hero* HeroPtr = nullptr;

        BOOST_DESCRIBE_CLASS(HeroAnimComp, (BaseAnimComp), (HeroPtr, DashAnimFrameInterval, IdleAnimFrameInterval, RunAnimFrameInterval), (), ())
    };
}

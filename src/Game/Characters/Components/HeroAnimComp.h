#pragma once
#include "../../../Engine/Platform/Platform.h"
#include "../../../Engine/Core/Components/BaseAnimComp.h"
#include "../../Characters/Hero.h"
#include "../../../Engine/Core/Events/EventDelegate.h"

namespace ETG
{
    class HeroAnimComp : public BaseAnimComp<HeroStateEnum>
    {
    public:
        HeroAnimComp();

        //Override
        void Update() override;
        void SetAnimations() override;
        void StartDash(HeroDashEnum direction);
        void EndDash();
        bool IsDashAnimFinished() const;

    public:
        //Dash state
        bool IsDashing = false;

        //Animation interval time for next frame. Lower is faster. Higher is slower.  
        float DashAnimFrameInterval = 0.075;
        //NOTE: if any dash animation's frames are less (i.e Dash/Right), that dash will take less time to complete. Idk if this is a problem. For this reason we have MinDashDuration. But all dash frames are enough to make minimum half second so, for now MinDashDuration doesn't have any effect.
        float IdleAnimFrameInterval = 0.15;
        float RunAnimFrameInterval = 0.15;
        HeroDashEnum CurrentDashDirection = HeroDashEnum::Unknown;

        EventDelegate<HeroDashEnum> OnDashStart;
        EventDelegate<> OnDashEnd;

    private:
        Hero* HeroPtr = nullptr;

        //At least for 0.2 do not let dash to be stopped. Adding or removing these won't have any affect. 
        float DashTimer = 0.0f;
        float MinDashDuration = 0.2f;

        BOOST_DESCRIBE_CLASS(HeroAnimComp, (BaseAnimComp), (HeroPtr,DashAnimFrameInterval,IdleAnimFrameInterval, RunAnimFrameInterval, CurrentDashDirection), (), ())
    };
}

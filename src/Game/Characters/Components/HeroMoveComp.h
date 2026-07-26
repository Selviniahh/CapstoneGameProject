#pragma once
#include "../Hero.h"
#include "../../../Engine/Core/Components/BaseMoveComp.h"
#include "../../../Engine/Core/Events/EventDelegate.h"

namespace ETG
{
    class Hero; // Forward-declare the hero.
    enum class HeroDashEnum;

    //NOTE: This component no longer decides anything about state. It used to set Idle/Run/Dash itself and keep its
    //own dash timer alongside a second one in HeroAnimComp. Now it only does movement maths, when the machine asks
    class HeroMoveComp : public BaseMoveComp
    {
    public:
        HeroMoveComp();

        //Only resolves forces. Normal movement and dashing are driven by the hero's state machine nodes
        void Update() override;
        void Initialize() override;

        //Called from the Locomotion node's tick
        void UpdateMovement();

        //Called from the Dash node's enter / tick / exit
        void BeginDash();
        void MakeDashMovement(float elapsed);
        void StartDashCooldown();

        [[nodiscard]] bool IsDashAvailable() const;

        //How long the current dash direction's animation runs for
        [[nodiscard]] float GetDashDuration() const;

    public:

        Hero* HeroPtr = nullptr;

        //Dash. Both are Stats because items modify them ("dash goes further", "dash recharges faster"); MinDashDuration
        //stays a plain float because it is an animation-safety floor, not something an item has any business touching
        StatModifier DashAmount = 300.f;
        StatModifier DashCooldown = 0.5f; //After dash is over, the cooldown to be able to dash again

        //NOTE: if any dash animation has fewer frames (i.e Dash/Right), that dash would complete sooner. For this
        //reason the Dash state refuses to end before this much time has passed
        float MinDashDuration = 0.2f;

    private:
        float DashCooldownTimer = 0.f; //After each dash this will be assigned to `DashCooldown` and once it gets 0, dash will be available again
        ETG::Vector2f DashDirection; //This will set (-1, 1) based on DashDirectionEnum

        BOOST_DESCRIBE_CLASS(HeroMoveComp, (BaseMoveComp),
                             (HeroPtr, DashAmount, DashCooldown, MinDashDuration), (), ())
    };
}

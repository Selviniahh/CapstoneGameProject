#pragma once
#include "../Hero.h"
#include "../../../Engine/Core/Components/BaseMoveComp.h"
#include "../../../Engine/Core/Events/EventDelegate.h"

namespace ETG
{
    class Hero; // Forward-declare the hero.
    enum class HeroDashEnum;

    class HeroMoveComp : public BaseMoveComp
    {
    public:
        HeroMoveComp();

        void UpdateMovement();
        void Update() override;
        void Initialize() override;
        void MakeDashMovement();
        void ApplyDashImpulse();

        [[nodiscard]] bool IsDashAvailable() const;
        void StartDashCooldown();

    public:

        Hero* HeroPtr = nullptr;

        //Dash
        float DashAmount = 300;
        float DashCooldown = 0.5; //After dash is over, the cooldown to be able to dash again
        float DashDuration{}; //Dash will take this variable's value as total dash time. Calculated as current dash animation's TotalAnimationTime. 

    private:
        void SetupDashListeners();

        float DashCooldownTimer = 0.f; //After each dash this will be assigned to `DashCooldown` and once it gets 0, dash will be available again
        float DashTimer = 0.f; //Current Dash duration timer
        ETG::Vector2f DashDirection; //This will set (-1, 1) based on DashDirectionEnum 

        BOOST_DESCRIBE_CLASS(HeroMoveComp, (BaseMoveComp),
                             (HeroPtr, DashAmount, DashCooldown, DashDuration), (), ())
    };
}

#include "../../../Engine/Managers/Time.h"
#include "HeroMoveComp.h"
#include "HeroAnimComp.h"
#include "../../../Engine/Managers/InputManager.h"
#include "../../../Utils/Math.h"
#include "../Hero.h"  // For Hero
#include "../../../Utils/DirectionUtils.h"

namespace ETG
{
    HeroMoveComp::HeroMoveComp() : BaseMoveComp(200.f, 8000.f, 8000.f) // Adjust parameters as needed.
    {
    }

    void HeroMoveComp::Initialize()
    {
        BaseMoveComp::Initialize();
        if (!HeroPtr) HeroPtr = Hero::Get();
    }

    //NOTE: Forces (knockback from a hit, from death) have to keep resolving no matter which state the hero is in,
    //so they stay on the normal component update. Everything else moved into the state machine's nodes
    void HeroMoveComp::Update()
    {
        BaseMoveComp::Update();

        if (DashCooldownTimer > 0.f) DashCooldownTimer -= Time::FrameTick;
    }

    void HeroMoveComp::BeginDash()
    {
        DashDirection = DirectionUtils::GetDashDirectionVector();
    }

    float HeroMoveComp::GetDashDuration() const
    {
        //NOTE: Looked up by state and direction rather than by asking for "the current animation". The animation
        //component updates after the state machine, so on the very first dash tick "current" would still be the
        //idle animation and the dash would be paced with the wrong duration
        auto& dashAnims = HeroPtr->AnimationComp->AnimManagerDict[HeroStateEnum::Dash].AnimationDict;
        const auto it = dashAnims.find(HeroPtr->CurrentDashDirection);

        return it != dashAnims.end() ? it->second.GetTotalAnimationTime() : 0.f;
    }

    void HeroMoveComp::MakeDashMovement(const float elapsed)
    {
        const float dashDuration = GetDashDuration();
        if (dashDuration <= 0.f) return;

        //Calculate dash progress (0 to 1). For now this will only be 0.46 seconds for Right and Left dash. Rest will be 0.62 seconds
        const float dashProgress = elapsed / dashDuration;

        //Use bell curve to get velocity
        const ETG::Vector2f dashVelocity = Math::ApplyBellCurveForce(dashProgress, DashDirection, DashAmount, Time::FrameTick);

        HeroPtr->SetPosition(HeroPtr->GetPosition() + dashVelocity);

        //Override normal velocity during dash
        Velocity = {0, 0};
    }

    void HeroMoveComp::UpdateMovement()
    {
        // Determine input direction.
        const ETG::Vector2f inputDir = InputManager::IsMoving() ? InputManager::direction : ETG::Vector2f{0.f, 0.f};

        // Use the base helper to update velocity and position.
        BaseMoveComp::UpdateMovement(inputDir, const_cast<ETG::Vector2f&>(HeroPtr->GetPosition()));
    }

    //NOTE: The Die / Hit checks that used to live here are gone. Dash is a child of Alive, and its transition is
    //only ever evaluated while the hero is inside that subtree, so being dead already rules it out
    bool HeroMoveComp::IsDashAvailable() const
    {
        return DashCooldownTimer <= 0.f;
    }

    void HeroMoveComp::StartDashCooldown()
    {
        DashCooldownTimer = DashCooldown;
    }
}

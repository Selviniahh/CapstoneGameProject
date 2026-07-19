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

        SetupDashListeners();
    }

    //Instead of using lerp math class I decided to do this inline just like in here 

    void HeroMoveComp::Update()
    {
        // Call the base update (it doing nothing for now)
        BaseMoveComp::Update();

        //Update dash cooldown timer
        if (DashCooldown >= 0)
        {
            DashCooldownTimer -= Time::FrameTick;
        }

        //handle dash movement if dashing
        if (HeroPtr->AnimationComp->IsDashing)
        {
            DashTimer += Time::FrameTick;
            // DashDuration = HeroPtr->AnimationComp->AnimManagerDict[HeroStateEnum::Dash].AnimationDict[HeroPtr->AnimationComp->CurrentDashDirection].GetTotalAnimationTime();
            DashDuration = HeroPtr->AnimationComp->GetCurrentAnimation()->GetTotalAnimationTime();
            if (DashTimer <= DashDuration)
            {
                MakeDashMovement();
            }
            else
            {
                HeroPtr->AnimationComp->IsDashing = false;
            }
        }

        // Do normal movement only if not dashing 
        if (!HeroPtr->AnimationComp->IsDashing)
        {
            UpdateMovement();
        }
    }

    void HeroMoveComp::ApplyDashImpulse()
    {
        HeroPtr->AnimationComp->IsDashing = true;
        DashTimer = 0.f;
        DashDirection = DirectionUtils::GetDashDirectionVector();
    }

    void HeroMoveComp::SetupDashListeners()
    {
        HeroPtr->AnimationComp->OnDashStart.AddListener([this](const HeroDashEnum direction)
        {
            ApplyDashImpulse();
        });

        HeroPtr->AnimationComp->OnDashEnd.AddListener([this]()
        {
            HeroPtr->AnimationComp->IsDashing = false;
            DashTimer = 0.f;
        });
    }

    void HeroMoveComp::MakeDashMovement()
    {
        //Calculate dash progress (0 to DashDuration). For now DashDuration will only be 0.46 seconds for Right and Left dash. Rest will be 0.62 seconds
        const float dashProgress = DashTimer / DashDuration;

        //Use bell curve to get velocity
        const ETG::Vector2f dashVelocity = Math::ApplyBellCurveForce(dashProgress, DashDirection, DashAmount, Time::FrameTick);

        HeroPtr->SetPosition(HeroPtr->GetPosition() + dashVelocity);

        //Override normal velocity during dash
        Velocity = {0, 0};
    }

    void HeroMoveComp::UpdateMovement()
    {
        // Determine input direction.
        ETG::Vector2f inputDir(0.f, 0.f);
        if (InputManager::IsMoving() && !HeroPtr->AnimationComp->IsDashing && HeroPtr->GetState() != HeroStateEnum::Die && HeroPtr->GetState() != HeroStateEnum::Hit)
        {
            inputDir = InputManager::direction;
            HeroPtr->SetState(HeroStateEnum::Run);
        }
        else if (HeroPtr->MoveComp->HeroPtr->AnimationComp->IsDashing && HeroPtr->GetState() != HeroStateEnum::Die && HeroPtr->GetState() != HeroStateEnum::Hit)
        {
            HeroPtr->SetState(HeroStateEnum::Dash);
        }

        //If not dead or hit, set to idle as last resort
        else if (HeroPtr->GetState() != HeroStateEnum::Die && HeroPtr->GetState() != HeroStateEnum::Hit)
        {
            HeroPtr->SetState(HeroStateEnum::Idle);
        }

        // Use the base helper to update velocity and position.
        BaseMoveComp::UpdateMovement(inputDir, const_cast<ETG::Vector2f&>(HeroPtr->GetPosition()));
    }

    bool HeroMoveComp::IsDashAvailable() const
    {
        return DashCooldownTimer <= 0.f && HeroPtr->GetState() != HeroStateEnum::Die && HeroPtr->GetState() != HeroStateEnum::Hit;
    }

    void HeroMoveComp::StartDashCooldown()
    {
        DashCooldownTimer = DashCooldown;
    }
}

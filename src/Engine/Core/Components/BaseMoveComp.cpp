#include "BaseMoveComp.h"
#include "../../Managers/Globals.h"
#include "../../../Utils/Math.h"

namespace ETG
{
    BaseMoveComp::BaseMoveComp(const float maxSpeed, const float acceleration, const float deceleration)
        : MaxSpeed(maxSpeed), Acceleration(acceleration), Deceleration(deceleration), Velocity(0.f, 0.f)
    {
    }

    void BaseMoveComp::Update()
    {
        // Update force effect in the base Update
        UpdateForce();
    }

    void BaseMoveComp::UpdateMovement(const ETG::Vector2f& inputDir, ETG::Vector2f& position)
    {
        // Don't process normal movement when being forced
        if (IsBeingForced) return;

        const float deltaTime = Globals::FrameTick; // delta time from your globals

        //There's a movement input
        if (inputDir != ETG::Vector2f(0.f, 0.f))
        {
            // Accelerate: add (normalized input * acceleration * dt)
            const ETG::Vector2f normDir = Math::Normalize(inputDir);
            Velocity += normDir * Acceleration * deltaTime;

            // Clamp speed to MaxSpeed.
            const float currentSpeed = Math::VectorLength(Velocity);
            if (currentSpeed > MaxSpeed)
            {
                //Firstly normalize so that we will just have pure direction and then multiply by max speed. 
                Velocity = Math::Normalize(Velocity) * MaxSpeed;
            }
        }
        else
        {
            // No input so decelerate.
            const float currentSpeed = Math::VectorLength(Velocity);
            const float decAmount = Deceleration * deltaTime;
            if (decAmount > currentSpeed)
            {
                Velocity = ETG::Vector2f(0.f, 0.f);
            }
            else
            {
                Velocity -= Math::Normalize(Velocity) * decAmount;
            }
        }

        // Update the position by velocity.
        position += Velocity * deltaTime;
    }

    void BaseMoveComp::ApplyForce(const ETG::Vector2f& forceDirection, const float magnitude, const float forceDuration)
    {
        // Set force parameters
        ForceDirection = forceDirection;
        ForceMagnitude = magnitude;
        ForceMaxDuration = forceDuration;
        ForceTimer = 0.0f;
        IsBeingForced = true;

        // Broadcast force start event
        OnForceStart.Broadcast();
    }

    void BaseMoveComp::UpdateForce()
    {
        if (!IsBeingForced) return;
        if (!Owner) return;

        ForceTimer += Globals::FrameTick;

        if (ForceTimer < ForceMaxDuration)
        {
            // Calculate current force magnitude using lerp (starts strong, gradually weakens)
            //So for instance we will lerp from 200 to 0 over half second and then apply the value to position
            const float currentForce = Math::IntervalLerp(ForceMagnitude * ForceSpeed, 0.0f, ForceMaxDuration, ForceTimer);

            // Apply force to position
            Owner->SetPosition(Owner->GetPosition() + ForceDirection * currentForce * Globals::FrameTick);
        }
        else
        {
            // Force effect is done
            IsBeingForced = false;
            OnForceEnd.Broadcast();
            Velocity = ETG::Vector2f(0.f, 0.f); // Reset velocity after force ends so based on acceleration it will slowly build up velocity in every force
            
        }
    }
}

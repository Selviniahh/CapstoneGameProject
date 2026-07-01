#pragma once
#include "Platform/Platform.h"
#include "../ComponentBase.h"
#include "../Events/EventDelegate.h"

namespace ETG
{
    class BaseMoveComp : public ComponentBase
    {
    protected:
        // Protected constructor so only derived classes can create one.
        BaseMoveComp(float maxSpeed, float acceleration, float deceleration = 8000.f);

    public:
        // Parameters:
        float MaxSpeed; // Maximum speed (magnitude)
        float Acceleration; // Acceleration rate when input is present
        float Deceleration; // Deceleration rate when no input

        // Current velocity
        ETG::Vector2f Velocity;

        void Update() override;

        // Movement function
        void UpdateMovement(const ETG::Vector2f& inputDir, ETG::Vector2f& position);

        // Force handling
        void ApplyForce(const ETG::Vector2f& forceDirection, float magnitude, float forceDuration);
        void UpdateForce();

        

        // Force parameters
        float ForceSpeed = 1;
        float ForceMagnitude = 0.0f;
        float ForceTimer = 0.0f;
        float ForceMaxDuration = 0.0f; //will be set by the ApplyForce function
        ETG::Vector2f ForceDirection = {0.0f, 0.0f};
        bool IsBeingForced = false;

        // Events for force application
        EventDelegate<> OnForceStart;
        EventDelegate<> OnForceEnd;

        BOOST_DESCRIBE_CLASS(BaseMoveComp, (ComponentBase),
                             (MaxSpeed, Acceleration, Deceleration, Velocity,
                                 ForceSpeed, ForceMaxDuration, IsBeingForced), (), ())
    };
}

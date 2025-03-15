#include "BaseMoveComp.h"
#include "../../Managers/Globals.h"
#include "../../Utils/Math.h"

BaseMoveComp::BaseMoveComp(float maxSpeed, float acceleration, float deceleration)
    : ComponentBase(), MaxSpeed(maxSpeed),
      Acceleration(acceleration),
      Deceleration(deceleration),
      Velocity(0.f, 0.f)
{
}

void BaseMoveComp::Update()
{
    ComponentBase::Update();
    IsMoving = Velocity != sf::Vector2f{0,0};
}

void BaseMoveComp::UpdateMovement(const sf::Vector2f& inputDir, sf::Vector2f& position)
{
    const float deltaTime = ETG::Globals::FrameTick; // delta time from your globals

    //There's a movement input
    if (inputDir != sf::Vector2f(0.f, 0.f))
    {
        // Accelerate: add (normalized input * acceleration * dt)
        const sf::Vector2f normDir = Math::Normalize(inputDir);
        Velocity += normDir * Acceleration * deltaTime;
        
        // Clamp speed to MaxSpeed.
        float currentSpeed = Math::Length(Velocity);
        if (currentSpeed > MaxSpeed)
        {
            Velocity = Math::Normalize(Velocity) * MaxSpeed;
        }
    }
    else
    {
        // No input: decelerate.
        float currentSpeed = Math::Length(Velocity);
        float decAmount = Deceleration * deltaTime;
        if (decAmount > currentSpeed)
        {
            Velocity = sf::Vector2f(0.f, 0.f);
        }
        else
        {
            Velocity -= Math::Normalize(Velocity) * decAmount;
        }
    }

    // Update the position by velocity.
    position += Velocity * deltaTime;
}

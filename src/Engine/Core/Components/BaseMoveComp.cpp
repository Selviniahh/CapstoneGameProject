#include "../../Managers/Time.h"
#include "BaseMoveComp.h"
#include "../../Managers/RenderContext.h"
#include "../../../Utils/Math.h"
#include "../Systems/CollisionSystem.h"

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

        const float deltaTime = Time::FrameTick; // delta time from your globals

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

        // Pozisyonu velocity ile guncelle - duvarin gecirmedigi kisim dusulerek
        MoveAndSlide(position, Velocity * deltaTime);
    }

    //NOTE: `position` ile Velocity'yi bilerek BIRLIKTE yaziyor. Ikisi tek cevabin iki yarisi: pozisyon duvarin
    //yuzunde duruyor, velocity de o yone itmeye devam edecegi bileseni kaybediyor. Sadece pozisyonu duzeltmek,
    //bir frame dogru gorunup sonrasinda yanlis hissettiren versiyondur - mover duvara dayanmis dururken hala
    //dosdogru duvara nisan almis bir velocity tasir, ve baska yone dondugu anda o birikmis itis yanlamasina disari cikar
    bool BaseMoveComp::MoveAndSlide(ETG::Vector2f& position, const ETG::Vector2f& delta)
    {
        //Kati olarak hicbir sey isimlendirilmemis, dolayisiyla karsi test edilecek bir sey yok ve hareket hic
        //dokunulmadan gecer. Bu, henuz dahil olmamis her mover'in her frame izledigi yol; yani tek bir if, o kadar
        if (!BodyCollider || BodyCollider->BlockingMask == CollisionLayer::None || !BodyCollider->IsCollisionEnabled())
        {
            position += delta;
            return false;
        }

        const CollisionSystem::SlideResult result = CollisionSystem::MoveAndSlide(BodyCollider, delta, Velocity);

        position += result.Delta;
        Velocity = result.Velocity;
        return result.Blocked;
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

        ForceTimer += Time::FrameTick;

        if (ForceTimer < ForceMaxDuration)
        {
            // Calculate current force magnitude using lerp (starts strong, gradually weakens) So for instance we will lerp from 200 to 0 over half second and then apply the value to position
            //Suan uygulanmasi gereken Force
            const float currentForce = Math::IntervalLerp(ForceMagnitude * ForceMultiplier, 0.0f, ForceMaxDuration, ForceTimer);

            //Force'u pozisyona uygula - diger her hareket turu gibi duvarlardan gecerek. Duvara giren bir
            //knockback duvarda durur; aciyla geldiyse duvar boyunca kaymaya devam eder: serbest olan eksen hala
            //serbesttir ve bu, burasi hicbir sey bilmek zorunda kalmadan cozumun kendisinden dogar
            ETG::Vector2f position = Owner->GetPosition();
            MoveAndSlide(position, ForceDirection * currentForce * Time::FrameTick);
            Owner->SetPosition(position);
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

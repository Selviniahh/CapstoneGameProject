#include "MagazineDrop.h"
#include <algorithm>
#include <cstdint>
#include "../../../Engine/Managers/AssetManager.h"
#include "../../../Engine/Managers/SpriteBatch.h"
#include "../../../Engine/Managers/Time.h"

namespace ETG
{
    MagazineDrop::MagazineDrop()
    {
        MagazineDrop::Initialize();
    }

    void MagazineDrop::SetSprite(const std::string& relativePath)
    {
        Texture = AssetManager::LoadTexture(relativePath);

        //Centred, so SpinSpeed turns the magazine about itself instead of swinging it around a corner
        Origin = {
            static_cast<float>(Texture->getSize().x) / 2.f,
            static_cast<float>(Texture->getSize().y) / 2.f
        };

        //DrawProps caches a RAW pointer to the texture, and the copy taken in the constructor was taken while
        //there was no texture at all. Anything that changes what this object looks like has to republish, or
        //the next Draw submits the stale copy
        ComputeDrawProperties();
    }

    void MagazineDrop::Drop(const ETG::Vector2f& worldPos, const float rotation, const ETG::Vector2f& velocity,
                            const float depth)
    {
        //A gun that never called SetSprite has nothing to throw. Silently doing nothing is right here: the
        //gun's reload still runs, it just does not litter
        if (!HasSprite()) return;

        Position = worldPos;
        Rotation = rotation;
        Velocity = velocity;
        Depth = depth;
        TimeLeft = LifeTime;
        Color = ETG::Color::White; //undo the fade the previous drop ended on

        //Published here and not left to the next Update, because there may not be one before the next Draw:
        //the drop is triggered from the gun's Update, which for AK47 runs AFTER GunBase::Update has already
        //ticked this object for the frame. Without this the magazine's first drawn frame uses the properties
        //from before it was thrown - including a null texture pointer on the very first drop, which is a
        //dereference inside SpriteBatch rather than a wrong-looking sprite
        ComputeDrawProperties();
    }

    void MagazineDrop::Update()
    {
        if (!IsFalling()) return;

        const float deltaTime = Time::FrameTick;

        //Plain Euler integration. A magazine is on screen for under a second and nothing depends on where it
        //lands, so there is nothing here worth a better integrator
        Velocity.y += Gravity * deltaTime;
        Position += Velocity * deltaTime;
        Rotation += SpinSpeed * deltaTime;

        TimeLeft -= deltaTime;

        //Fades over the last FadeTime seconds of the life, which is what stands in for it landing. Alpha is
        //written into Color because ComputeDrawProperties copies it straight into the draw call
        const float fade = FadeTime > 0.f ? std::min(TimeLeft / FadeTime, 1.f) : 1.f;
        Color.a = static_cast<std::uint8_t>(std::clamp(fade, 0.f, 1.f) * 255.f);

        GameObjectBase::Update();
    }

    void MagazineDrop::Draw()
    {
        if (!IsFalling() || !HasSprite()) return;

        SpriteBatch::Draw(GetDrawProperties());
    }
}

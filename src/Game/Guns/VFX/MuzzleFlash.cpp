#include "MuzzleFlash.h"
#include <complex>
#include <filesystem>
#include <numbers>
#include "../../../Engine/Managers/RenderContext.h"
#include "../../../Engine/Managers/SpriteBatch.h"

namespace ETG
{
    MuzzleFlash::MuzzleFlash()
    {
        isActive = false; //Make sure animation not playing at the start
        MuzzleFlash::Initialize();
    }

    void MuzzleFlash::SetAnimation(const std::string& relativePath, const std::string& fileName, const std::string& extension, const float frameSpeed)
    {
        this->frameSpeed = frameSpeed;
        Animation = Animation::CreateSpriteSheet(relativePath, fileName, extension, frameSpeed);
        isActive = false;

        if (Animation.Texture)
        {
            Origin = {
                static_cast<float>(Animation.Texture->getSize().x / 2),
                static_cast<float>(Animation.Texture->getSize().y / 2)
            };
        }
    }

    void MuzzleFlash::Initialize()
    {
        GameObjectBase::Initialize();
    }

    void MuzzleFlash::Update()
    {
        GameObjectBase::Update();

        // Update animation if active
        if (isActive)
        {
            Animation.Update();

            // If animation finished, deactivate
            if (Animation.IsAnimationFinished())
            {
                Deactivate();
            }
        }

        UpdatePosition();
    }

    void MuzzleFlash::Draw()
    {
        if (!isActive || !Animation.Texture) return;

        // Draw the muzzle flash animation
        Animation.Draw(Animation.Texture, Position, ETG::Color::White, Rotation, Origin, Scale, Depth);
    }

    // Update position based on parent if available
    void MuzzleFlash::UpdatePosition()
    {
        if (parentObject)
        {
            // Get parent's properties
            const auto& parentProps = parentObject->GetDrawProperties();
            const float angle = parentProps.Rotation * (std::numbers::pi / 180.0f);

            // Create a copy of the attachment offset
            ETG::Vector2f offsetToUse = AttachmentOffset;

            // If the parent is flipped vertically, flip the Y component of the offset
            if (parentProps.Scale.y < 0) offsetToUse.y = -offsetToUse.y;

            // Calculate rotated offset with the potentially flipped Y value
            const ETG::Vector2f rotatedOffset = {
                offsetToUse.x * std::cos(angle) - offsetToUse.y * std::sin(angle),
                offsetToUse.x * std::sin(angle) + offsetToUse.y * std::cos(angle)
            };

            // Set position relative to parent
            Position = parentProps.Position + rotatedOffset;

            if (InheritParentRotation)
            {
                Rotation = parentProps.Rotation;
            }
            else
            {
                //Standing upright means the gun's angle can no longer tell this effect which
                //way it is pointing, so take the facing from the parent's flip instead. Without
                //it a plume drifts forwards when aiming right and backwards when aiming left.
                Rotation = 0.f;
                Scale.x = parentProps.Scale.y < 0 ? -std::abs(Scale.x) : std::abs(Scale.x);
            }
        }
    }

    void MuzzleFlash::Activate()
    {
        //A gun that never calls SetAnimation (the AK47, Magnum and SawedOff all hide their
        //flash) has no frames, and Animation::Update would index an empty FrameRects.
        if (!HasAnimation()) return;

        isActive = true;
        Animation.Active = true;
    }

    void MuzzleFlash::Deactivate()
    {
        isActive = false;
        Animation.Active = false;
    }

    void MuzzleFlash::Restart()
    {
        Animation.Restart();
        Activate();
    }
}

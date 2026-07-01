#include "MuzzleFlash.h"
#include <complex>
#include <filesystem>
#include <numbers>
#include "../../Managers/Globals.h"
#include "../../Managers/SpriteBatch.h"

namespace ETG
{
    MuzzleFlash::MuzzleFlash(const std::string& relativePath, const std::string& fileName, const std::string& extension, const float frameSpeed) : frameSpeed(frameSpeed)
    {
        // Load the animation from the sprite sheet
        Animation = Animation::CreateSpriteSheet(relativePath, fileName, extension, frameSpeed);
        isActive = false; //Make sure animation not playing at the start
        
        if (Animation.Texture)
        {
            Origin = {
                static_cast<float>(Animation.Texture->getSize().x / 2),
                static_cast<float>(Animation.Texture->getSize().y / 2)
            };
        }

        MuzzleFlash::Initialize();
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
            ETG::Vector2f offsetToUse = attachmentOffset;

            // If the parent is flipped vertically, flip the Y component of the offset
            if (parentProps.Scale.y < 0) offsetToUse.y = -offsetToUse.y;

            // Calculate rotated offset with the potentially flipped Y value
            const ETG::Vector2f rotatedOffset = {
                offsetToUse.x * std::cos(angle) - offsetToUse.y * std::sin(angle),
                offsetToUse.x * std::sin(angle) + offsetToUse.y * std::cos(angle)
            };

            // Set position relative to parent
            Position = parentProps.Position + rotatedOffset;
            Rotation = parentProps.Rotation;
        }
    }

    void MuzzleFlash::Activate()
    {
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

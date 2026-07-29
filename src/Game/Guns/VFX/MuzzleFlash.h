#pragma once
#include "../../../Engine/Platform/Platform.h"
#include "../../../Engine/Core/GameObjectBase.h"
#include "../../../Engine/Animation/Animation.h"

namespace ETG
{
    class MuzzleFlash : public GameObjectBase
    {
    public:
        //Starts out empty: a flash belongs to a specific gun, so the owner hands it its
        //sheet via SetAnimation instead of GunBase guessing one for everybody.
        MuzzleFlash();
        ~MuzzleFlash() override = default;

        //Loads the sheet and centres the origin on it. Callers that need the origin
        //somewhere else (a multi-frame strip, where the sheet centre is meaningless)
        //override it with SetOrigin afterwards.
        void SetAnimation(const std::string& relativePath, const std::string& fileName, const std::string& extension, float frameSpeed = 0.10f);
        [[nodiscard]] bool HasAnimation() const { return Animation.Texture != nullptr; }

        void Initialize() override;
        void UpdatePosition();
        void Update() override;
        void Draw() override;
        
        // Control methods
        void Activate();
        void Deactivate();
        void Restart();
        bool IsActive() const { return isActive; }
        bool IsFinished() const { return Animation.IsAnimationFinished(); }
        
        // Set attachment offset (relative to parent position)
        void SetAttachmentOffset(const ETG::Vector2f& offset) { AttachmentOffset = offset; }
        ETG::Vector2f GetAttachedOffset() const { return AttachmentOffset; }
        
        // Set parent object to follow
        void SetParent(GameObjectBase* parent) { parentObject = parent; }

        //A flash points down the barrel, so it turns with the gun. Smoke rises no matter
        //which way the gun is aimed - and aiming left puts the gun at ~180 degrees, which
        //would otherwise stand the smoke on its head and make it pour downwards.
        //The attachment offset still rotates either way, so the effect stays put on the gun.
        void SetInheritParentRotation(const bool inherit) { InheritParentRotation = inherit; }
        
        Animation Animation;

    private:
        bool isActive = false;
        bool InheritParentRotation = true;
        GameObjectBase* parentObject = nullptr;
        
        // Frame speed for animation
        float frameSpeed = 0.10f;
        ETG::Vector2f AttachmentOffset = {0.0f, 0.0f};
        
        BOOST_DESCRIBE_CLASS(MuzzleFlash, (GameObjectBase),
            (Texture, AttachmentOffset, isActive, frameSpeed),
            (Animation),
            ())
    };
}
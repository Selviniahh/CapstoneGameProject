#pragma once
#include "Platform/Platform.h"
#include "../../Core/GameObjectBase.h"
#include "../../Animation/Animation.h"

namespace ETG
{
    class MuzzleFlash : public GameObjectBase
    {
    public:
        // Constructor with path to animation sprite sheet
        ~MuzzleFlash() override = default;
        MuzzleFlash(const std::string& relativePath, const std::string& fileName, const std::string& extension, float frameSpeed = 0.10f);
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
        void SetOffset(const ETG::Vector2f& offset) { attachmentOffset = offset; }
        ETG::Vector2f GetOffset() const { return attachmentOffset; }
        
        // Set parent object to follow
        void SetParent(GameObjectBase* parent) { parentObject = parent; }
        
        Animation Animation;

    private:
        bool isActive = false;
        ETG::Vector2f attachmentOffset = {0.0f, 0.0f};
        GameObjectBase* parentObject = nullptr;
        
        // Frame speed for animation
        float frameSpeed = 0.10f;
        
        BOOST_DESCRIBE_CLASS(MuzzleFlash, (GameObjectBase),
            (isActive, frameSpeed),
            (Animation, attachmentOffset),
            ())
    };
}
#pragma once
#include "../../../Engine/Platform/Platform.h"
#include "../../../Engine/Core/GameObjectBase.h"
#include "../../../Engine/Animation/Animation.h"

namespace ETG
{
    class MuzzleFlash : public GameObjectBase
    {
    public:
        // Başlangıçta boştur: flash belirli bir silaha aittir. Bu nedenle GunBase herkes için tahmin
        // yürütmek yerine owner, sprite sheet'i SetAnimation aracılığıyla verir.
        MuzzleFlash();
        ~MuzzleFlash() override = default;

        // Sprite sheet'i yükler ve origin'i merkezine yerleştirir. Origin'e başka bir yerde ihtiyaç duyan
        // caller'lar (sprite sheet merkezinin anlamsız olduğu multi-frame strip gibi) daha sonra
        // SetOrigin ile override eder.
        void SetAnimation(const std::string& relativePath, const std::string& fileName, const std::string& extension, float frameSpeed = 0.10f);
        [[nodiscard]] bool HasAnimation() const { return Animation.Texture != nullptr; }

        void Initialize() override;
        void UpdatePosition();
        void Update() override;
        void Draw() override;
        
        // Control method'ları
        void Activate();
        void Deactivate();
        void Restart();
        bool IsActive() const { return isActive; }
        bool IsFinished() const { return Animation.IsFinished(); }
        
        // Attachment offset'i ayarla (parent position'a göre)
        void SetAttachmentOffset(const ETG::Vector2f& offset) { AttachmentOffset = offset; }
        ETG::Vector2f GetAttachedOffset() const { return AttachmentOffset; }
        
        // Takip edilecek parent object'i ayarla
        void SetParent(GameObjectBase* parent) { parentObject = parent; }

        // Flash barrel yönüne baktığından silahla birlikte döner. Smoke, silahın nişan aldığı yönden bağımsız
        // olarak yükselir. Sola nişan almak silahı yaklaşık 180 dereceye getirir; aksi durumda bu,
        // smoke'u ters çevirip aşağı akmasına neden olur. Attachment offset her iki durumda da dönmeye
        // devam ettiği için effect silah üzerindeki konumunu korur.
        void SetInheritParentRotation(const bool inherit) { InheritParentRotation = inherit; }
        
        Animation Animation;

    private:
        bool isActive = false;
        bool InheritParentRotation = true;
        GameObjectBase* parentObject = nullptr;
        
        // Animation için frame hızı
        float frameSpeed = 0.10f;
        ETG::Vector2f AttachmentOffset = {0.0f, 0.0f};
        
        BOOST_DESCRIBE_CLASS(MuzzleFlash, (GameObjectBase),
            (Texture, AttachmentOffset, isActive, frameSpeed),
            (Animation),
            ())
    };
}

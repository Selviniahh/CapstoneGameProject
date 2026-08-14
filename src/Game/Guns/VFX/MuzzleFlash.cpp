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
        isActive = false; // Animation'ın başlangıçta oynamadığından emin ol
        MuzzleFlash::Initialize();
    }

    void MuzzleFlash::SetAnimation(const std::string& relativePath, const std::string& fileName, const std::string& extension, const float frameSpeed)
    {
        this->frameSpeed = frameSpeed;
        Animation = Animation::CreateSpriteSheet(relativePath, fileName, extension, frameSpeed);

        // Flash ve smoke puff bir kez oynatılır ve biter. Aşağıdaki Update(), animation'ın bittiğini
        // bildirdiği anda tüm object'i deactivate eder; bu yüzden loop etmemelidir.
        Animation.Loops = false;
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

        // Active ise animation'ı update et
        if (isActive)
        {
            Animation.Update();

            // Animation bittiyse deactivate et
            if (Animation.IsFinished())
            {
                Deactivate();
            }
        }

        UpdatePosition();
    }

    void MuzzleFlash::Draw()
    {
        if (!isActive || !Animation.Texture) return;

        // Muzzle flash animation'ını çiz
        Animation.Draw(Position, ETG::Color::White, Rotation, Origin, Scale, Depth);
    }

    // Parent varsa position'ı ona göre update et
    void MuzzleFlash::UpdatePosition()
    {
        if (parentObject)
        {
            // Parent property'lerini al
            const auto& parentProps = parentObject->GetDrawProperties();
            const float angle = parentProps.Rotation * (std::numbers::pi / 180.0f);

            // Attachment offset'in bir copy'sini oluştur
            ETG::Vector2f offsetToUse = AttachmentOffset;

            // Parent dikey olarak flip edilmişse offset'in Y component'ini flip et
            if (parentProps.Scale.y < 0) offsetToUse.y = -offsetToUse.y;

            // Gerekirse flip edilmiş Y değeriyle rotated offset'i hesapla
            const ETG::Vector2f rotatedOffset = {
                offsetToUse.x * std::cos(angle) - offsetToUse.y * std::sin(angle),
                offsetToUse.x * std::sin(angle) + offsetToUse.y * std::cos(angle)
            };

            // Position'ı parent'a göre ayarla
            Position = parentProps.Position + rotatedOffset;

            if (InheritParentRotation)
            {
                Rotation = parentProps.Rotation;
            }
            else
            {
                // Dik durduğunda silahın angle'ı effect'in hangi yöne baktığını artık belirtemez;
                // bu nedenle facing bilgisini parent'ın flip durumundan al. Bu olmazsa smoke plume,
                // sağa nişan alırken ileriye, sola nişan alırken geriye doğru hareket eder.
                Rotation = 0.f;
                Scale.x = parentProps.Scale.y < 0 ? -std::abs(Scale.x) : std::abs(Scale.x);
            }
        }
    }

    void MuzzleFlash::Activate()
    {
        // SetAnimation çağırmayan bir silahın (AK47, Magnum ve SawedOff flash'ı gizler) frame'i yoktur;
        // bu durumda Animation::Update boş bir FrameRects'i index etmeye çalışır.
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

#include "ShellEjector.h"
#include <cmath>
#include <random>
#include "../Base/GunBase.h"
#include "../../../Engine/Managers/AssetManager.h"
#include "../../../Engine/Managers/SpriteBatch.h"
#include "../../../Engine/Managers/Time.h"
#include "../../../Utils/TextureUtils.h"

namespace ETG
{
    ShellEjector::ShellEjector()
    {
        // Component'in kendi görseli yoktur; çizdiği şey sahip olduğu kovanlardır
        IsVisible = false;
    }

    void ShellEjector::SetSprite(const std::string& relativePath)
    {
        Texture = AssetManager::LoadTexture(relativePath);

        // Merkezlenmiştir: kovan bir köşesi etrafında savrulmak yerine kendi ortası etrafında döner. Draw
        // her kovan için bu Origin'i kullanır, dolayısıyla burada bir kez hesaplanması yeterlidir.
        Origin = {
            static_cast<float>(Texture->getSize().x) / 2.f,
            static_cast<float>(Texture->getSize().y) / 2.f
        };
    }

    float ShellEjector::Jitter(const float amount)
    {
        if (amount <= 0.f) return 0.f;

        // Tek bir shared engine: her kovan için yeni bir tane seed'lemek hem pahalı olur hem de aynı frame'de
        // çıkan kovanlara aynı sapmayı verirdi
        static std::mt19937 engine{std::random_device{}()};

        return std::uniform_real_distribution{-amount, amount}(engine);
    }

    void ShellEjector::Eject(const GunBase& gun)
    {
        if (!Enabled) return;

        ShellCasing casing{};

        // Ejection port silahın bir pixel'idir: rotate, mirror ve frame Origin kayması WorldPointOnGun içinde
        // hallolur, dolayısıyla kovan silah nereye nişan alırsa alsın doğru delikten çıkar
        casing.Position = gun.WorldPointOnGun(EjectPoint);

        casing.Velocity = {
            EjectVelocity.x + Jitter(VelocityJitter),
            EjectVelocity.y + Jitter(VelocityJitter)
        };

        // Silahla birlikte mirror edilir; böylece sola nişan alırken kovan hero'nun içine değil, yine silahın
        // arkasına doğru fırlar
        if (!gun.IsHeldOnRightHand) casing.Velocity.x = -casing.Velocity.x;

        casing.Rotation = Jitter(180.f);
        casing.SpinSpeed = SpinSpeed + Jitter(SpinJitter);

        // Zemin, sabit bir ekran çizgisi değil, ejection anındaki yüksekliğe göredir. Hero yukarı doğru
        // yürürken atılan kovan da bu sayede onunla birlikte yukarıda kalır.
        casing.GroundY = casing.Position.y + GroundDrop;

        Casings.push_back(casing);

        // MaxCasings 0 ise sınır yoktur ve kovanlar sonsuza kadar birikir; bu, istenen davranıştır. Sınır
        // konduğunda en eski kovan düşer, çünkü yeni atılan her zaman hero'nun bulunduğu yerdedir ve ilgi çeken odur.
        if (MaxCasings > 0)
            while (static_cast<int>(Casings.size()) > MaxCasings)
                Casings.erase(Casings.begin());
    }

    void ShellEjector::Update()
    {
        const float deltaTime = Time::FrameTick;

        for (ShellCasing& casing : Casings)
        {
            // Yere inmiş kovan bir daha hiç simüle edilmez. Loop yine hepsinin üzerinden geçer ama gövde tek bir
            // bool testine iner; bu yüzden binlerce kovan birikmesi Update tarafında bir şey maliyet etmez.
            if (casing.Landed) continue;

            casing.Velocity.y += Gravity * deltaTime;
            casing.Position += casing.Velocity * deltaTime;
            casing.Rotation += casing.SpinSpeed * deltaTime;

            if (casing.Position.y < casing.GroundY) continue;

            // Zemine değdi. simdi amaç yere çarpınca kovanı yukarı sektirmek Bounce hızın ne kadarını geri veriyorsa o kadar seker; sekme her temasta küçülür ve
            // yeterince yavaşladığında kovan olduğu yerde durur.
            casing.Position.y = casing.GroundY;

            //Yere çarptı hafif Yukarı sekmesi gerekiyor 
            // casing.Velocity.y = 10.0f;
            // Bounce = 0.5f;
            // upwardSpeed = -10.0f * 0.5f; // -5
            const float upwardSpeed = -casing.Velocity.y * Bounce;
            if (upwardSpeed > 12.f)
            {
                casing.Velocity.y = -upwardSpeed;
                casing.Velocity.x *= Bounce;
                casing.SpinSpeed *= Bounce;
                continue;
            }

            // Artık zıplayacak kadar hızlı değil: burada kalır. Spin de durur, aksi hâlde kovan yerde sonsuza
            // kadar dönerdi.
            casing.Velocity = {};
            casing.SpinSpeed = 0.f;
            casing.Landed = true;
        }
    }

    void ShellEjector::Draw()
    {
        if (Casings.empty()) return;

        GameObjectBase::DrawProperties props{};
        props.Depth = Depth;

        if (HasSprite())
        {
            // Artwork olduğu gibi çizilir; yedek yolun CasingSize/CasingColor değerleri karışmaz. Component'in
            // kendi Scale'i yine de geçer: sprite'ı yeniden çizmeden küçültüp büyütmenin yolu SetScale'dir.
            props.Texture = Texture.get();
            props.Origin = Origin;
            props.Scale = Scale;
        }
        else
        {
            // Silah kovan sprite'ı vermemiş: 1x1 beyaz pixel CasingSize'a ölçeklenip CasingColor ile tint'lenir.
            // Böylece kovanlar artwork üretilene kadar da görünür ve boyutları ImGui'dan ayarlanır.
            static std::shared_ptr<ETG::Texture> pixelTex = GetPixelTexture();
            props.Texture = pixelTex.get();
            props.Origin = {0.5f, 0.5f}; // 1x1 texture'ın merkezi: kovan kendi ortası etrafında döner
            props.Scale = CasingSize;
            props.Color = CasingColor;
        }

        for (const ShellCasing& casing : Casings)
        {
            props.Position = casing.Position;
            props.Rotation = casing.Rotation;
            SpriteBatch::Draw(props);
        }
    }
}

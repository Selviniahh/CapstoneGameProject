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

        // Merkezlenmiştir; böylece SpinSpeed magazine'i bir köşe etrafında savurmak yerine kendi etrafında döndürür
        Origin = {
            static_cast<float>(Texture->getSize().x) / 2.f,
            static_cast<float>(Texture->getSize().y) / 2.f
        };

        // DrawProps, texture için RAW pointer'ı cache'ler ve constructor'da alınan copy henüz texture yokken
        // oluşturulmuştur. Bu object'in görünümünü değiştiren her işlem yeniden publish edilmelidir; aksi hâlde
        // sonraki Draw eski copy'yi gönderir.
        ComputeDrawProperties();
    }

    void MagazineDrop::Drop(const ETG::Vector2f& worldPos, const float rotation, const ETG::Vector2f& velocity,
                            const float depth)
    {
        // SetSprite çağırmamış bir silahın fırlatacağı bir şey yoktur. Burada sessizce hiçbir şey yapmamak
        // doğrudur: silahın reload işlemi yine çalışır, yalnızca geride magazine bırakmaz.
        if (!HasSprite()) return;

        Position = worldPos;
        Rotation = rotation;
        Velocity = velocity;
        Depth = depth;
        TimeLeft = LifeTime;
        Color = ETG::Color::White; // Önceki düşüşün sonunda kalan fade'i geri al

        // Burada publish edilir ve sonraki Update'e bırakılmaz; çünkü bir sonraki Draw'dan önce Update olmayabilir.
        // Düşüş, silahın Update'inden trigger edilir; AK47 için bu, GunBase::Update object'i o frame için tick
        // ettikten SONRA çalışır. Bu işlem olmazsa magazine'in çizilen ilk frame'i fırlatılmadan önceki property'leri
        // kullanır. İlk düşüşte buna null texture pointer da dahildir; bu yalnızca hatalı görünen bir sprite değil,
        // SpriteBatch içinde bir dereference oluşturur.
        ComputeDrawProperties();
    }

    void MagazineDrop::Update()
    {
        if (!IsFalling()) return;

        const float deltaTime = Time::FrameTick;

        // Basit Euler integration. Magazine ekranda bir saniyeden kısa süre kalır ve hiçbir şey nereye düştüğüne
        // bağlı değildir; bu yüzden burada daha iyi bir integrator kullanmaya değmez.
        Velocity.y += Gravity * deltaTime;
        Position += Velocity * deltaTime;
        Rotation += SpinSpeed * deltaTime;

        TimeLeft -= deltaTime;

        // LifeTime'ın son FadeTime saniyesinde fade olur; bu, yere inişi temsil eder. ComputeDrawProperties
        // değeri doğrudan draw call'a kopyaladığı için alpha, Color içine yazılır.
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

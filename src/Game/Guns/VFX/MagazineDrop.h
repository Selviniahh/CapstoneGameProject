#pragma once
#include <string>
#include "../../../Engine/Core/GameObjectBase.h"

namespace ETG
{
    // Bir silahın reload sırasında dışarı attığı magazine. Tamamen görseldir: magazine well'de bırakılıp
    // itilmesinin ardından yalnızca gravity, spin ve fade uygulanır; hiçbir şey onunla collide olmaz
    // ve hiçbir yer onu tekrar okumaz.
    //
    // NOTE: Bilerek MuzzleFlash gibi silaha parent EDİLMEMİŞTİR. Flash barrel üzerine çizildiğinden baktığı
    // her yerde onu takip etmelidir; düşen magazine ise silahtan ayrılmıştır ve hero hareket ederken bırakıldığı
    // world position'ı korur. Farklı bir sprite sheet kullanan başka bir MuzzleFlash olmak yerine ayrı bir
    // class olmasının temel nedeni bu farktır.
    class MagazineDrop : public GameObjectBase
    {
    public:
        // Başlangıçta texture içermez: hangi magazine'in düşeceğine owner silah karar verir. Bu nedenle
        // herkes için tahmin yürütmek yerine sprite'ı silah tarafından verilir.
        MagazineDrop();
        ~MagazineDrop() override = default;

        // Sprite'ı yükleyip origin'i merkezine yerleştirir; böylece düşen nesne merkezi etrafında döner
        void SetSprite(const std::string& relativePath);
        [[nodiscard]] bool HasSprite() const { return Texture != nullptr; }

        // Bir magazine fırlatır. Düşüş için gereken her şey burada belirlenir; böylece son magazine fade
        // olmadan tekrar reload yapan silah ikinci bir object sızdırmak yerine bunu yeniden başlatır.
        void Drop(const ETG::Vector2f& worldPos, float rotation, const ETG::Vector2f& velocity, float depth);

        void Update() override;
        void Draw() override;

        [[nodiscard]] bool IsFalling() const { return TimeLeft > 0.f; }

        // Saniyenin karesi başına pixel. Dungeon top-down çizilse de düşüş, zemine düşen gerçek bir object
        // gibi algılanır; bu yüzden silah boyunca değil, ekranda doğrudan aşağı doğru hızlanır.
        float Gravity{260.f};

        // Saniye başına degree. Silahtan çıkan magazine takla atar; bu olmazsa yapıştırılmış gibi aşağı kayar
        float SpinSpeed{220.f};

        // Düşüşün toplam süresi ve son kısmının ne kadarının fade ile geçeceği. Üzerine düşülecek bir zemin
        // olmadığından magazine'in yere inişini fade temsil eder ve magazine bu şekilde kaybolur.
        float LifeTime{0.9f};
        float FadeTime{0.35f};

    private:
        ETG::Vector2f Velocity{};
        float TimeLeft{};

        BOOST_DESCRIBE_CLASS(MagazineDrop, (GameObjectBase),
                             (Gravity, SpinSpeed, LifeTime, FadeTime),
                             (),
                             (Velocity, TimeLeft))
    };
}

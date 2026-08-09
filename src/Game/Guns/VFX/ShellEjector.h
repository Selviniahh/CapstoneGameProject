#pragma once
#include <string>
#include <vector>
#include <boost/describe.hpp>
#include "../../../Engine/Core/ComponentBase.h"
#include "../../../Engine/Platform/Platform.h"

namespace ETG
{
    class GunBase;

    // Tek bir kovan.
    //
    // NOTE: Bilerek GameObjectBase DEĞİLDİR. Kovanlar hiç kaybolmadığından sayıları bir oturumda yüzlere, binlere
    // çıkar; her biri scene listesine kaydolsaydı hierarchy paneli dolar ve her frame binlerce sanal Update/Draw
    // çağrısı ödenirdi. Bir kovanın adı, owner'ı, reflection'ı veya collision'ı zaten hiç kullanılmıyor -- geriye
    // yalnızca birkaç sayı kalıyor, o da düz bir struct demek.
    struct ShellCasing
    {
        ETG::Vector2f Position{};
        ETG::Vector2f Velocity{};
        float Rotation{};
        float SpinSpeed{};

        // Kovanın yere indiği sayılacağı Y. Ejection anında hesaplanır; böylece hero'nun o anda durduğu yükseklikte
        // yere iner, ekranda sabit bir zemin çizgisine değil.
        float GroundY{};

        // İndikten sonra bir daha simüle edilmez: Update onu atlar, yalnızca Draw çizmeye devam eder.
        bool Landed{false};
    };

    // Silahın her shot'ta dışarı attığı kovanlar. Düşer, zıplar, yere iner ve orada KALIR -- hiçbiri fade olmaz
    // veya silinmez. MagazineDrop'tan farkı budur: o tek bir object'i yeniden kullanıp fade ile yok eder, bu ise
    // biriktirir.
    class ShellEjector : public ComponentBase
    {
    public:
        ShellEjector();

        // <---------- Silahın author ettiği ---------->
        // Kovan atmayan silah (RogueSpecial gibi gerçek bir revolver) bunu kapatır.
        bool Enabled{true};

        // Kovanın çıktığı ejection port'u: HandRig::AnchorOrigin'e göre gun-local. Yanındaki Visualize kutucuğunu
        // tiklersen dünyada işaretlenir; yeri gözle ayarlanır.
        ETG::Vector2f EjectPoint{};

        // Kovanın çıkarken aldığı itme, saniye başına pixel. World-space'tir ve X, silah sola geçtiğinde mirror
        // edilir; böylece kovan iki tarafta da hero'nun içinden değil, silahın arkasından dışarı fırlar.
        ETG::Vector2f EjectVelocity{-14.f, -40.f};

        // Her kovanın hızına ve spin'ine eklenen rastgele sapma. Sıfır bırakılırsa tüm kovanlar aynı noktaya
        // üst üste iner ve tek bir kovan gibi okunur.
        float VelocityJitter{9.f};
        float SpinJitter{160.f};

        // Ejection noktasının kaç pixel altında yere indiği. Hero'nun ayak hizasına denk gelmesi beklenir.
        float GroundDrop{8.f};

        float Gravity{300.f};
        float SpinSpeed{420.f};

        // Yere ilk değdiğinde ne kadarını geri kazandığı. 0 çarptığı yerde yapışır; küçük bir değer kovana
        // gerçek bir sekme verir. Sekme, hız yeterince küçülünce kendiliğinden biter.
        float Bounce{0.35f};

        // Silah, hero ve zemin arasında nereye sort edileceği. SpriteBatch büyük depth'i önce çizer, dolayısıyla
        // hero'nunkinden büyük bir değer kovanı onun arkasında bırakır.
        float Depth{5.f};

        // Kovanın sprite'ı yoksa çizilen yedek: 1x1 pixel texture'ı bu boyuta ölçeklenip bu renkle tint'lenir.
        // Sprite verildiği anda ikisi de kullanılmaz; artwork ne büyüklükte ve ne renkteyse o çizilir.
        ETG::Vector2f CasingSize{2.f, 1.f};
        ETG::Color CasingColor{206, 160, 62};

        // Aynı anda tutulacak en fazla kovan; 0 SINIRSIZ demektir ve varsayılan odur. Sınır konursa en eski kovan
        // düşer. Kovanlar hiç kaybolmadığından bu, uzun bir oturumda biriken sayının tek freni olur.
        int MaxCasings{0};

        // Kovanın görseli. MagazineDrop ile aynı sözleşme: hangi kovanın düştüğüne silah karar verir, dolayısıyla
        // sprite'ı silah verir ve component kimse için tahmin yürütmez. Verilmezse aşağıdaki CasingSize/CasingColor
        // yolu çalışmaya devam eder, yani kendi kovan artwork'ü olmayan silahlar eskisi gibi kalır.
        //
        // NOTE: Tek frame'dir. Kovan zaten SpinSpeed ile döndüğünden animasyon eklense her frame'i döndürülmüş
        // 2 pixel olurdu; üstelik kovanlar yerde kalıcı olduğundan yüzlerce landed kovanın da kendi animation
        // saatini taşıması gerekirdi.
        void SetSprite(const std::string& relativePath);
        [[nodiscard]] bool HasSprite() const { return Texture != nullptr; }

        // <---------- GunBase'in çağırdıkları ---------->
        // Gerçekten çıkan her shot'ta bir kovan fırlatır.
        void Eject(const GunBase& gun);

        void Update() override;
        void Draw() override;

        [[nodiscard]] int CasingCount() const { return static_cast<int>(Casings.size()); }

        BOOST_DESCRIBE_CLASS(ShellEjector, (ComponentBase),
                             (Enabled, EjectPoint, EjectVelocity, VelocityJitter, SpinJitter, GroundDrop,
                                 Gravity, SpinSpeed, Bounce, Depth, CasingSize, CasingColor, MaxCasings),
                             (), ())

    private:
        // -Amount..+Amount arasında rastgele bir sapma
        [[nodiscard]] static float Jitter(float amount);

        std::vector<ShellCasing> Casings;
    };
}

#pragma once
#include <boost/describe.hpp>
#include "../../../Engine/Core/ComponentBase.h"
#include "../../../Engine/Platform/Platform.h"

namespace ETG
{
    // ============================== Hazır hareketler ==============================
    // Üçü de gun-local pixel cinsinden displacement üretir ve dinlenme hâlinde TAM OLARAK zero döner. Zero dönmeleri
    // tesadüf değil, sözleşmenin kendisidir: gesture'lar anchor'ın üzerine eklendiğinden, hareketsiz bir motion elin
    // authored grip pixel'inde durması demektir. Bu sayede istemediği hareketi "kapatmak" için hiçbir silahın ek kod
    // yazması gerekmez; genliği sıfır bırakmak yeterlidir ve varsayılan da odur.

    // Serbest çalışan nefes. El rest pose'undan Height kadar AŞAĞI iner, sonra geri çıkar ve bu hiç durmaz.
    // Yukarı çıkmaz: asılı duran bir elin nefesi budur.
    struct BreathMotion
    {
        // Elin rest pose'unun kaç pixel altına indiği. 0 nefesi tamamen kapatır.
        float Height{0.f};

        // Bir tam nefesin -- bir iniş ve bir çıkış -- saniye cinsinden süresi. Küçültmek nefesi hızlandırır.
        float Period{2.2f};

        // Period'un inişe ayrılan kısmı. 0.5 inişi ve çıkışı eşit hızlı yapar; büyütmek "yavaşça in, hızla kalk",
        // küçültmek "hızla in, yavaşça kalk" demektir.
        float FallRatio{0.5f};

        // Saati ilerletir ve bu frame'in displacement'ını döndürür
        ETG::Vector2f Advance(float deltaSeconds);

        BOOST_DESCRIBE_CLASS(BreathMotion, (), (Height, Period, FallRatio), (), ())

    private:
        // Kendi saati. GunBase::Timer kullanılamaz: o her shot'ta sıfırlanır, nefes ise ateş etmekten bağımsız akar.
        float Elapsed{};
    };

    // Silahın ateş anında yaptığı tek seferlik performans. Tek bir saatle sürülür: silah bir daire çizerken boştaki
    // el aynı anda nefes hattının üstüne bir kez çıkıp iner. İki genliği de sıfır bırakmak performansı kapatır.
    //
    // NOTE: Silahın kayması ile elin kayması bilerek aynı yerde durur. İkisi tek bir olayın -- shot'ın -- iki
    // yüzüdür; ayrı saatlere bölünmeleri, aynı anda başlaması gereken iki şeyin sessizce birbirinden kaymasına
    // izin verirdi.
    struct ShotKickMotion
    {
        // Performansın saniye cinsinden süresi. Silahın FireRate'inden kısa tutulmalıdır; aksi hâlde sonraki shot
        // daireyi ortasından keser.
        float Duration{0.16f};

        // Silahın çizdiği dairenin pixel cinsinden yarıçapı. Kavrayış toplamda iki radius kadar geriye gider.
        float GunCircleRadius{0.f};

        // Boştaki elin aynı süre içinde nefes hattının kaç pixel üstüne çıktığı.
        float OffHandRise{0.f};

        // Performansı baştan başlatır. GunBase her gerçek shot'ta çağırır.
        void Trigger();

        // Saati ilerletir. Aşağıdaki iki offset bu çağrıdan sonra okunur.
        void Advance(float deltaSeconds);

        // Silahın kendi position'ına uygulanacak kayma. Silah kaydığında ona anchor edilmiş eller kaymayı
        // beraberinde alır; HeldOffset bunu yapamaz, çünkü WorldPointOnGun onu bilerek geri alıp elleri sabit tutar.
        [[nodiscard]] ETG::Vector2f GunOffset() const;

        // Boştaki elin gesture'ına eklenecek vuruş.
        [[nodiscard]] ETG::Vector2f OffHandOffset() const;

        BOOST_DESCRIBE_CLASS(ShotKickMotion, (), (Duration, GunCircleRadius, OffHandRise), (), ())

    private:
        // Negatif değer performansın çalışmadığını belirtir. Silah "kick bitmiş" durumda doğar ve spawn anında
        // sallanmaz.
        float Elapsed{-1.f};
    };

    // Reload boyunca iki elin birlikte yaptığı performans. Çalışan el grip'ini bırakır, WorkingPoint'e uzanır, bir
    // kez yukarı çekip geri indirir ve yerine döner; sabit kalan el aynı weight ile SteadyPoint'e kayar.
    //
    // NOTE: Kendi saati YOKTUR; reload'un kendi 0..1 progress'i ile sürülür. Kendi timer'ını çalıştırsaydı
    // ReloadSlider'ın geri saydığı süreden sapar ve performans reload'dan farklı bir anda biterdi.
    struct ReloadReachMotion
    {
        // Silah reload performansı yazmıyorsa hiç çalıştırılmaz ve eller authored anchor'larında kalır.
        bool Enabled{false};

        // Aşağıdaki iki nokta da Origin-relative gun-local uzaydadır: image editor'da frame'in sol üstünden okunan
        // pixel'den frame'in Origin'i çıkarılarak elde edilir. Anchor'larla aynı uzayda olmaları, frame boyutu veya
        // Origin değişiminin gesture'a gizli bir offset eklememesini sağlar.

        // Çalışan elin uzandığı nokta (AK'de magazine well).
        ETG::Vector2f WorkingPoint{};

        // Sabit kalan elin kaydığı nokta (AK'de eğilen reload pose'undaki grip).
        ETG::Vector2f SteadyPoint{};

        // Üç beat, ReloadTime'ın fraction'ları olarak. Aşağı uzanma GrabEnd'e, yukarı-aşağı stroke StrokeEnd'e
        // kadar sürer; kalan sürede el anchor'ına geri döner.
        //
        // NOTE: 0.3 saniye değil, reload süresinin %30'u demektir.
        float GrabEnd{0.3f};
        float StrokeEnd{0.75f};

        // Stroke'un pixel cinsinden ne kadar yukarı çektiği. "Yukarı çekip indirme" hareketinin kendisidir.
        float StrokeHeight{3.f};

        // İki gesture'ı da yazar ve 0..1 engagement döndürür: elin idle grip'inden ne kadar uzaklaştığı. Silah,
        // magazine'in ne zaman ayrılacağı gibi kararları buradan okuyabilir.
        float Evaluate(float progress,
                       const ETG::Vector2f& workingAnchor, const ETG::Vector2f& steadyAnchor,
                       ETG::Vector2f& workingGesture, ETG::Vector2f& steadyGesture) const;

        BOOST_DESCRIBE_CLASS(ReloadReachMotion, (),
                             (Enabled, WorkingPoint, SteadyPoint, GrabEnd, StrokeEnd, StrokeHeight), (), ())
    };

    // ============================== HandRig ==============================
    // Silahın ellerine dair NE VARSA burada: hangi pixel'lerden tutulduğu, o ellerin bu frame'de nereye kaydığı,
    // grip pinning tercihi ve kaymayı üreten hazır hareketler. Her silah bunu Initialize'ında author eder, GunBase
    // her frame çalıştırır; silahın kendi Update'inde yazacağı bir şey kalmaz.
    //
    // NOTE: "Right" silahı tutan el, "Left" ise boştaki eldir. Bu, Character'ın Hand/OffHand ikilisiyle birebir
    // eşleşir ve silah taraf değiştirdiğinde ikisi ekranın karşı taraflarına geçer. Bu yüzden hiçbir hareketin
    // mirror edilmesi gerekmez: nefes "sol el" için değil, "boştaki el" için yazılmıştır.
    class HandRig : public ComponentBase
    {
    public:
        HandRig();

        // <---------- Silahın author ettiği grip ---------->
        // Her elin silahı kavradığı konum, silahın Origin'ine göre gun-local offset'tir. Image editor'da frame'in
        // sol üstünden okunan bir pixel, burada saklanmadan önce `pixel - frameOrigin` ile bu uzaya çevrilir.
        ETG::Vector2f RightHandAnchor{};
        ETG::Vector2f LeftHandAnchor{};

        // El yalnızca gerçekten ölçülmüş bir anchor için silaha attach edilir. Ölçülmemiş off hand bunun yerine
        // character body üzerinde kalır.
        bool HasRightHandAnchor{false};
        bool HasLeftHandAnchor{false};

        // <---------- Bu frame'in sonucu ---------->
        // Elin YERLEŞTİRİLDİĞİ konuma eklenen displacement'tır; Character bunları okur.
        //
        // NOTE: Anchor'lara yazılmak yerine onlardan ayrı tutulur. Anchor'lar silahın nereden tutulduğuna ilişkin
        // authored gerçektir ve grip pinning hangi pixel'in sabit duracağını belirlemek için LeftHandAnchor'ı okur.
        // Anchor içine yazılan gesture pin'i de beraberinde sürükler ve el her hareket ettiğinde tüm silah sallanırdı.
        //
        // NOTE: Elin yerleştirildiği uzayda ifade edilir. Silahı kavrayan el için bu gun-local'dır ve silahla
        // birlikte rotate edilir. Silahın anchor vermediği boştaki el ise body üzerinde durur; oradaki displacement
        // world-space'tir. Aksi hâlde boştaki elin nefesi aim angle ile birlikte eğilirdi.
        ETG::Vector2f RightHandGesture{};
        ETG::Vector2f LeftHandGesture{};

        // <---------- Grip pinning ---------->
        // Two-handed silahın barrel'ı yukarı bakarken forward grip holder'ın altından dışarı savrulursa görünüm
        // bozulur; off hand sprite'ın tamamen dışında, havada kalır. Pinning bu pixel'i body'ye göre sabitler ve
        // barrel'ın onun etrafında hareket etmesini sağlar. Silah yine rotate edilir, ancak kendi Origin'i yerine
        // grip etrafında döner. One-handed silah false bırakır.
        //
        // NOTE: Kuralın gun state okuyan yarısı (barrel yukarıda mı, pin hangi angle'da donuyor) GunBase'de
        // virtual olarak kalır; burada duran yalnızca silahın authored tercihidir.
        bool PinsGripWhenAimingUp{false};

        // <---------- Hazır hareketler ---------->
        BreathMotion OffHandBreath{};
        ShotKickMotion ShotKick{};
        ReloadReachMotion ReloadReach{};

        // <---------- Silahın / GunBase'in çağırdıkları ---------->
        // Gerçekten çıkan her shot'ta çağrılır. Ateşleme kararını GunBase verdiği için oradan çağrılır; silahın
        // PrepareShooting'i override etmesi gerekmez.
        void OnShotFired();

        // Her frame, GunBase::Update içinde. `reloadProgress` negatifse reload çalışmıyordur.
        void Tick(float deltaSeconds, float reloadProgress);

        // Silahın kendi position'ına uygulanacak bu frame'lik kayma. GunBase uygular.
        [[nodiscard]] ETG::Vector2f GunKickOffset() const { return ShotKick.GunOffset(); }

        // İki gesture'ı da sıfırlar; bu, elleri authored anchor'larına geri koyar.
        void RestHands();

        BOOST_DESCRIBE_CLASS(HandRig, (ComponentBase),
                             (RightHandAnchor, LeftHandAnchor, HasRightHandAnchor, HasLeftHandAnchor,
                                 RightHandGesture, LeftHandGesture, PinsGripWhenAimingUp,
                                 OffHandBreath, ShotKick, ReloadReach),
                             (), ())
    };
}
